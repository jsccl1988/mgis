// Copyright (c) 2026 The MGIS Authors.
// All rights reserved.
//
// Tiny in-process sdbd HTTP mock for mgis tests / local Bind proof.
//   out\sdbd_mock.exe [port]
// Default port 8021. Not a mogu sdbd product.
//
//   curl http://127.0.0.1:8021/sdbd/api/v1/health
//   curl http://127.0.0.1:8021/sdbd/api/v1/catalog
//   curl -X POST http://127.0.0.1:8021/sdbd/api/v1/recordset/query ^
//     -H "Content-Type: application/json" ^
//     -d "{\"layer\":\"t_rtree_demo\",\"bbox\":{\"min_x\":100,\"min_y\":100,\"max_x\":200,\"max_y\":200},\"predicate\":\"intersects\"}"

#include "testing/sdbd/sdbd_http_mock.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

int main(int argc, char** argv) {
  int port = 8021;
  if (argc > 1) {
    port = std::atoi(argv[1]);
  }
  testing::sdbd::SdbdHttpMock mock;
  if (!mock.Start("127.0.0.1", port)) {
    std::cerr << "sdbd_mock: failed to listen on 127.0.0.1:" << port << "\n";
    return 1;
  }
  std::cout << "sdbd_mock listening on " << mock.base_url()
            << " (GET /sdbd/api/v1/health|catalog layers/{layer}; "
               "POST /sdbd/api/v1/recordset/open|fetch|close|query)\n"
            << "golden: t_rtree_demo narrow bbox (100,100)-(200,200) "
               "count==1 id==2\n"
            << "Ctrl+C to stop.\n";
  for (;;) {
    std::this_thread::sleep_for(std::chrono::seconds(60));
  }
}
