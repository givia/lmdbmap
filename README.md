# lmdbmap

![CI](https://github.com/givia/lmdbmap/actions/workflows/ci.yml/badge.svg)
![License](https://img.shields.io/badge/license-MIT-blue.svg)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)

**lmdbmap** is a high-performance C++ wrapper for [LMDB](https://www.symas.com/lmdb) (Lightning Memory-Mapped Database) that provides a familiar `std::map` and `std::multimap` like interface. It seamlessly integrates with [Boost.Serialization](https://www.boost.org/doc/libs/release/libs/serialization/) to allow storing arbitrary C++ objects as keys and values.

## Project Description

This library bridges the gap between the raw, low-level C API of LMDB and the idiomatic, type-safe C++ STL containers. It is designed for applications that require:
- **Persistence**: Data survives process restarts.
- **Performance**: Leveraging LMDB's memory-mapping for read speeds comparable to in-memory hash tables.
- **Ease of Use**: Drop-in replacement for `std::map` in many contexts (with transaction awareness).
- **Type Safety**: Automatic serialization/deserialization of complex types.

## Tags / Topics

`cpp`, `cplusplus`, `lmdb`, `database`, `key-value-store`, `persistence`, `serialization`, `boost`, `map`, `multimap`, `embedded-database`, `high-performance`

## Features

- **std-like API**: `insert`, `find`, `erase`, `begin`, `end`, `lower_bound`, `upper_bound`, `equal_range`.
- **Persistence**: Data is stored in LMDB (Lightning Memory-Mapped Database).
- **Serialization**: Automatic binary serialization of keys and values using Boost.Serialization.
- **Transactions**: Explicit transaction management for efficiency and consistency.
- **Range Support**: Efficient range queries using LMDB cursors.

## Dependencies

- C++17 compiler
- [LMDB](https://www.symas.com/lmdb)
- [Boost.Serialization](https://www.boost.org/doc/libs/release/libs/serialization/)
- [Boost.System](https://www.boost.org/doc/libs/release/libs/system/)
- [Boost.Filesystem](https://www.boost.org/doc/libs/release/libs/filesystem/)

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Testing

```bash
cd build
ctest --output-on-failure
```

## Usage

### CMake Integration

#### Option 1: Installed Package (Debian/Ubuntu)

If you have installed the library via the `.deb` package:

```cmake
find_package(lmdbmap REQUIRED)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE lmdbmap::lmdbmap)
```

#### Option 2: FetchContent

You can include `lmdbmap` directly in your project using CMake's `FetchContent`:

```cmake
include(FetchContent)

FetchContent_Declare(
  lmdbmap
  GIT_REPOSITORY https://github.com/givia/lmdbmap.git
  GIT_TAG v0.2.0
)
FetchContent_MakeAvailable(lmdbmap)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE lmdbmap)
```

### Map

```cpp
#include <lmdbmap/map.hpp>
#include <lmdbmap/environment.hpp>
#include <lmdbmap/transaction.hpp>
#include <iostream>

int main() {
    // Create environment
    lmdbmap::environment env("my_db");
    
    // Create map
    lmdbmap::map<int, std::string> m(env, "my_map");

    // Write transaction
    {
        lmdbmap::transaction txn(env);
        m.insert(txn, 1, "Hello");
        m.insert(txn, 2, "World");
        txn.commit();
    }

    // Read transaction
    {
        lmdbmap::transaction txn(env, true); // Read-only transaction
        auto it = m.find(txn, 1);
        if (it != m.end(txn)) {
            std::cout << it->second << std::endl;
        }
        
        // Iteration
        for (auto it = m.begin(txn); it != m.end(txn); ++it) {
            std::cout << it->first << ": " << it->second << std::endl;
        }
    }
    return 0;
}
```

### Multimap

```cpp
#include <lmdbmap/multimap.hpp>

// ... setup env ...
lmdbmap::multimap<std::string, int> m(env, "my_multimap");

{
    lmdbmap::transaction txn(env);
    m.insert(txn, "key1", 10);
    m.insert(txn, "key1", 20);
    txn.commit();
}
```

## Benchmarks

The project includes benchmarks using Google Benchmark.

```bash
./benchmarks/benchmark_map
```

## License

MIT License
