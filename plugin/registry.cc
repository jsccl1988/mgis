// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "plugin/registry.h"

#include <ctype.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <string>

namespace {

const int kMaxJsonDepth = 32;
const size_t kMaxManifestBytes = 256 * 1024;

std::wstring Utf8ToWide(const std::string& utf8) {
  if (utf8.empty())
    return std::wstring();
  const int in_len = static_cast<int>(utf8.size());
  const int out_len = MultiByteToWideChar(
      CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(), in_len, NULL, 0);
  if (out_len <= 0)
    return std::wstring();
  std::wstring wide(static_cast<size_t>(out_len), L'\0');
  MultiByteToWideChar(
      CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(), in_len, &wide[0], out_len);
  return wide;
}

bool ReadFileUtf8(const std::wstring& path, std::string* out) {
  HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE)
    return false;
  LARGE_INTEGER size;
  if (!GetFileSizeEx(file, &size) || size.QuadPart < 0 ||
      static_cast<unsigned long long>(size.QuadPart) > kMaxManifestBytes) {
    CloseHandle(file);
    return false;
  }
  out->assign(static_cast<size_t>(size.QuadPart), '\0');
  DWORD read = 0;
  BOOL ok = TRUE;
  if (!out->empty()) {
    ok = ReadFile(file, &(*out)[0], static_cast<DWORD>(out->size()), &read,
                  NULL);
  }
  CloseHandle(file);
  return ok == TRUE && read == out->size();
}

void AppendUtf8(std::string* out, unsigned int code) {
  if (code <= 0x7Fu) {
    out->push_back(static_cast<char>(code));
  } else if (code <= 0x7FFu) {
    out->push_back(static_cast<char>(0xC0u | (code >> 6)));
    out->push_back(static_cast<char>(0x80u | (code & 0x3Fu)));
  } else {
    out->push_back(static_cast<char>(0xE0u | (code >> 12)));
    out->push_back(static_cast<char>(0x80u | ((code >> 6) & 0x3Fu)));
    out->push_back(static_cast<char>(0x80u | (code & 0x3Fu)));
  }
}

int HexNibble(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return -1;
}

// Accepts the plugin.json subset: objects, arrays, strings, and skipped scalars.
class ManifestParser {
 public:
  explicit ManifestParser(const std::string& text) : text_(text), i_(0) {}

  bool ParseManifest(plugin::Manifest* manifest) {
    if (!Consume('{'))
      return false;
    bool saw_id = false;
    bool saw_name = false;
    bool saw_commands = false;
    SkipWs();
    if (Consume('}'))
      return false;
    for (;;) {
      std::string key;
      if (!ParseString(&key))
        return false;
      if (!Consume(':'))
        return false;
      if (key == "id") {
        if (saw_id || !ParseString(&manifest->id))
          return false;
        saw_id = true;
      } else if (key == "name") {
        if (saw_name || !ParseString(&manifest->name))
          return false;
        saw_name = true;
      } else if (key == "commands") {
        if (saw_commands || !ParseCommandArray(&manifest->commands))
          return false;
        saw_commands = true;
      } else if (!SkipValue(0)) {
        return false;
      }
      SkipWs();
      if (Consume('}'))
        break;
      if (!Consume(','))
        return false;
    }
    SkipWs();
    if (i_ != text_.size())
      return false;
    return saw_id && saw_name && saw_commands;
  }

 private:
  void SkipWs() {
    while (i_ < text_.size()) {
      const char c = text_[i_];
      if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
        break;
      ++i_;
    }
  }

  bool Consume(char expected) {
    SkipWs();
    if (i_ >= text_.size() || text_[i_] != expected)
      return false;
    ++i_;
    return true;
  }

  bool ParseString(std::string* out) {
    SkipWs();
    if (i_ >= text_.size() || text_[i_] != '"')
      return false;
    ++i_;
    out->clear();
    while (i_ < text_.size()) {
      const char c = text_[i_++];
      if (c == '"')
        return true;
      if (c == '\\') {
        if (i_ >= text_.size())
          return false;
        const char esc = text_[i_++];
        if (esc == '"' || esc == '\\' || esc == '/') {
          out->push_back(esc);
        } else if (esc == 'b') {
          out->push_back('\b');
        } else if (esc == 'f') {
          out->push_back('\f');
        } else if (esc == 'n') {
          out->push_back('\n');
        } else if (esc == 'r') {
          out->push_back('\r');
        } else if (esc == 't') {
          out->push_back('\t');
        } else if (esc == 'u') {
          unsigned int code = 0;
          if (!ParseHex4(&code))
            return false;
          if (code >= 0xD800u && code <= 0xDFFFu)
            return false;
          AppendUtf8(out, code);
        } else {
          return false;
        }
      } else if (static_cast<unsigned char>(c) < 0x20u) {
        return false;
      } else {
        out->push_back(c);
      }
    }
    return false;
  }

  bool ParseHex4(unsigned int* code) {
    if (i_ + 4 > text_.size())
      return false;
    unsigned int value = 0;
    for (int n = 0; n < 4; ++n) {
      const int nibble = HexNibble(text_[i_++]);
      if (nibble < 0)
        return false;
      value = (value << 4) | static_cast<unsigned int>(nibble);
    }
    *code = value;
    return true;
  }

  bool ParseCommandArray(std::vector<std::string>* commands) {
    if (!Consume('['))
      return false;
    commands->clear();
    SkipWs();
    if (Consume(']'))
      return true;
    for (;;) {
      std::string command;
      if (!ParseString(&command))
        return false;
      commands->push_back(command);
      SkipWs();
      if (Consume(']'))
        return true;
      if (!Consume(','))
        return false;
    }
  }

  bool SkipValue(int depth) {
    if (depth > kMaxJsonDepth)
      return false;
    SkipWs();
    if (i_ >= text_.size())
      return false;
    const char c = text_[i_];
    if (c == '"') {
      std::string ignored;
      return ParseString(&ignored);
    }
    if (c == '{')
      return SkipObject(depth + 1);
    if (c == '[')
      return SkipArray(depth + 1);
    if (c == 't')
      return ConsumeLiteral("true");
    if (c == 'f')
      return ConsumeLiteral("false");
    if (c == 'n')
      return ConsumeLiteral("null");
    return SkipNumber();
  }

  bool ConsumeLiteral(const char* literal) {
    for (size_t n = 0; literal[n] != '\0'; ++n) {
      if (i_ >= text_.size() || text_[i_] != literal[n])
        return false;
      ++i_;
    }
    return true;
  }

  bool SkipNumber() {
    if (i_ < text_.size() && text_[i_] == '-')
      ++i_;
    if (i_ >= text_.size() || !isdigit(static_cast<unsigned char>(text_[i_])))
      return false;
    while (i_ < text_.size() && isdigit(static_cast<unsigned char>(text_[i_])))
      ++i_;
    if (i_ < text_.size() && text_[i_] == '.') {
      ++i_;
      if (i_ >= text_.size() ||
          !isdigit(static_cast<unsigned char>(text_[i_])))
        return false;
      while (i_ < text_.size() &&
             isdigit(static_cast<unsigned char>(text_[i_])))
        ++i_;
    }
    if (i_ < text_.size() && (text_[i_] == 'e' || text_[i_] == 'E')) {
      ++i_;
      if (i_ < text_.size() && (text_[i_] == '+' || text_[i_] == '-'))
        ++i_;
      if (i_ >= text_.size() ||
          !isdigit(static_cast<unsigned char>(text_[i_])))
        return false;
      while (i_ < text_.size() &&
             isdigit(static_cast<unsigned char>(text_[i_])))
        ++i_;
    }
    return true;
  }

  bool SkipObject(int depth) {
    if (!Consume('{'))
      return false;
    SkipWs();
    if (Consume('}'))
      return true;
    for (;;) {
      std::string ignored;
      if (!ParseString(&ignored) || !Consume(':') || !SkipValue(depth))
        return false;
      SkipWs();
      if (Consume('}'))
        return true;
      if (!Consume(','))
        return false;
    }
  }

  bool SkipArray(int depth) {
    if (!Consume('['))
      return false;
    SkipWs();
    if (Consume(']'))
      return true;
    for (;;) {
      if (!SkipValue(depth))
        return false;
      SkipWs();
      if (Consume(']'))
        return true;
      if (!Consume(','))
        return false;
    }
  }

  const std::string& text_;
  size_t i_;
};

bool LoadManifestFile(const std::wstring& path, plugin::Manifest* manifest) {
  std::string text;
  if (!ReadFileUtf8(path, &text))
    return false;
  if (text.size() >= 3 &&
      static_cast<unsigned char>(text[0]) == 0xEFu &&
      static_cast<unsigned char>(text[1]) == 0xBBu &&
      static_cast<unsigned char>(text[2]) == 0xBFu) {
    text.erase(0, 3);
  }
  ManifestParser parser(text);
  return parser.ParseManifest(manifest);
}

}  // namespace

namespace plugin {

Registry::Registry() {}

Registry::~Registry() {}

bool Registry::Scan(const std::string& directory_utf8) {
  const std::wstring dir = Utf8ToWide(directory_utf8);
  if (dir.empty())
    return false;
  const DWORD attr = GetFileAttributesW(dir.c_str());
  if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY) == 0)
    return false;

  records_.clear();

  Manifest root_manifest;
  if (LoadManifestFile(dir + L"\\plugin.json", &root_manifest))
    Register(root_manifest);

  const std::wstring pattern = dir + L"\\*";
  WIN32_FIND_DATAW data;
  HANDLE find = FindFirstFileW(pattern.c_str(), &data);
  if (find == INVALID_HANDLE_VALUE)
    return true;
  do {
    if (wcscmp(data.cFileName, L".") == 0 || wcscmp(data.cFileName, L"..") == 0)
      continue;
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
      continue;
    Manifest child;
    const std::wstring child_json =
        dir + L"\\" + data.cFileName + L"\\plugin.json";
    if (LoadManifestFile(child_json, &child))
      Register(child);
  } while (FindNextFileW(find, &data));
  FindClose(find);
  return true;
}

bool Registry::Register(const Manifest& manifest) {
  if (manifest.id.empty() || manifest.name.empty())
    return false;
  for (size_t i = 0; i < manifest.commands.size(); ++i) {
    if (manifest.commands[i].empty())
      return false;
  }
  if (Find(manifest.id) != NULL)
    return false;
  records_.push_back(manifest);
  return true;
}

const Manifest* Registry::Find(const std::string& id) const {
  for (size_t i = 0; i < records_.size(); ++i) {
    if (records_[i].id == id)
      return &records_[i];
  }
  return NULL;
}

std::vector<Manifest> Registry::List() const {
  return records_;
}

}  // namespace plugin
