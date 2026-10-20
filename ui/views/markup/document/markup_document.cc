// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "ui/views/markup/document/markup_document.h"

#include <cctype>
#include <cstdio>
#include <sstream>
#include <string>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "pugixml.hpp"

namespace views {
namespace {

std::wstring Utf8ToWidePath(const std::string& u8) {
  if (u8.empty())
    return std::wstring();
  const int n = MultiByteToWideChar(CP_UTF8, 0, u8.c_str(), -1, NULL, 0);
  if (n <= 0)
    return std::wstring();
  std::wstring w(static_cast<size_t>(n), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, u8.c_str(), -1, &w[0], n);
  w.resize(static_cast<size_t>(n - 1));
  return w;
}

std::string ReadFileUtf8(const std::string& path) {
  const std::wstring wpath = Utf8ToWidePath(path);
  if (wpath.empty())
    return std::string();
  FILE* f = NULL;
  if (_wfopen_s(&f, wpath.c_str(), L"rb") != 0 || !f)
    return std::string();
  std::ostringstream ss;
  char buf[4096];
  while (const size_t n = fread(buf, 1, sizeof(buf), f))
    ss.write(buf, static_cast<std::streamsize>(n));
  fclose(f);
  return ss.str();
}

bool WriteFileUtf8(const std::string& path, const std::string& data) {
  const std::wstring wpath = Utf8ToWidePath(path);
  if (wpath.empty())
    return false;
  FILE* f = NULL;
  if (_wfopen_s(&f, wpath.c_str(), L"wb") != 0 || !f)
    return false;
  const size_t n = fwrite(data.data(), 1, data.size(), f);
  fclose(f);
  return n == data.size();
}

std::string JoinPath(std::string_view base_dir, std::string_view rel) {
  if (base_dir.empty())
    return std::string(rel);
  std::string out(base_dir);
  if (!out.empty() && out.back() != '/' && out.back() != '\\')
    out.push_back('/');
  out.append(rel);
  return out;
}

std::string ToLower(std::string_view s) {
  std::string out;
  out.reserve(s.size());
  for (char c : s) {
    out.push_back(
        static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
  }
  return out;
}

}  // namespace

MarkupDocument::MarkupDocument()
    : doc_(new pugi::xml_document()) {}

MarkupDocument::~MarkupDocument() {}

bool MarkupDocument::LoadXml(std::string_view xml_utf8,
                             std::string_view base_dir,
                             std::string* error) {
  doc_->reset();
  css_.Clear();
  const pugi::xml_parse_result r =
      doc_->load_buffer(xml_utf8.data(), xml_utf8.size());
  if (!r) {
    if (error)
      *error = std::string("xml: ") + r.description();
    return false;
  }
  return LoadStyleLinks(base_dir, error);
}

bool MarkupDocument::LoadCss(std::string_view css_utf8, std::string* error) {
  CssParser extra;
  if (!extra.Parse(css_utf8, error))
    return false;
  return css_.AppendRules(extra);
}

bool MarkupDocument::LoadStyleLinks(std::string_view base_dir,
                                    std::string* error) {
  pugi::xml_node root = doc_->document_element();
  if (!root) {
    if (error)
      *error = "xml: missing root";
    return false;
  }

  for (pugi::xml_node child : root.children()) {
    if (ToLower(child.name()) != "style")
      continue;
    if (const char* src = child.attribute("src").as_string(NULL)) {
      const std::string path = JoinPath(base_dir, src);
      const std::string text = ReadFileUtf8(path);
      if (text.empty()) {
        if (error)
          *error = "css: failed to read " + path;
        return false;
      }
      if (!LoadCss(text, error))
        return false;
    } else if (child.child_value() && child.child_value()[0] != '\0') {
      if (!LoadCss(child.child_value(), error))
        return false;
    }
  }
  return true;
}

std::string MarkupDocument::Name() const {
  pugi::xml_node root = doc_->document_element();
  if (!root)
    return std::string();
  return root.attribute("name").as_string("");
}

std::string MarkupDocument::SerializeXml() const {
  std::ostringstream ss;
  doc_->save(ss, "  ");
  return ss.str();
}

std::string MarkupDocument::SerializeCss() const {
  return css_.Serialize();
}

bool MarkupDocument::SaveFiles(const std::string& xml_path,
                               const std::string& css_path) const {
  if (!WriteFileUtf8(xml_path, SerializeXml()))
    return false;
  if (!css_path.empty() && !WriteFileUtf8(css_path, SerializeCss()))
    return false;
  return true;
}

}  // namespace views
