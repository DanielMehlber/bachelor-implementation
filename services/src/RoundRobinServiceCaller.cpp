#include "services/caller/impl/RoundRobinServiceCaller.h"
#include "jobsystem/manager/JobManager.h"
#include <cmath>

using namespace services::impl;
using namespace services;
using namespace jobsystem;

bool RoundRobinServiceCaller::IsCallable() const noexcept {
  std::unique_lock lock(m_service_executors_mutex);
  for (const auto &stub : m_service_executors) {
    if (stub->IsCallable()) {
      return true;
    }
  }

  return false;
}

void RoundRobinServiceCaller::AddExecutor(SharedServiceExecutor stub) {
  std::unique_lock lock(m_service_executors_mutex);
  m_service_executors.push_back(stub);
}

std::future<SharedServiceResponse>
RoundRobinServiceCaller::Call(SharedServiceRequest request,
                              jobsystem::SharedJobManager job_manager,
                              bool only_local) noexcept {

  std::shared_ptr<std::promise<SharedServiceResponse>> promise =
      std::make_shared<std::promise<SharedServiceResponse>>();
  std::future<SharedServiceResponse> future = promise->get_future();

  SharedJob job = JobSystemFactory::CreateJob(
      [_this = shared_from_this(), promise, request, only_local,
       job_manager](JobContext *context) {
        std::optional<SharedServiceExecutor> opt_stub =
            _this->SelectNextUsableCaller(only_local);
        if (opt_stub.has_value()) {
          // call stub
          SharedServiceExecutor stub = opt_stub.value();
          auto result_future = stub->Call(request, job_manager);

          // wait for call to complete
          context->GetJobManager()->WaitForCompletion(result_future);
          try {
            SharedServiceResponse response = result_future.get();
            promise->set_value(response);
          } catch (...) {
            promise->set_exception(std::current_exception());
          }
        } else {
          auto exception = BUILD_EXCEPTION(NoCallableServiceFound,
                                           "no callable stub for service "
                                               << request->GetServiceName());
          promise->set_exception(std::make_exception_ptr(exception));
        }

        return JobContinuation::DISPOSE;
      });

  job_manager->KickJob(job);
  return future;
}

std::optional<SharedServiceExecutor>
RoundRobinServiceCaller::SelectNextUsableCaller(bool only_local) {
  std::unique_lock lock(m_service_executors_mutex);
  // just do one round-trip searching for finding a callable service stub.
  // Otherwise, there is none.
  size_t tries = 0;
  while (tries < m_service_executors.size()) {
    size_t next_index =
        (std::min(m_service_executors.size() - 1, m_last_index) + 1) %
        m_service_executors.size();

    const SharedServiceExecutor &stub = m_service_executors.at(next_index);
    tries++;
    m_last_index = next_index;

    if (stub->IsCallable()) {
      if (only_local) {
        if (!stub->IsLocal()) {
          return {};
        }
      }
      return stub;
    }
  }

  return {};
}

bool RoundRobinServiceCaller::ContainsLocallyCallable() const noexcept {
  std::unique_lock lock(m_service_executors_mutex);
  for (const auto &stub : m_service_executors) {
    if (stub->IsCallable() && stub->IsLocal()) {
      return true;
    }
  }

  return false;
}

size_t RoundRobinServiceCaller::GetCallableCount() const noexcept {
  std::unique_lock lock(m_service_executors_mutex);
  size_t count = 0;
  for (auto &stubs : m_service_executors) {
    if (stubs->IsCallable()) {
      count++;
    }
  }

  return count;
}
