#pragma once

#include <memory>
#include <vector>

#include "folly/executors/CPUThreadPoolExecutor.h"
#include "folly/executors/task_queue/UnboundedBlockingQueue.h"
#include "folly/logging/xlog.h"

namespace pcf {
template <class AppType>
class MpcAppExecutor {
 public:
  explicit MpcAppExecutor(int16_t concurrency) : concurrency_{concurrency} {
    executor_ = std::make_unique<folly::CPUThreadPoolExecutor>(
        concurrency,
        std::make_unique<folly::UnboundedBlockingQueue<
            folly::CPUThreadPoolExecutor::CPUTask>>());

    executor_->addObserver(std::make_shared<MyObserver>());
  }

  // execute MpcApps concurrently
  void execute(const std::vector<std::unique_ptr<AppType>>& mpcApps) {
    for (auto& app : mpcApps) {
      executor_->add([&app]() { app->run(); });
    }

    executor_->join();
  }

 private:
  int16_t concurrency_;
  std::unique_ptr<folly::CPUThreadPoolExecutor> executor_;

  class MyObserver : public folly::CPUThreadPoolExecutor::Observer {
   public:
    void threadStarted(
        folly::CPUThreadPoolExecutor::ThreadHandle* threadHandle) override {
      numThread_++;
      XLOGF(INFO, "Thread started. Total threads: {}", numThread_);
    }

    void threadStopped(
        folly::CPUThreadPoolExecutor::ThreadHandle* threadHandle) override {
      numThread_--;
      XLOGF(INFO, "Thread ended. Total threads: {}", numThread_);
    }

   private:
    int numThread_;
  };
};
} // namespace pcf
