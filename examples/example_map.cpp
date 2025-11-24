#include <lmdbmap/map.hpp>
#include <lmdbmap/environment.hpp>
#include <lmdbmap/transaction.hpp>
#include <iostream>
#include <filesystem>

int main() {
    try {
        std::filesystem::remove_all("example_db_map");
        lmdbmap::environment env("example_db_map");
        lmdbmap::map<int, std::string> m(env, "my_map");

        {
            lmdbmap::transaction txn(env);
            m.insert(txn, 1, "Hello");
            m.insert(txn, 2, "World");
            txn.commit();
        }

        {
            lmdbmap::transaction txn(env, true);
            for (auto it = m.begin(txn); it != m.end(txn); ++it) {
                std::cout << it->first << ": " << it->second << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
