#ifndef SIMULATION_FRAMEWORK_BOOSTFIBERRECURSIVESPINLOCK_H
#define SIMULATION_FRAMEWORK_BOOSTFIBERRECURSIVESPINLOCK_H

#include "boost/fiber/all.hpp"
#include "common/synchronization/SpinLock.h"
#include <string>

namespace jobsystem {

union OwnerId {
  boost::fibers::fiber::id fiber_id;
  std::thread::id thread_id;
};

/**
 * Information about the owner of the recursive spin lock, which is either a
 * thread or a fiber.
 */
struct OwnerInfo {
  bool is_fiber;
  OwnerId id;

  bool operator==(OwnerInfo &other) const {
    if (is_fiber == other.is_fiber) {
      if (is_fiber) {
        return id.fiber_id == other.id.fiber_id;
      } else {
        return id.thread_id == other.id.thread_id;
      }
    }

    return false;
  }
};

class BoostFiberRecursiveSpinLock {
protected:

  /** another normal spin lock synchronizing access to owner information */
  mutable common::sync::SpinLock m_owner_info_mutex;

  /** information about the current owner (thread or fiber) */
  std::optional<OwnerInfo> m_owner_info;

  /** amount of recursive locks by the current owner */
  std::atomic_int m_owner_lock_count{0};

  /** if true, this is currently locked */
  std::atomic_flag m_lock;

  void yield() const;

public:
  void lock();
  void unlock();
  bool try_lock();
};

} // namespace jobsystem

#endif // SIMULATION_FRAMEWORK_BOOSTFIBERRECURSIVESPINLOCK_H
