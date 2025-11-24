#pragma once
#include <lmdb.h>
#include <stdexcept>
#include "environment.hpp"

namespace lmdbmap {

class transaction {
public:
    transaction(environment& env, bool read_only = false) {
        int rc = mdb_txn_begin(env, nullptr, read_only ? MDB_RDONLY : 0, &txn_);
        if (rc != 0) throw std::runtime_error(mdb_strerror(rc));
    }

    ~transaction() {
        if (txn_) mdb_txn_abort(txn_);
    }

    void commit() {
        if (!txn_) return;
        int rc = mdb_txn_commit(txn_);
        txn_ = nullptr;
        if (rc != 0) throw std::runtime_error(mdb_strerror(rc));
    }

    void abort() {
        if (!txn_) return;
        mdb_txn_abort(txn_);
        txn_ = nullptr;
    }

    operator MDB_txn*() const { return txn_; }

private:
    MDB_txn* txn_ = nullptr;
};

}
