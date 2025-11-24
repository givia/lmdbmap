#include <gtest/gtest.h>
#include <lmdbmap/multimap.hpp>
#include <lmdbmap/environment.hpp>
#include <lmdbmap/transaction.hpp>
#include <filesystem>

class MultimapTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::filesystem::remove_all("test_db_multimap");
        env = std::make_unique<lmdbmap::environment>("test_db_multimap");
    }

    void TearDown() override {
        env.reset();
        std::filesystem::remove_all("test_db_multimap");
    }

    std::unique_ptr<lmdbmap::environment> env;
};

TEST_F(MultimapTest, InsertAndGet) {
    lmdbmap::multimap<int, std::string> m(*env, "mmap1");
    {
        lmdbmap::transaction txn(*env);
        m.insert(txn, 1, "one_a");
        m.insert(txn, 1, "one_b");
        m.insert(txn, 2, "two");
        txn.commit();
    }
    {
        lmdbmap::transaction txn(*env, true);
        auto vals = m.get(txn, 1);
        ASSERT_EQ(vals.size(), 2);
        EXPECT_EQ(vals[0], "one_a");
        EXPECT_EQ(vals[1], "one_b");
    }
}

TEST_F(MultimapTest, Range) {
    lmdbmap::multimap<int, std::string> m(*env, "mmap_range");
    {
        lmdbmap::transaction txn(*env);
        m.insert(txn, 10, "ten");
        m.insert(txn, 20, "twenty_a");
        m.insert(txn, 20, "twenty_b");
        m.insert(txn, 30, "thirty");
        txn.commit();
    }
    {
        lmdbmap::transaction txn(*env, true);
        
        // lower_bound
        auto it = m.lower_bound(txn, 15);
        ASSERT_NE(it, m.end(txn));
        EXPECT_EQ(it->first, 20);
        
        it = m.lower_bound(txn, 20);
        ASSERT_NE(it, m.end(txn));
        EXPECT_EQ(it->first, 20);
        
        it = m.lower_bound(txn, 35);
        EXPECT_EQ(it, m.end(txn));
        
        // upper_bound
        it = m.upper_bound(txn, 15);
        ASSERT_NE(it, m.end(txn));
        EXPECT_EQ(it->first, 20);
        
        it = m.upper_bound(txn, 20);
        ASSERT_NE(it, m.end(txn));
        EXPECT_EQ(it->first, 30);
        
        it = m.upper_bound(txn, 30);
        EXPECT_EQ(it, m.end(txn));
        
        // equal_range
        auto range = m.equal_range(txn, 20);
        EXPECT_EQ(range.first->first, 20);
        EXPECT_EQ(range.second->first, 30);
        
        // Check that iterating from first to second gives all 20s
        int count = 0;
        for (auto i = range.first; i != range.second; ++i) {
            EXPECT_EQ(i->first, 20);
            count++;
        }
        EXPECT_EQ(count, 2);
    }
}

TEST_F(MultimapTest, Empty) {
    lmdbmap::multimap<int, std::string> m(*env, "mmap_empty");
    {
        lmdbmap::transaction txn(*env, true);
        EXPECT_TRUE(m.empty(txn));
    }
    {
        lmdbmap::transaction txn(*env);
        m.insert(txn, 1, "one");
        txn.commit();
    }
    {
        lmdbmap::transaction txn(*env, true);
        EXPECT_FALSE(m.empty(txn));
    }
}
