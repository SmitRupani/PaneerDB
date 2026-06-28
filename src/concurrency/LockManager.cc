#include "LockManager.h"
#include <iostream>

bool LockManager::LockTable(Transaction* txn, LockMode mode, const std::string& tableName) {
  if (txn->GetState() != TransactionState::GROWING) {
    txn->SetState(TransactionState::ABORTED);
    return false; // Strict 2PL requires locks to only be acquired in growing phase
  }

  std::unique_lock<std::mutex> lock(mutex_);
  auto& state = table_locks_[tableName];

  if (mode == LockMode::SHARED) {
    // Wait until no exclusive locks
    state.cv.wait(lock, [&state] { return state.exclusive_count == 0; });
    state.shared_count++;
  } else {
    // Wait until no exclusive AND no shared locks
    state.cv.wait(lock, [&state] { return state.exclusive_count == 0 && state.shared_count == 0; });
    state.exclusive_count++;
  }

  txn->AddLock({tableName, mode});
  // std::cout << "Transaction " << txn->GetTransactionId() << " acquired " << (mode == LockMode::SHARED ? "SHARED" : "EXCLUSIVE") << " lock on table " << tableName << "\n";
  return true;
}

void LockManager::UnlockTable(Transaction* txn, const std::string& tableName) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  // Find lock
  LockMode mode = LockMode::SHARED;
  bool found = false;
  for (const auto& req : txn->GetLocks()) {
    if (req.tableName == tableName) {
      mode = req.mode;
      found = true;
      break;
    }
  }

  if (!found) return;

  auto& state = table_locks_[tableName];
  if (mode == LockMode::SHARED) {
    state.shared_count--;
  } else {
    state.exclusive_count--;
  }

  state.cv.notify_all();
}

void LockManager::ReleaseAllLocks(Transaction* txn) {
  std::lock_guard<std::mutex> lock(mutex_);
  for (const auto& req : txn->GetLocks()) {
    auto& state = table_locks_[req.tableName];
    if (req.mode == LockMode::SHARED) {
      state.shared_count--;
    } else {
      state.exclusive_count--;
    }
    state.cv.notify_all();
  }
}
