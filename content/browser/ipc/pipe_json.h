// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_BROWSER_IPC_PIPE_JSON_H_
#define CONTENT_BROWSER_IPC_PIPE_JSON_H_

#include <string>

// Windows.h may already have defined min/max; RapidJSON needs the names free.
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#include <rapidjson/document.h>

namespace content {

// RapidJSON helpers for the named-pipe envelope (same shape as smartgis
// sdbd_json / args_json: parse into Document, typed getters on Value).

bool parse_pipe_json(const std::string& text, rapidjson::Document* out);

bool pipe_json_string(const rapidjson::Value& obj,
                      const char* key,
                      std::string* out);
bool pipe_json_double(const rapidjson::Value& obj,
                      const char* key,
                      double* out);
bool pipe_json_bool(const rapidjson::Value& obj, const char* key, bool* out);
// Exact integer (HWND / sizes). Rejects non-integer numbers.
bool pipe_json_int64(const rapidjson::Value& obj,
                     const char* key,
                     long long* out);

// True when |obj| is an object whose member set is exactly |keys|.
bool pipe_json_exact_keys(const rapidjson::Value& obj,
                          const char* const* keys,
                          size_t count);

const rapidjson::Value* pipe_json_member(const rapidjson::Value& obj,
                                         const char* key);

void pipe_json_copy(rapidjson::Document* dest, const rapidjson::Value& src);

std::string pipe_json_stringify(const rapidjson::Value& value);

}  // namespace content

#endif  // CONTENT_BROWSER_IPC_PIPE_JSON_H_
