#pragma once
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <sstream>
#include <string>
#include <lmdb.h>

namespace lmdbmap {

template<typename T>
std::string serialize(const T& obj) {
    std::ostringstream oss;
    boost::archive::binary_oarchive oa(oss);
    oa << obj;
    return oss.str();
}

template<typename T>
T deserialize(const void* data, size_t size) {
    std::string s(static_cast<const char*>(data), size);
    std::istringstream iss(s);
    boost::archive::binary_iarchive ia(iss);
    T obj;
    ia >> obj;
    return obj;
}

template<typename T>
T deserialize(const MDB_val& val) {
    return deserialize<T>(val.mv_data, val.mv_size);
}

}
