// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#include "content/public/browser/trace_log.h"

#include <windows.h>

#include <sstream>

#include "base/logging.h"

namespace content {
namespace {

const size_t kTraceCapacity = 256;

unsigned long long NowMs() {
  return GetTickCount64();
}

}  // namespace

TraceLine::TraceLine() : channel(kTraceConsole), tick_ms(0) {}

TraceLog::Observer::Observer() {}

TraceLog::Observer::~Observer() {}

void TraceLog::Observer::OnTrace(const TraceLine& line) {
  (void)line;
}

TraceLog::TraceLog() {}

TraceLog::~TraceLog() {}

void TraceLog::AddObserver(Observer* observer) {
  if (!observer)
    return;
  for (size_t i = 0; i < observers_.size(); ++i) {
    if (observers_[i] == observer)
      return;
  }
  observers_.push_back(observer);
}

void TraceLog::RemoveObserver(Observer* observer) {
  for (size_t i = 0; i < observers_.size(); ++i) {
    if (observers_[i] == observer) {
      observers_.erase(observers_.begin() + static_cast<std::ptrdiff_t>(i));
      return;
    }
  }
}

void TraceLog::Append(TraceChannel channel, const std::string& text) {
  TraceLine line;
  line.channel = channel;
  line.tick_ms = NowMs();
  line.text = text;
  lines_.push_back(line);
  if (lines_.size() > kTraceCapacity)
    lines_.erase(lines_.begin());
  if (channel == kTracePerf)
    DLOG(INFO) << "trace " << text;
  else
    LOG(INFO) << "trace " << text;
  const std::vector<Observer*> observers = observers_;
  for (size_t i = 0; i < observers.size(); ++i) {
    if (observers[i])
      observers[i]->OnTrace(line);
  }
}

void TraceLog::Snapshot(TraceChannel channel,
                        std::vector<std::string>* out) const {
  if (!out)
    return;
  out->clear();
  for (size_t i = 0; i < lines_.size(); ++i) {
    if (lines_[i].channel == channel)
      out->push_back(lines_[i].text);
  }
}

TraceScope::TraceScope(TraceLog* log, const char* name)
    : log_(log), name_(name), start_ms_(NowMs()) {}

TraceScope::~TraceScope() {
  if (!log_ || !name_)
    return;
  const unsigned long long elapsed = NowMs() - start_ms_;
  std::ostringstream text;
  text << name_ << " " << elapsed << "ms";
  log_->Append(kTracePerf, text.str());
}

}  // namespace content
