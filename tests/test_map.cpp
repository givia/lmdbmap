#include <gtest/gtest.h>
#include <lmdbmap/map.hpp>
#include <lmdbmap/environment.hpp>
#include <lmdbmap/transaction.hpp>
#include <filesystem>

class MapTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::filesystem::remove_all("test_db_map");
        env = std::make_unique<lmdbmap::environment>("test_db_map");
    }

    void TearDown() override {
        env.reset();
        std::filesystem::remove_all("test_db_map");
    }

    std::unique_ptr<lmdbmap::environment> env;
};

TEST_F(MapTest, InsertAndGet) {
    lmdbmap::map<int, std::string> m(*env, "map1");
    {
        lmdbmap::transaction txn(*env);
        m.insert(txn, 1, "one");
        m.insert(txn, 2, "two");
        txn.commit();
    }
    {
        lmdbmap::transaction txn(*env, true);
        auto v1 = m.get(txn, 1);
        ASSERT_TRUE(v1.has_value());
        EXPECT_EQ(*v1, "one");
        
        auto v2 = m.get(txn, 2);
        ASSERT_TRUE(v2.has_value());
        EXPECT_EQ(*v2, "two");
        
        auto v3 = m.get(txn, 3);
        ASSERT_FALSE(v3.has_value());
    }
}

TEST_F(MapTest, Iterator) {
    lmdbmap::map<int, std::string> m(*env, "map2");
    {
        lmdbmap::transaction txn(*env);
        m.insert(txn, 1, "one");
        m.insert(txn, 3, "three");
        m.insert(txn, 2, "two");
        txn.commit();
    }
    {
        lmdbmap::transaction txn(*env, true);
        auto it = m.begin(txn);
        ASSERT_NE(it, m.end(txn));
        EXPECT_EQ(it->first, 1);
        EXPECT_EQ(it->second, "one");
        
        ++it;
        ASSERT_NE(it, m.end(txn));
        EXPECT_EQ(it->first, 2);
        EXPECT_EQ(it->second, "two");
        
        ++it;
        ASSERT_NE(it, m.end(txn));
        EXPECT_EQ(it->first, 3);
        EXPECT_EQ(it->second, "three");
        
        ++it;
        EXPECT_EQ(it, m.end(txn));
    }
}

TEST_F(MapTest, Find) {
    lmdbmap::map<int, std::string> m(*env, "map3");
    {
        lmdbmap::transaction txn(*env);
        m.insert(txn, 10, "ten");
        txn.commit();
    }
    {
        lmdbmap::transaction txn(*env, true);
        auto it = m.find(txn, 10);
        ASSERT_NE(it, m.end(txn));
        EXPECT_EQ(it->second, "ten");
        
        auto it2 = m.find(txn, 20);
        EXPECT_EQ(it2, m.end(txn));
    }
}

TEST_F(MapTest, Range) {
    lmdbmap::map<int, std::string> m(*env, "map_range");
    {
        lmdbmap::transaction txn(*env);
        m.insert(txn, 10, "ten");
        m.insert(txn, 20, "twenty");
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
        
        range = m.equal_range(txn, 25);
        EXPECT_EQ(range.first->first, 30);
        EXPECT_EQ(range.second->first, 30);
    }
}
