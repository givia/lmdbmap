#include <lmdbmap/multimap.hpp>
#include <lmdbmap/environment.hpp>
#include <lmdbmap/transaction.hpp>
#include <iostream>
#include <filesystem>

int main() {
    try {
        std::filesystem::remove_all("example_db_multimap");
        lmdbmap::environment env("example_db_multimap");
        lmdbmap::multimap<std::string, int> m(env, "my_multimap");

        {
            lmdbmap::transaction txn(env);
            m.insert(txn, "key1", 10);
            m.insert(txn, "key1", 20);
            m.insert(txn, "key2", 30);
            txn.commit();
        }

        {
            lmdbmap::transaction txn(env, true);
            auto vals = m.get(txn, "key1");
            std::cout << "key1 values:" << std::endl;
            for (const auto& v : vals) {
                std::cout << v << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
