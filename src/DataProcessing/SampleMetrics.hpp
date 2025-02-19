#ifndef CRYPTOLYSER_ATTACKER_SAMPLEMETRICS_HPP
#define CRYPTOLYSER_ATTACKER_SAMPLEMETRICS_HPP

#include <cmath>
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
        size_t aSize =
            std::distance(begin, end); // I do not like that std::distance can be negative
        if (aSize == 0)
            return {};
        else if (aSize == 1)
            return {*begin, 1, *begin, 0, 0};

        Real_t aSum = std::accumulate(begin, end, 0); // overflow?
        Real_t aMean = aSum / aSize;
        Real_t squareSum = 0;
        for (auto it = begin; it != end; ++it)
        {
            squareSum += (*it - aMean) * (*it - aMean);
        }
        Real_t aVariance = squareSum / (aSize - 1);
        Real_t sStdDev = std::sqrt(aVariance);
        return {aSum, aSize, aMean, aVariance, sStdDev};
    }
};

#endif // CRYPTOLYSER_ATTACKER_SAMPLEMETRICS_HPP
