// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/browser/devtools/mcp_server.h"

#include <stdio.h>

#include <windows.h>

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#include <rapidjson/document.h>

namespace content {
namespace {

McpBindFn g_bind_hook = NULL;

bool EnsureWinsock() {
  static bool ready = false;
  if (ready)
    return true;
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    return false;
  ready = true;
  return true;
}

std::wstring DiscoveryPath() {
  wchar_t local_app[MAX_PATH];
  const DWORD n =
      GetEnvironmentVariableW(L"LOCALAPPDATA", local_app, MAX_PATH);
  if (n == 0 || n >= MAX_PATH)
    return std::wstring();
  return std::wstring(local_app) + L"\\MView\\debug-agent.json";
}

void DeleteDiscovery() {
  const std::wstring path = DiscoveryPath();
  if (!path.empty())
    DeleteFileW(path.c_str());
}

bool WriteDiscovery(int port) {
  const std::wstring path = DiscoveryPath();
  if (path.empty() || port <= 0)
    return false;
  const std::wstring dir = path.substr(0, path.find_last_of(L'\\'));
  CreateDirectoryW(dir.c_str(), NULL);
  const std::string json =
      "{\"port\":" + std::to_string(port) + ",\"url\":\"http://127.0.0.1:" +
      std::to_string(port) + "/mcp\"}";
  HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE)
    return false;
  DWORD written = 0;
  const BOOL ok = WriteFile(file, json.data(), static_cast<DWORD>(json.size()),
                            &written, NULL);
  CloseHandle(file);
  return ok && written == json.size();
}

void CloseListen(SOCKET* listen_socket) {
  if (*listen_socket == INVALID_SOCKET)
    return;
  closesocket(*listen_socket);
  *listen_socket = INVALID_SOCKET;
}

std::string EscapeJson(const std::string& text) {
  std::string out;
  out.reserve(text.size());
  for (size_t i = 0; i < text.size(); ++i) {
    const unsigned char c = static_cast<unsigned char>(text[i]);
    switch (c) {
      case '\\':
        out += "\\\\";
        break;
      case '"':
        out += "\\\"";
        break;
      case '\n':
        out += "\\n";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\t':
        out += "\\t";
        break;
      default:
        if (c < 0x20) {
          char hex[8];
          _snprintf_s(hex, sizeof(hex), _TRUNCATE, "\\u%04x", c);
          out += hex;
        } else {
          out.push_back(static_cast<char>(c));
        }
        break;
    }
  }
  return out;
}

std::string JsonId(const rapidjson::Document& doc) {
  if (!doc.IsObject() || !doc.HasMember("id"))
    return "null";
  const rapidjson::Value& id = doc["id"];
  if (id.IsInt64())
    return std::to_string(id.GetInt64());
  if (id.IsUint64())
    return std::to_string(id.GetUint64());
  if (id.IsString())
    return std::string("\"") + EscapeJson(id.GetString()) + "\"";
  return "null";
}

std::string RpcError(const std::string& id, int code, const char* message) {
  return std::string("{\"jsonrpc\":\"2.0\",\"id\":") + id +
         ",\"error\":{\"code\":" + std::to_string(code) + ",\"message\":\"" +
         message + "\"}}";
}

std::string RpcResult(const std::string& id, const std::string& text) {
  return std::string("{\"jsonrpc\":\"2.0\",\"id\":") + id +
         ",\"result\":{\"content\":[{\"type\":\"text\",\"text\":\"" +
         EscapeJson(text) + "\"}]}}";
}

bool JsonScalar(const rapidjson::Value& value, std::string* out) {
  if (value.IsString()) {
    *out = value.GetString();
    return true;
  }
  if (value.IsInt64()) {
    *out = std::to_string(value.GetInt64());
    return true;
  }
  if (value.IsUint64()) {
    *out = std::to_string(value.GetUint64());
    return true;
  }
  if (value.IsDouble()) {
    const double number = value.GetDouble();
    const long long whole = static_cast<long long>(number);
    if (number == static_cast<double>(whole))
      *out = std::to_string(whole);
    else
      *out = std::to_string(number);
    return true;
  }
  return false;
}

bool Arg(const rapidjson::Value* args, const char* key, std::string* out) {
  if (!args || !args->IsObject() || !args->HasMember(key))
    return false;
  return JsonScalar((*args)[key], out);
}

bool IsKnownTool(const std::string& name) {
  return name == "help" || name == "exec_line" || name == "scene_mode" ||
         name == "attach" || name == "pointer" || name == "command" ||
         name == "activate_tool" || name == "expect_camera" ||
         name == "capture" || name == "suite" || name == "mark" ||
         name == "trace";
}

// Turns a tools/call name plus arguments into one glang line. Returns false
// when the tool is unknown or a required field is missing, so the caller
// does not touch ExecLine or marks.
bool BuildToolLine(const std::string& name,
                   const rapidjson::Value* args,
                   std::string* line) {
  if (name == "exec_line") {
    std::string text;
    if (!Arg(args, "line", &text))
      return false;
    *line = text;
    return true;
  }
  if (name == "help" || name == "trace") {
    *line = name;
    return true;
  }
  if (name == "scene_mode") {
    std::string mode;
    if (!Arg(args, "mode", &mode))
      return false;
    *line = "scene_mode " + mode;
    return true;
  }
  if (name == "attach") {
    std::string view;
    if (!Arg(args, "view", &view))
      return false;
    *line = "attach " + view;
    std::string backend;
    if (Arg(args, "backend", &backend))
      *line += " " + backend;
    return true;
  }
  if (name == "pointer") {
    std::string view;
    std::string action;
    std::string x;
    std::string y;
    if (!Arg(args, "view", &view) || !Arg(args, "action", &action) ||
        !Arg(args, "x", &x) || !Arg(args, "y", &y))
      return false;
    *line = "pointer " + view + " " + action + " " + x + " " + y;
    std::string delta;
    if (Arg(args, "delta", &delta))
      *line += " " + delta;
    return true;
  }
  if (name == "command" || name == "activate_tool") {
    std::string id;
    std::string view;
    if (!Arg(args, "id", &id) || !Arg(args, "view", &view))
      return false;
    *line = name + " " + id + " " + view;
    return true;
  }
  if (name == "expect_camera") {
    *line = "expect_camera";
    std::string view;
    std::string lon;
    std::string lat;
    std::string height;
    if (Arg(args, "view", &view))
      *line += " " + view;
    if (Arg(args, "lon", &lon))
      *line += " " + lon;
    if (Arg(args, "lat", &lat))
      *line += " " + lat;
    if (Arg(args, "height", &height))
      *line += " " + height;
    return true;
  }
  if (name == "capture") {
    *line = "capture";
    std::string view;
    std::string path;
    if (Arg(args, "view", &view))
      *line += " " + view;
    if (Arg(args, "path", &path))
      *line += " " + path;
    return true;
  }
  if (name == "suite") {
    std::string path;
    if (!Arg(args, "path", &path))
      return false;
    *line = "suite " + path;
    return true;
  }
  if (name == "mark") {
    std::string mark_name;
    if (!Arg(args, "name", &mark_name))
      return false;
    *line = "mark " + mark_name;
    return true;
  }
  return false;
}

int ContentLength(const std::string& headers) {
  const char* keys[] = {"Content-Length:", "content-length:"};
  for (size_t i = 0; i < 2; ++i) {
    const size_t pos = headers.find(keys[i]);
    if (pos == std::string::npos)
      continue;
    size_t index = pos + strlen(keys[i]);
    while (index < headers.size() && headers[index] == ' ')
      ++index;
    int value = 0;
    while (index < headers.size() && headers[index] >= '0' &&
           headers[index] <= '9') {
      value = value * 10 + (headers[index] - '0');
      ++index;
    }
    return value;
  }
  return -1;
}

bool SendAll(SOCKET sock, const std::string& bytes) {
  int sent = 0;
  while (sent < static_cast<int>(bytes.size())) {
    const int n = send(sock, bytes.data() + sent,
                       static_cast<int>(bytes.size()) - sent, 0);
    if (n <= 0)
      return false;
    sent += n;
  }
  return true;
}

}  // namespace

void SetMcpBindHookForTest(McpBindFn hook) {
  g_bind_hook = hook;
}

McpServer::McpServer(LineFn line_fn, void* context)
    : line_fn_(line_fn),
      context_(context),
      listen_(INVALID_SOCKET),
      port_(0),
      bind_failed_(false),
      stopping_(false) {}

McpServer::~McpServer() {
  Stop();
}

bool McpServer::Start() {
  Stop();
  stopping_.store(false);
  bind_failed_ = false;
  port_ = 0;
  if (!EnsureWinsock()) {
    bind_failed_ = true;
    return false;
  }
  listen_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (listen_ == INVALID_SOCKET) {
    bind_failed_ = true;
    return false;
  }
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = htons(0);
  const int bind_result =
      g_bind_hook ? g_bind_hook(listen_, reinterpret_cast<sockaddr*>(&addr),
                                sizeof(addr))
                  : ::bind(listen_, reinterpret_cast<sockaddr*>(&addr),
                           sizeof(addr));
  if (bind_result != 0) {
    bind_failed_ = true;
    CloseListen(&listen_);
    return false;
  }
  int addr_len = sizeof(addr);
  if (getsockname(listen_, reinterpret_cast<sockaddr*>(&addr), &addr_len) !=
      0) {
    bind_failed_ = true;
    CloseListen(&listen_);
    return false;
  }
  port_ = ntohs(addr.sin_port);
  if (port_ <= 0 || listen(listen_, 8) != 0) {
    bind_failed_ = true;
    port_ = 0;
    CloseListen(&listen_);
    return false;
  }
  if (!WriteDiscovery(port_)) {
    port_ = 0;
    CloseListen(&listen_);
    return false;
  }
  thread_ = std::thread(&McpServer::Run, this);
  return true;
}

void McpServer::Stop() {
  stopping_.store(true);
  CloseListen(&listen_);
  if (thread_.joinable())
    thread_.join();
  port_ = 0;
  DeleteDiscovery();
}

int McpServer::port() const {
  return port_;
}

bool McpServer::bind_failed() const {
  return bind_failed_;
}

void McpServer::Run() {
  while (!stopping_.load()) {
    const SOCKET client = accept(listen_, NULL, NULL);
    if (client == INVALID_SOCKET)
      break;
    Serve(client);
    closesocket(client);
  }
}

void McpServer::Serve(SOCKET client) {
  const DWORD timeout_ms = 3000;
  setsockopt(client, SOL_SOCKET, SO_RCVTIMEO,
             reinterpret_cast<const char*>(&timeout_ms), sizeof(timeout_ms));
  std::string data;
  char buf[4096];
  while (data.find("\r\n\r\n") == std::string::npos) {
    const int n = recv(client, buf, sizeof(buf), 0);
    if (n <= 0)
      return;
    data.append(buf, buf + n);
    if (data.size() > 1024 * 1024)
      return;
  }
  const size_t header_end = data.find("\r\n\r\n");
  const std::string headers = data.substr(0, header_end);
  std::string body = data.substr(header_end + 4);
  const int content_length = ContentLength(headers);
  if (content_length >= 0) {
    while (static_cast<int>(body.size()) < content_length) {
      const int n = recv(client, buf, sizeof(buf), 0);
      if (n <= 0)
        break;
      body.append(buf, buf + n);
    }
    if (static_cast<int>(body.size()) > content_length)
      body.resize(static_cast<size_t>(content_length));
  }
  const bool is_mcp = headers.find("POST /mcp ") == 0;
  const std::string payload =
      is_mcp ? HandleJson(body) : RpcError("null", -32601, "method not found");
  std::string response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
                         "Connection: close\r\nContent-Length: ";
  response += std::to_string(payload.size());
  response += "\r\n\r\n";
  response += payload;
  SendAll(client, response);
}

std::string McpServer::HandleJson(const std::string& body) const {
  rapidjson::Document doc;
  doc.Parse(body.c_str());
  if (doc.HasParseError() || !doc.IsObject())
    return RpcError("null", -32700, "parse error");
  const std::string id = JsonId(doc);
  if (!doc.HasMember("method") || !doc["method"].IsString() ||
      std::string(doc["method"].GetString()) != "tools/call") {
    return RpcError(id, -32601, "method not found");
  }
  if (!doc.HasMember("params") || !doc["params"].IsObject())
    return RpcError(id, -32602, "invalid params");
  const rapidjson::Value& params = doc["params"];
  if (!params.HasMember("name") || !params["name"].IsString())
    return RpcError(id, -32602, "invalid params");
  const std::string name = params["name"].GetString();
  if (!IsKnownTool(name))
    return RpcError(id, -32602, "unknown tool");
  const rapidjson::Value* args =
      params.HasMember("arguments") ? &params["arguments"] : NULL;
  std::string line;
  if (!BuildToolLine(name, args, &line))
    return RpcError(id, -32602, "invalid params");
  if (!line_fn_)
    return RpcError(id, -32603, "internal error");
  return RpcResult(id, line_fn_(context_, line));
}

}  // namespace content
