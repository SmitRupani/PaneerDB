#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <vector>
#include <string>
#include <unordered_set>

enum class TransactionState { GROWING, SHRINKING, COMMITTED, ABORTED };
enum class LockMode { SHARED, EXCLUSIVE };

struct LockRequest {
  std::string tableName;
  LockMode mode;
  bool operator==(const LockRequest& other) const {
      return tableName == other.tableName && mode == other.mode;
  }
};

namespace std {
  template<> struct hash<LockRequest> {
      size_t operator()(const LockRequest& r) const {
          return hash<string>()(r.tableName) ^ hash<int>()(static_cast<int>(r.mode));
      }
  };
}

class Transaction {
public:
  Transaction(int txn_id) : txn_id_(txn_id), state_(TransactionState::GROWING) {}

  int GetTransactionId() const { return txn_id_; }
  TransactionState GetState() const { return state_; }
  void SetState(TransactionState state) { state_ = state; }

  void AddLock(const LockRequest& req) { locks_.insert(req); }
  const std::unordered_set<LockRequest>& GetLocks() const { return locks_; }

private:
  int txn_id_;
  TransactionState state_;
  std::unordered_set<LockRequest> locks_;
};

#endif
