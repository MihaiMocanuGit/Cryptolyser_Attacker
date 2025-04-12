#include "../SampleData.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"

namespace
{
constexpr double tolerance = 0.0000001;
using Catch::Matchers::WithinRel;
} // namespace

TEST_CASE("SampleData creation", "[SampleData]")
{
    SECTION("Default constructor")
    {
        Old::SampleData<double> sampleData = {};
        REQUIRE(sampleData.data().empty());
        REQUIRE(sampleData.begin() == sampleData.end());

        SECTION("Metrics for empty sample")
        {
            const auto &metrics{sampleData.metrics()};
            REQUIRE(metrics.size == 0);
            REQUIRE(metrics.sum == 0);
            REQUIRE(metrics.mean == 0);
            REQUIRE(metrics.variance == 0);
            REQUIRE(metrics.stdDev == 0);
        }
        SECTION("Adding element")
        {
            constexpr double value = 1.0 / 8.0; // should be representable as a double with no loss.
            sampleData.insert(value);
            REQUIRE(sampleData.size() == 1);
            REQUIRE(sampleData[0] == value);
            REQUIRE(++sampleData.begin() == sampleData.end());

            const auto &metrics{sampleData.metrics()};
            REQUIRE(metrics.size == 1);
            REQUIRE(metrics.sum == value);
            REQUIRE(metrics.mean == value);
            REQUIRE(metrics.variance == 0);
            REQUIRE(metrics.stdDev == 0);
        }
    }
    SECTION("Vector constructor")
    {
        SECTION("Lvalue Vector")
        {
            std::vector<double> data = {1, 2, 3, 4, 5};
            Old::SampleData<double> sampleData{data};
            REQUIRE(sampleData.size() == data.size());
            REQUIRE(sampleData.begin() + data.size() == sampleData.end());

            SECTION("Metrics for sample")
            {
                const auto &metrics{sampleData.metrics()};
                REQUIRE(metrics.size == data.size());
                REQUIRE(metrics.sum == 1 + 2 + 3 + 4 + 5);
                REQUIRE(metrics.mean == metrics.sum / metrics.size);
                REQUIRE_THAT(metrics.variance, WithinRel(2.5, tolerance));
                REQUIRE_THAT(metrics.stdDev, WithinRel(1.58113883008, tolerance));
            }
        }
        SECTION("Rvalue Vector")
        {
            std::vector<double> data = {1, 2, 3, 4, 5};
            constexpr size_t size = 5;
            Old::SampleData<double> sampleData{std::move(data)};
            REQUIRE(sampleData.size() == size);
            REQUIRE(sampleData.begin() + size == sampleData.end());

            SECTION("Metrics for sample")
            {
                const auto &metrics{sampleData.metrics()};
                REQUIRE(metrics.size == size);
                REQUIRE(metrics.sum == 1 + 2 + 3 + 4 + 5);
                REQUIRE(metrics.mean == metrics.sum / metrics.size);
                REQUIRE_THAT(metrics.variance, WithinRel(2.5, tolerance));
                REQUIRE_THAT(metrics.stdDev, WithinRel(1.5811388, tolerance));
            }
        }
    }
    SECTION("Iterator Range constructor")
    {
        std::vector<double> data = {1, 2, 3, 4, 5};
        Old::SampleData<double> sampleData{data.begin(), data.end()};
        REQUIRE(sampleData.size() == data.size());
        REQUIRE(sampleData.begin() + data.size() == sampleData.end());

        SECTION("Metrics for sample")
        {
            const auto &metrics{sampleData.metrics()};
            REQUIRE(metrics.size == data.size());
            REQUIRE(metrics.sum == 1 + 2 + 3 + 4 + 5);
            REQUIRE(metrics.mean == metrics.sum / metrics.size);
            REQUIRE_THAT(metrics.variance, Catch::Matchers::WithinRel(2.5, tolerance));
            REQUIRE_THAT(metrics.stdDev, Catch::Matchers::WithinRel(1.5811388, tolerance));
        }
    }
}

TEST_CASE("Metric Computation from SampleData", "[SampleData]")
{
    SECTION("Basic values")
    {
        std::vector<double> data(100);
        std::ranges::fill(data, std::numeric_limits<uint16_t>::max());
        Old::SampleData<double> sample{data};

        const auto &metrics{sample.metrics()};
        for (size_t count = 1; count <= 100; ++count)
        {
            REQUIRE(metrics.size == count * data.size());
            REQUIRE_THAT(
                metrics.sum,
                WithinRel(count * std::numeric_limits<uint16_t>::max() * data.size(), tolerance));
            REQUIRE_THAT(metrics.mean, WithinRel(std::numeric_limits<uint16_t>::max(), tolerance));
            REQUIRE(metrics.variance == 0);
            REQUIRE(metrics.stdDev == 0);

            sample.insert(data.begin(), data.end());
        }
    }
    SECTION("PSEUDO-RANDOM VALUES")
    {
        Old::SampleData<double> totalSample;
        for (size_t count = 1; count <= 5; ++count)
        {
            std::vector<double> data(100);
            std::generate(data.begin(), data.end(),
                          []() -> double
                          {
                              // Simple pseudo-random generator:
                              // https://en.wikipedia.org/wiki/Linear_congruential_generator#Parameters_in_common_use
                              static size_t seed = 123456789;
                              constexpr size_t a = 1103515245;
                              constexpr size_t c = 12345;
                              constexpr size_t m = 1 << 31;
                              seed = (a * seed + c) % m;
                              size_t wholePart = seed % std::numeric_limits<uint16_t>::max();
                              seed = (a * seed + c) % m;
                              size_t fractionalPart =
                                  seed % std::numeric_limits<uint8_t>::max() + 1;
                              return static_cast<double>(wholePart) +
                                     1.0 / static_cast<double>(fractionalPart);
                          });
            // The metrics for the current generated data.
            Old::SampleData<double> currentSample{data};
            REQUIRE(currentSample.size() == data.size());
            REQUIRE(currentSample.begin() + data.size() == currentSample.end());
            const auto &currentMetrics{currentSample.metrics()};
            REQUIRE(currentMetrics.size == data.size());
            const double currentTargetSum =
                std::accumulate(currentSample.begin(), currentSample.end(), 0.0);
            REQUIRE_THAT(currentMetrics.sum, WithinRel(currentTargetSum, tolerance));
            REQUIRE_THAT(currentMetrics.mean,
                         WithinRel(currentMetrics.sum / currentMetrics.size, tolerance));
            double currentVariance = 0;
            for (double x : currentSample)
            {
                currentVariance += (x - currentMetrics.mean) * (x - currentMetrics.mean);
            }
            currentVariance /= (currentMetrics.size - 1);
            REQUIRE_THAT(currentMetrics.variance, WithinRel(currentVariance, tolerance));

            // The metrics for the combined new and old data;
            totalSample.insert(data.begin(), data.end());
            REQUIRE(totalSample.size() == count * data.size());
            REQUIRE(totalSample.begin() + count * data.size() == totalSample.end());
            const auto &totalMetrics{totalSample.metrics()};
            REQUIRE(totalMetrics.size == count * data.size());
            const double targetSum = std::accumulate(totalSample.begin(), totalSample.end(), 0.0);
            REQUIRE_THAT(totalMetrics.sum, WithinRel(targetSum, tolerance));
            REQUIRE_THAT(totalMetrics.mean,
                         WithinRel(totalMetrics.sum / totalMetrics.size, tolerance));
            double totalVariance = 0;
            for (double x : totalSample)
            {
                totalVariance += (x - totalMetrics.mean) * (x - totalMetrics.mean);
            }
            totalVariance /= (totalMetrics.size - 1);
            REQUIRE_THAT(totalMetrics.variance, WithinRel(totalVariance, tolerance));
        }
    }
}
