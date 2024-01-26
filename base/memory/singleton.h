#ifndef BASE_MEMORY_SINGLETON_H_
#define BASE_MEMORY_SINGLETON_H_

#include <memory>
#include <mutex>

#include "base/util/at_exit.h"

namespace base {
template <typename TSingleton, bool release_atexitmanager = true>
class Singleton {
 private:
  friend TSingleton;
  using TSingletonDefaultDelete = std::default_delete<TSingleton>;
  using TSingletonPtr = std::unique_ptr<TSingleton, TSingletonDefaultDelete>;

 private:
  Singleton(void) = default;
  virtual ~Singleton() = default;
  Singleton(const Singleton&) = delete;
  Singleton(Singleton&&) = delete;
  Singleton& operator=(const Singleton&) = delete;

 public:
  static TSingletonPtr& GetInstance() {
    static TSingletonPtr instance = nullptr;
    static std::once_flag oc;
    std::call_once(oc, [&] {
      instance = std::make_unique<TSingleton>();
      if (release_atexitmanager) {
        AtExitManager::RegisterCallback(
            [&](void*) -> void {
              instance.reset(nullptr);
#if _MSC_VER > 1900
              oc._Opaque = 0;
#else
					oc._Flag = 0;
#endif
            },
            nullptr);
      }
    });

    return instance;
  }
};

}  // namespace base

#endif  // BASE_MEMORY_SINGLETON_H_
