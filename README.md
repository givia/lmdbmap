# lmdbmap

A C++ wrapper for LMDB providing a `std::map` and `std::multimap` like interface with automatic Boost.Serialization support.

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
