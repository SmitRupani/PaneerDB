#ifndef LOCK_MANAGER_H
#define LOCK_MANAGER_H

#include "Transaction.h"
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <string>

class LockManager {
public:
  LockManager() = default;

  bool LockTable(Transaction* txn, LockMode mode, const std::string& tableName);
  void UnlockTable(Transaction* txn, const std::string& tableName);
  void ReleaseAllLocks(Transaction* txn);

private:
  struct LockState {
    int shared_count = 0;
    int exclusive_count = 0; // Can only be 0 or 1
    std::condition_variable cv;
  };

  std::mutex mutex_;
  std::unordered_map<std::string, LockState> table_locks_;
};

#endif
