#ifndef TRANSACTION_MANAGER_H
#define TRANSACTION_MANAGER_H

#include "Transaction.h"
#include "LockManager.h"
#include <atomic>

class TransactionManager {
public:
  TransactionManager(LockManager* lock_manager) : lock_manager_(lock_manager), next_txn_id_(0) {}

  Transaction* Begin() {
    return new Transaction(next_txn_id_++);
  }

  void Commit(Transaction* txn) {
    txn->SetState(TransactionState::COMMITTED);
    lock_manager_->ReleaseAllLocks(txn);
  }

  void Abort(Transaction* txn) {
    txn->SetState(TransactionState::ABORTED);
    lock_manager_->ReleaseAllLocks(txn);
    // In a full implementation, we would undo changes here.
  }

private:
  LockManager* lock_manager_;
  std::atomic<int> next_txn_id_;
};

#endif
