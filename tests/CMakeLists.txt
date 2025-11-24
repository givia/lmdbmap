find_package(GTest REQUIRED)

add_executable(test_map test_map.cpp)
target_link_libraries(test_map lmdbmap GTest::GTest GTest::Main)
add_test(NAME test_map COMMAND test_map)

add_executable(test_multimap test_multimap.cpp)
target_link_libraries(test_multimap lmdbmap GTest::GTest GTest::Main)
add_test(NAME test_multimap COMMAND test_multimap)
