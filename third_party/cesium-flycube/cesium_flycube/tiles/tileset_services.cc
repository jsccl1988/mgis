// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "cesium_flycube/tiles/tileset_services.h"

#include "cesium_flycube/tiles/prepare_resources.h"

#include <CesiumAsync/ITaskProcessor.h>
#include <CesiumCurl/CurlAssetAccessor.h>
#include <CesiumUtility/CreditSystem.h>

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>

namespace cesium_flycube {
namespace {

class ThreadPoolTaskProcessor : public CesiumAsync::ITaskProcessor {
 public:
  ThreadPoolTaskProcessor() {
    for (int i = 0; i < 4; ++i) {
      workers_.emplace_back([this] { Run(); });
    }
  }

  ~ThreadPoolTaskProcessor() override {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      stop_ = true;
    }
    condition_.notify_all();
    for (std::thread& worker : workers_) {
      worker.join();
    }
  }

  void startTask(std::function<void()> function) override {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      queue_.push(std::move(function));
    }
    condition_.notify_one();
  }

 private:
  void Run() {
    for (;;) {
      std::function<void()> function;
      {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this] { return stop_ || !queue_.empty(); });
        if (stop_ && queue_.empty()) {
          return;
        }
        function = std::move(queue_.front());
        queue_.pop();
      }
      function();
    }
  }

  std::mutex mutex_;
  std::condition_variable condition_;
  std::queue<std::function<void()>> queue_;
  std::vector<std::thread> workers_;
  bool stop_ = false;
};

}  // namespace

struct TilesetServices::State {
  std::shared_ptr<CesiumAsync::ITaskProcessor> tasks;
  CesiumAsync::AsyncSystem async;
  std::shared_ptr<CesiumCurl::CurlAssetAccessor> assets;
  std::shared_ptr<CesiumUtility::CreditSystem> credits;
  std::shared_ptr<PrepareRendererResources> prepare;

  State()
      : tasks(std::make_shared<ThreadPoolTaskProcessor>()),
        async(tasks),
        assets(std::make_shared<CesiumCurl::CurlAssetAccessor>()),
        credits(std::make_shared<CesiumUtility::CreditSystem>()),
        prepare(std::make_shared<PrepareRendererResources>()) {}
};

TilesetServices::TilesetServices() : state_(std::make_unique<State>()) {}

TilesetServices::~TilesetServices() = default;

Cesium3DTilesSelection::TilesetExternals TilesetServices::Externals() const {
  return Cesium3DTilesSelection::TilesetExternals{
      state_->assets,
      state_->prepare,
      state_->async,
      state_->credits,
  };
}

void TilesetServices::Pump() {
  state_->assets->tick();
  state_->async.dispatchMainThreadTasks();
}

CesiumUtility::CreditSystem& TilesetServices::credits() {
  return *state_->credits;
}

}  // namespace cesium_flycube
