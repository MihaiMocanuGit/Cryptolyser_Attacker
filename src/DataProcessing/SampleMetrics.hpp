#ifndef CRYPTOLYSER_ATTACKER_SAMPLEMETRICS_HPP
#define CRYPTOLYSER_ATTACKER_SAMPLEMETRICS_HPP

#include <cmath>
#include <cstdint>
#include <numeric>

template <typename Real_t>
struct SampleMetrics
{
    Real_t sum{0};
    size_t size{0};
    Real_t mean{0};
    Real_t variance{0};
    Real_t stdDev{0};

    SampleMetrics() = default;
    SampleMetrics(Real_t sum, size_t size, Real_t mean, Real_t variance, Real_t stdDev)
        : sum{sum}, size{size}, mean{mean}, variance{variance}, stdDev{stdDev}
    {
    }
    static SampleMetrics<Real_t> combineMetrics(const SampleMetrics &metric1,
                                                const SampleMetrics &metric2) noexcept;
    template <typename InputIterator>
    static SampleMetrics<Real_t> compute(InputIterator begin, InputIterator end)
    {
        size_t aSize{0};
        if (end > begin)
            aSize = end - begin;

        if (aSize == 0)
            return {};
        if (aSize == 1)
            return {*begin, 1, *begin, 0, 0};

        Real_t aSum{std::accumulate(begin, begin + aSize, static_cast<Real_t>(0.0))};
        // TODO: while there may be overflow, we should still be able to compute a valid mean
        Real_t aMean{aSum / static_cast<Real_t>(aSize)};
        Real_t squareSum{0};
        for (auto it{begin}; it != end; ++it)
        {
            squareSum += (*it - aMean) * (*it - aMean);
        }
        Real_t aVariance{squareSum / static_cast<Real_t>(aSize - 1)};
        Real_t aStdDev{static_cast<Real_t>(std::sqrtl(aVariance))};
        return {aSum, aSize, aMean, aVariance, aStdDev};
    }
};

#endif // CRYPTOLYSER_ATTACKER_SAMPLEMETRICS_HPP
