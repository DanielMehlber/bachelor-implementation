#ifndef JOBMANAGER_H
#define JOBMANAGER_H

#include "JobManagerState.h"
#include "jobsystem/JobSystemFactory.h"
#include "jobsystem/execution/IJobExecution.h"
#include "jobsystem/execution/impl/fiber/FiberExecutionImpl.h"
#include "jobsystem/execution/impl/singleThreaded/SingleThreadedExecutionImpl.h"
#include "jobsystem/job/Job.h"
#include "jobsystem/job/TimerJob.h"
#include "logging/Logging.h"
#include <future>
#include <memory>
#include <queue>
#include <set>

using namespace jobsystem::job;
using namespace jobsystem::execution;

#ifdef JOB_SYSTEM_SINGLE_THREAD
typedef impl::SingleThreadedExecutionImpl JobExecutionImpl;
#else
typedef impl::FiberExecutionImpl JobExecutionImpl;
#endif

namespace jobsystem {

class JobManager {
private:
  /**
   * @brief All jobs for the initialization phase of the cycle are collected
   * here.
   */
  std::queue<SharedJob> m_init_queue;
  std::mutex m_init_queue_mutex;
  SharedJobCounter m_init_phase_counter;

  /**
   * @brief All jobs for the main processing phase of the cycle are collected
   * here.
   */
  std::queue<SharedJob> m_main_queue;
  std::mutex m_main_queue_mutex;
  SharedJobCounter m_main_phase_counter;

  /**
   * @brief All jobs for the clean-up phase of the cycle are collected here.
   */
  std::queue<SharedJob> m_clean_up_queue;
  std::mutex m_clean_up_queue_mutex;
  SharedJobCounter m_clean_up_phase_counter;

  /**
   * @brief All jobs that should not be kicked for the following cycles instead
   * for the currently running cycles will be collected here. This is the case,
   * when a job is automatically rescheduled for future cycles (e.g.
   * time-interval jobs).
   */
  std::queue<SharedJob> m_next_cycle_queue;
  std::mutex m_next_cycle_queue_mutex;

  /**
   * @brief Some jobs must be prevented from being rescheduled for the next
   * cycle. This is often the case when trying to detach jobs which are
   * currently in the execution and must therefore be prevented from requeueing.
   */
  std::set<std::string> m_continuation_requeue_blacklist;
  std::mutex m_continuation_requeue_blacklist_mutex;

  JobManagerState m_current_state{READY};
  JobExecutionImpl m_execution;

  size_t m_total_cycle_count{0};

// Debug values
#ifndef NDEBUG
  std::atomic<size_t> m_job_execution_counter{0};
  size_t m_cycles_counter{0};
#endif

  void ExecuteQueueAndWait(std::queue<SharedJob> &queue,
                           std::mutex &queue_mutex, SharedJobCounter counter);

  void ScheduleAllJobsInQueue(std::queue<SharedJob> &queue,
                              std::mutex &queue_mutex,
                              SharedJobCounter counter);

  void ResetContinuationRequeueBlacklist();

public:
  JobManager();
  ~JobManager();
  void PrintStatusLog();

  /**
   * @brief Pass detached job instance to manager in order to be executed in the
   * current cycle or the next one, if none is currently running.
   * @param job job that should be executed
   * @note The job will be executed when the execution cycle has been invoked
   */
  void KickJob(SharedJob job);

  /**
   * @brief Ensures that a job which is not yet in execution will not be
   * executed (again)
   * @param job_id id of this job
   */
  void DetachJob(const std::string &job_id);

  /**
   * @brief Pass detached job instance to manager in order to be exected in the
   * next cycle (not the current one).
   * @param job job that should be executed
   * @note This is useful for re-queueing jobs for later execution
   */
  void KickJobForNextCycle(SharedJob job);

  /**
   * @brief Starts a new execution cycle and passes queued jobs to the
   * execution. The calling thread will be blocked until the cycle has finished.
   */
  void InvokeCycleAndWait();

  /**
   * @brief Execution will wait (or will be deferred, depending on the execution
   * environment) until the waitable object has been finished.
   * @param waitable process that needs finish for the current calling party to
   * continue.
   * @note On single-threaded implementations, this cannot be called from inside
   * a job because it would deadlock the worker thread.
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

  size_t GetTotalCyclesCount() noexcept;
};

inline void
JobManager::WaitForCompletion(std::shared_ptr<IJobWaitable> counter) {
  m_execution.WaitForCompletion(counter);
}

template <typename FutureType>
inline void
JobManager::WaitForCompletion(const std::future<FutureType> &future) {
  m_execution.WaitForCompletion(future);
}

typedef std::shared_ptr<JobManager> SharedJobManager;

} // namespace jobsystem

#endif /* JOBMANAGER_H */
