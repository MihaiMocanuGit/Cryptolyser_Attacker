add_executable(UTest_Samples
        utest_SampleData.cpp
        )
target_link_libraries(UTest_Samples PRIVATE Metrics Catch2::Catch2WithMain)
catch_discover_tests(UTest_Samples)
