// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef SDB_SDBD_URL_H
#define SDB_SDBD_URL_H

#include <string>

namespace core {
namespace sdb {

struct SdbdLinkUrl {
  std::string base_url;
  std::string layer;
  std::string host{"127.0.0.1"};
  int port = 8021;
  std::string error;
  bool ok = false;
  bool is_mapd_port = false;
};

SdbdLinkUrl parse_sdbd_url(const std::string& url);
std::string default_sdbd_base();
bool env_flag_set(const char* name);

}  // namespace sdb
}  // namespace core

#endif  // SDB_SDBD_URL_H
