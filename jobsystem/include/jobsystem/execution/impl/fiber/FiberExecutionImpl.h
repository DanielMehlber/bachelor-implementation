#ifndef FIBEREXECUTIONIMPL_H
#define FIBEREXECUTIONIMPL_H

#include "boost/fiber/all.hpp"
#include "jobsystem/execution/IJobExecution.h"
#include <condition_variable>
#include <memory>
#include <thread>
#include <vector>

using namespace jobsystem::execution;

namespace jobsystem::execution::impl {
class FiberExecutionImpl : IJobExecution<FiberExecutionImpl> {
private:
  JobExecutionState m_current_state{STOPPED};

  const size_t m_worker_thread_count{std::thread::hardware_concurrency() - 1};

  /**
   * @brief Contains all worker threads that run scheduled fibers.
   */
  std::vector<std::shared_ptr<std::thread>> m_worker_threads;

  std::unique_ptr<boost::fibers::buffered_channel<std::function<void()>>>
      m_job_channel;

  /**
   * @brief Managing instance necessary to execute jobs and build the
   * JobContext.
   * @note This is set when starting the execution and cleared when the
   * execution has stopped.
   */
  JobManager *m_managing_instance;

  void Init();
  void ShutDown();

  /**
   * @brief Processes jobs as fibers.
   * @note This is run by the worker threads
   */
  void
  ExecuteWorker(std::shared_ptr<boost::fibers::barrier> m_init_sync_barrier);

public:
  FiberExecutionImpl();
  virtual ~FiberExecutionImpl();

  /**
   * @brief Schedules the job for execution
   * @param job job to be executed.
   */
  void Schedule(std::shared_ptr<Job> job);

  /**
   * @brief Wait for the counter to become 0. The implementation can vary
   * depending on the underlying synchronization primitives.
   * @attention Some implementations do not support this call from inside a
   * running job (e.g. the single threaded implementation because this would
   * block the only execution thread)
   * @param waitable counter to wait for
   */
  void WaitForCompletion(std::shared_ptr<IJobWaitable> waitable);

  /**
   * @brief Execution of the calling party will wait (or will be deferred,
   * depending on the execution environment) until the passed future has been
   * resolved.
   * @tparam FutureType type of the future object
   * @param future future that must resolve in order for the calling party to
   * continue.
   */
  template <typename FutureType>
  void WaitForCompletion(const std::future<FutureType> &future);

  /**
   * @brief Starts processing scheduled jobs and invoke the execution
   * @param manager managing instance that started the execution
   */
  void Start(JobManager *manager);

  /**
   * @brief Stops processing scheduled jobs and pauses the execution
   * @note Already scheduled jobs will remain scheduled until the execution is
   * resumed.
   */
  void Stop();

  JobExecutionState GetState();
};

template <typename FutureType>
inline void
FiberExecutionImpl::WaitForCompletion(const std::future<FutureType> &future) {
  bool is_called_from_fiber =
      !boost::fibers::context::active()->is_context(boost::fibers::type::none);
  if (is_called_from_fiber) {
    // caller is a fiber, so yield
    while (future.wait_for(std::chrono::seconds(0)) !=
           std::future_status::ready) {
      boost::this_fiber::yield();
    }
  } else {
    // caller is a thread, so block
    future.wait();
  }
}

inline JobExecutionState FiberExecutionImpl::GetState() {
  return m_current_state;
}

} // namespace jobsystem::execution::impl

#endif /* FIBEREXECUTIONIMPL_H */
