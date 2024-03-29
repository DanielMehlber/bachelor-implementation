#ifndef TIMERJOB_H
#define TIMERJOB_H

#include "Job.h"
#include <chrono>

namespace jobsystem::job {

/**
 * While regular jobs are executed right away in the next execution
 * cycle, this job has the ability to defer execution for a certain amount of
 * time by skipping cycles.
 * @note This is useful to create jobs which execute in certain time-intervals
 * instead of every cycle.
 */
class TimerJob : public Job {
private:
  /**
   * Point in time where the timer was started. This is not the creation
   * of the job (constructor), but the first attempt of scheduling it.
   */
  std::chrono::time_point<std::chrono::high_resolution_clock> m_timer_start;

  /** Duration that has to pass for the job to get scheduled. */
  std::chrono::duration<double> m_time;

  /** If the timer has been started already. */
  bool m_timer_started{false};

public:
  TimerJob(std::function<JobContinuation(JobContext *)>, const std::string &id,
           std::chrono::duration<double> time, JobExecutionPhase phase = MAIN);
  TimerJob(std::function<JobContinuation(JobContext *)>,
           std::chrono::duration<double> time, JobExecutionPhase phase = MAIN);
  ~TimerJob() override = default;

  /**
   * Restarts the timer by resetting the start point.
   */
  void RestartTimer();

  bool IsReadyForExecution(const JobContext &context) final;
};
} // namespace jobsystem::job

#endif /* TIMERJOB_H */
