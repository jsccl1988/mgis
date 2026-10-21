// Copyright (c) 2026 The Mogu Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_BROWSER_TRACE_LOG_H_
#define CONTENT_PUBLIC_BROWSER_TRACE_LOG_H_

#include <string>
#include <vector>

namespace content {

// One session ring. Not a process-wide log. Main thread only.
enum TraceChannel {
  kTraceConsole = 0,
  kTracePerf = 1,
  kTraceAgent = 2,
};

struct TraceLine {
  TraceLine();

  TraceChannel channel;
  unsigned long long tick_ms;
  std::string text;
};

// Ring of facts the shell paints into the bottom console.
// Observers are not owned. Remove them before they die.
class TraceLog {
 public:
  class Observer {
   public:
    virtual ~Observer();
    virtual void OnTrace(const TraceLine& line);

   protected:
    Observer();

   private:
    Observer(const Observer&) = delete;
    Observer& operator=(const Observer&) = delete;
  };

  TraceLog();
  ~TraceLog();

  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

  void Append(TraceChannel channel, const std::string& text);

  // Copies lines for |channel|, oldest first. At most the ring capacity.
  void Snapshot(TraceChannel channel, std::vector<std::string>* out) const;

 private:
  TraceLog(const TraceLog&) = delete;
  TraceLog& operator=(const TraceLog&) = delete;

  std::vector<TraceLine> lines_;
  std::vector<Observer*> observers_;
};

// Records elapsed milliseconds on the perf channel when destroyed.
class TraceScope {
 public:
  TraceScope(TraceLog* log, const char* name);
  ~TraceScope();

 private:
  TraceScope(const TraceScope&) = delete;
  TraceScope& operator=(const TraceScope&) = delete;

  TraceLog* log_;
  const char* name_;
  unsigned long long start_ms_;
};

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_TRACE_LOG_H_
