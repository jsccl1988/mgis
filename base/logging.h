#pragma once

#ifdef NOMINMAX
#define GLOG_CUSTOM_PREFIX_SUPPORT
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include "glog/logging.h"
#else  // #ifdef NOMINMAX
// Define and undefine `NOMINMAX` to ensure that min()/std::max() macros are not
// leaked by glog's implementation by mistake.
#define NOMINMAX
#define GLOG_CUSTOM_PREFIX_SUPPORT
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include "glog/logging.h"
#undef NOMINMAX

#endif  // #ifdef NOMINMAX

namespace base {
namespace detail {
void LogFormatter(std::ostream&, const google::LogMessageInfo&, void*);
}  // namespace detail
}  // namespace base
