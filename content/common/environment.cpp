#include "content/common/environment.h"

namespace content {
Environment* Environment::singleton_ = NULL;
Environment* Environment::GetSingletonPtr(void) {
  if (singleton_ == NULL) {
    singleton_ = new Environment();
  }
  return singleton_;
}

void Environment::DestoryInstance(void) { SAFE_DELETE(singleton_); }
}  // namespace content