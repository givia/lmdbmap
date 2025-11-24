#pragma once
#include "environment.hpp"
#include "transaction.hpp"
#include "serialization.hpp"
#include <lmdb.h>
#include <string>
#include <optional>
#include <iterator>
#include <vector>
#include <cstring>

namespace lmdbmap {

template<typename Key, typename T>
class multimap {
public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<Key, T>;

    multimap(environment& env, const std::string& name) : env_(env) {
        transaction txn(env);
        int rc = mdb_dbi_open(txn, name.c_str(), MDB_CREATE | MDB_DUPSORT, &dbi_);
        if (rc != 0) throw std::runtime_error(mdb_strerror(rc));
        txn.commit();
    }

    ~multimap() {
        // mdb_dbi_close(env_, dbi_);
    }

    void insert(transaction& txn, const Key& key, const T& value) {
        std::string k = serialize(key);
        std::string v = serialize(value);
        MDB_val key_val{k.size(), k.data()};
        MDB_val data_val{v.size(), v.data()};
        int rc = mdb_put(txn, dbi_, &key_val, &data_val, 0);
        if (rc != 0) throw std::runtime_error(mdb_strerror(rc));
    }

    std::vector<T> get(transaction& txn, const Key& key) {
        std::vector<T> results;
        MDB_cursor* cursor;
        int rc = mdb_cursor_open(txn, dbi_, &cursor);
        if (rc != 0) throw std::runtime_error(mdb_strerror(rc));

        std::string k = serialize(key);
        MDB_val key_val{k.size(), k.data()};
        MDB_val data_val;

        rc = mdb_cursor_get(cursor, &key_val, &data_val, MDB_SET);
        if (rc == MDB_NOTFOUND) {
            mdb_cursor_close(cursor);
            return results;
        }
        if (rc != 0) {
            mdb_cursor_close(cursor);
            throw std::runtime_error(mdb_strerror(rc));
        }

        do {
            results.push_back(deserialize<T>(data_val));
            rc = mdb_cursor_get(cursor, &key_val, &data_val, MDB_NEXT_DUP);
        } while (rc == 0);

        mdb_cursor_close(cursor);
        return results;
    }

    void erase(transaction& txn, const Key& key) {
        std::string k = serialize(key);
        MDB_val key_val{k.size(), k.data()};
        int rc = mdb_del(txn, dbi_, &key_val, nullptr);
        if (rc != 0 && rc != MDB_NOTFOUND) throw std::runtime_error(mdb_strerror(rc));
    }

    void erase(transaction& txn, const Key& key, const T& value) {
        std::string k = serialize(key);
        std::string v = serialize(value);
        MDB_val key_val{k.size(), k.data()};
        MDB_val data_val{v.size(), v.data()};
        int rc = mdb_del(txn, dbi_, &key_val, &data_val);
        if (rc != 0 && rc != MDB_NOTFOUND) throw std::runtime_error(mdb_strerror(rc));
    }

    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<Key, T>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        iterator() : cursor_(nullptr), is_end_(true) {}
        
        iterator(MDB_cursor* cursor, bool end = false) : cursor_(cursor), is_end_(end) {
            if (!is_end_ && cursor_) {
                update_current();
            }
        }

        ~iterator() {
            if (cursor_) mdb_cursor_close(cursor_);
        }

        iterator(const iterator& other) {
            copy_from(other);
        }

        iterator& operator=(const iterator& other) {
            if (this != &other) {
                if (cursor_) mdb_cursor_close(cursor_);
                copy_from(other);
            }
            return *this;
        }

        iterator& operator++() {
            if (is_end_ || !cursor_) return *this;
            MDB_val k, v;
            int rc = mdb_cursor_get(cursor_, &k, &v, MDB_NEXT);
            if (rc == MDB_NOTFOUND) {
                is_end_ = true;
            } else if (rc != 0) {
                throw std::runtime_error(mdb_strerror(rc));
            } else {
                update_current();
            }
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const iterator& other) const {
            if (is_end_ && other.is_end_) return true;
            if (is_end_ || other.is_end_) return false;
            return current_ == other.current_;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

        reference operator*() { return current_; }
        pointer operator->() { return &current_; }

    private:
        MDB_cursor* cursor_ = nullptr;
        bool is_end_ = true;
        value_type current_;

        void copy_from(const iterator& other) {
            is_end_ = other.is_end_;
            if (other.cursor_) {
                int rc = mdb_cursor_open(mdb_cursor_txn(other.cursor_), mdb_cursor_dbi(other.cursor_), &cursor_);
                if (rc != 0) throw std::runtime_error("Failed to duplicate cursor");
                if (!is_end_) {
                    MDB_val k, v;
                    rc = mdb_cursor_get(other.cursor_, &k, &v, MDB_GET_CURRENT);
                    if (rc == 0) {
                        mdb_cursor_get(cursor_, &k, &v, MDB_GET_BOTH);
                        update_current();
                    } else {
                        is_end_ = true;
                    }
                }
            } else {
                cursor_ = nullptr;
            }
        }

        void update_current() {
            MDB_val k, v;
            int rc = mdb_cursor_get(cursor_, &k, &v, MDB_GET_CURRENT);
            if (rc == 0) {
                current_.first = deserialize<Key>(k);
                current_.second = deserialize<T>(v);
            }
        }
    };

    iterator begin(transaction& txn) {
        MDB_cursor* cursor;
        int rc = mdb_cursor_open(txn, dbi_, &cursor);
        if (rc != 0) throw std::runtime_error(mdb_strerror(rc));
        
        MDB_val k, v;
        rc = mdb_cursor_get(cursor, &k, &v, MDB_FIRST);
        if (rc == MDB_NOTFOUND) {
            mdb_cursor_close(cursor);
            return end(txn);
        }
        if (rc != 0) {
            mdb_cursor_close(cursor);
            throw std::runtime_error(mdb_strerror(rc));
        }
        return iterator(cursor, false);
    }

    iterator end(transaction& txn) {
        return iterator(nullptr, true);
    }

    iterator find(transaction& txn, const Key& key) {
        MDB_cursor* cursor;
        int rc = mdb_cursor_open(txn, dbi_, &cursor);
        if (rc != 0) throw std::runtime_error(mdb_strerror(rc));

        std::string k = serialize(key);
        MDB_val key_val{k.size(), k.data()};
        MDB_val data_val;
        
        rc = mdb_cursor_get(cursor, &key_val, &data_val, MDB_SET);
        if (rc == MDB_NOTFOUND) {
            mdb_cursor_close(cursor);
            return end(txn);
        }
        if (rc != 0) {
            mdb_cursor_close(cursor);
            throw std::runtime_error(mdb_strerror(rc));
        }
        return iterator(cursor, false);
    }

    iterator lower_bound(transaction& txn, const Key& key) {
        MDB_cursor* cursor;
        int rc = mdb_cursor_open(txn, dbi_, &cursor);
        if (rc != 0) throw std::runtime_error(mdb_strerror(rc));

        std::string k = serialize(key);
        MDB_val key_val{k.size(), k.data()};
        MDB_val data_val;
        
        rc = mdb_cursor_get(cursor, &key_val, &data_val, MDB_SET_RANGE);
        if (rc == MDB_NOTFOUND) {
            mdb_cursor_close(cursor);
            return end(txn);
        }
        if (rc != 0) {
            mdb_cursor_close(cursor);
            throw std::runtime_error(mdb_strerror(rc));
        }
        return iterator(cursor, false);
    }

    iterator upper_bound(transaction& txn, const Key& key) {
        MDB_cursor* cursor;
        int rc = mdb_cursor_open(txn, dbi_, &cursor);
        if (rc != 0) throw std::runtime_error(mdb_strerror(rc));

        std::string k = serialize(key);
        MDB_val key_val{k.size(), k.data()};
        MDB_val data_val;
        
        rc = mdb_cursor_get(cursor, &key_val, &data_val, MDB_SET_RANGE);
        if (rc == MDB_NOTFOUND) {
            mdb_cursor_close(cursor);
            return end(txn);
        }
        if (rc != 0) {
            mdb_cursor_close(cursor);
            throw std::runtime_error(mdb_strerror(rc));
        }

        if (key_val.mv_size == k.size() && std::memcmp(key_val.mv_data, k.data(), k.size()) == 0) {
            rc = mdb_cursor_get(cursor, &key_val, &data_val, MDB_NEXT_NODUP);
            if (rc == MDB_NOTFOUND) {
                mdb_cursor_close(cursor);
                return end(txn);
            }
            if (rc != 0) {
                mdb_cursor_close(cursor);
                throw std::runtime_error(mdb_strerror(rc));
            }
        }
        
        return iterator(cursor, false);
    }

    std::pair<iterator, iterator> equal_range(transaction& txn, const Key& key) {
        return {lower_bound(txn, key), upper_bound(txn, key)};
    }

private:
    environment& env_;
    MDB_dbi dbi_;
};

}
