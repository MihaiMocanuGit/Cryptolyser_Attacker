#include "../../../cmake-build-release/_deps/catch2-src/src/catch2/matchers/catch_matchers.hpp"
#include "SampleGroup.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"

namespace
{
constexpr double tolerance = 0.0000001;
using Catch::Matchers::WithinRel;
} // namespace

TEST_CASE("SampleGroup creation", "[SampleGroup]")
{
    SECTION("Default constructor")
    {
        SampleGroup<double> group;

        REQUIRE(group.size() == 0);
        REQUIRE(group.begin() == group.end());

        SECTION("Resizing")
        {
            group.resize(10);
            REQUIRE(group.size() == 10);
            group.resize(20);
            REQUIRE(group.size() == 20);
            group.resize(10);
            REQUIRE(group.size() == 10);
            group.resize(5);
            REQUIRE(group.size() == 5);

            REQUIRE(group[0].data().empty());
        }
    }

    SECTION("Size constructor")
    {
        SampleGroup<double> group{256};

        REQUIRE(group.size() == 256);
        REQUIRE(group.begin() + 256 == group.end());
        REQUIRE(group[0].data().empty());
        REQUIRE(group[255].data().empty());
    }
}

TEST_CASE("Metric Computation", "[SampleGroup]")
{
    SECTION("Simple values")
    {
        SECTION("Homogenous data")
        {
            SampleGroup<double> group{256};
            constexpr size_t sampleSize{100};
            constexpr size_t firstValue{std::numeric_limits<uint16_t>::max()};
            // start with one set of constant values
            for (size_t i{0}; i < group.size(); i++)
            {
                std::vector<double> data(sampleSize);
                std::ranges::fill(data, firstValue);
                group.insert(i, data.begin(), data.end());

                const SampleMetrics<double> &localMetrics = group.localMetrics(i);
                REQUIRE(localMetrics.mean == firstValue);
                REQUIRE(localMetrics.size == sampleSize);
                REQUIRE(localMetrics.variance == 0.0);
            }
            // add another set of constant values
            constexpr size_t secondValue{0};
            for (size_t i{0}; i < group.size(); i++)
            {
                std::vector<double> data(sampleSize);
                std::ranges::fill(data, secondValue);
                group.insert(i, data.begin(), data.end());

                const SampleMetrics<double> &localMetrics = group.localMetrics(i);
                REQUIRE_THAT(
                    localMetrics.mean,
                    WithinRel(static_cast<double>(firstValue + secondValue) / 2.0, tolerance));
                REQUIRE(localMetrics.size == 2 * sampleSize);
                double targetVariance{static_cast<double>(firstValue + secondValue) / 2.0};
                targetVariance *= targetVariance;
                targetVariance *= sampleSize * 2.0;
                targetVariance /= sampleSize * 2 - 1;
                REQUIRE_THAT(localMetrics.variance, WithinRel(targetVariance, tolerance));
            }
        }

        SECTION("2-Valued set")
        {
            SampleGroup<double> group{100};
            constexpr size_t sampleSize{100};
            constexpr double firstValue{static_cast<double>(std::numeric_limits<uint16_t>::max())};
            // populate the first half with a set of constant values
            for (size_t i{0}; i < 50; i++)
            {
                std::vector<double> data(sampleSize);
                std::ranges::fill(data, firstValue);
                group.insert(i, data.begin(), data.end());

                const SampleMetrics<double> &localMetrics = group.localMetrics(i);
                REQUIRE(localMetrics.mean == firstValue);
                REQUIRE(localMetrics.size == sampleSize);
                REQUIRE(localMetrics.variance == 0.0);
            }
            // populate the second half with another set of constant values
            constexpr double secondValue{0.0};
            for (size_t i{50}; i < 100; i++)
            {
                std::vector<double> data(sampleSize);
                std::ranges::fill(data, secondValue);
                group.insert(i, data.begin(), data.end());

                const SampleMetrics<double> &localMetrics = group.localMetrics(i);
                REQUIRE(localMetrics.mean == secondValue);
                REQUIRE(localMetrics.size == sampleSize);
                REQUIRE(localMetrics.variance == 0.0);
            }
            std::vector<double> dataList;
            dataList.reserve(sampleSize * 100);
            for (const auto &sample : group)
                for (const auto &data : sample)
                    dataList.push_back(data);
            SampleMetrics<double> targetGlobalMetrics =
                SampleMetrics<double>::compute(dataList.begin(), dataList.end());

            REQUIRE_THAT(group.globalMetrics().mean,
                         WithinRel(targetGlobalMetrics.mean, tolerance));
            REQUIRE_THAT(group.globalMetrics().variance,
                         WithinRel(targetGlobalMetrics.variance, tolerance));
        }
    }
}
