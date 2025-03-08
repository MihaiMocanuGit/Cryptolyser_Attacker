add_executable(UTest_Samples
        utest_SampleData.cpp
        utest_SampleGroup.cpp)
target_link_libraries(UTest_Samples PRIVATE Samples Catch2::Catch2WithMain)
catch_discover_tests(UTest_Samples)
