#pragma once
#include <lmdb.h>
#include <stdexcept>
#include <string>
#include <filesystem>
#include <iostream>

namespace lmdbmap {

class environment {
public:
    environment(const std::string& path, size_t map_size = 104857600, unsigned int max_dbs = 10) {
        int rc = mdb_env_create(&env_);
        if (rc != 0) throw std::runtime_error(mdb_strerror(rc));

        rc = mdb_env_set_mapsize(env_, map_size);
        if (rc != 0) {
            mdb_env_close(env_);
            throw std::runtime_error(mdb_strerror(rc));
        }

        rc = mdb_env_set_maxdbs(env_, max_dbs);
        if (rc != 0) {
            mdb_env_close(env_);
            throw std::runtime_error(mdb_strerror(rc));
        }

        std::filesystem::create_directories(path);
        rc = mdb_env_open(env_, path.c_str(), 0, 0664);
        if (rc != 0) {
            mdb_env_close(env_);
            throw std::runtime_error(mdb_strerror(rc));
        }
    }

    ~environment() {
        if (env_) mdb_env_close(env_);
    }

    operator MDB_env*() const { return env_; }

private:
    MDB_env* env_ = nullptr;
};

}
