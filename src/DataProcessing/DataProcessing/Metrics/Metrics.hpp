#ifndef CRYPTOLYSER_ATTACKER_METRICS_HPP
#define CRYPTOLYSER_ATTACKER_METRICS_HPP

#include <cassert>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <numeric>

template <typename T>
concept Real = std::floating_point<T> || std::integral<T>;

template <Real R>
struct Metrics
{
    R sum {0};
    size_t size {0};
    R mean {0};
    R variance {0};
    R stdDev {0};
    R min {std::numeric_limits<R>::max()};
    R max {std::numeric_limits<R>::min()};

    Metrics() = default;
    Metrics(R sum, size_t size, R mean, R variance, R stdDev, R min, R max);

    /**
     * Structural function used to satisfy the HasMetric concept.
     */
    const Metrics<R> &globalMetric() const noexcept;

    static Metrics combineMetrics(const Metrics &metric1, const Metrics &metric2) noexcept;

    template <typename InputIterator>
    static Metrics compute(InputIterator begin, InputIterator end);
};

template <Real Real_t>
template <typename InputIterator>
Metrics<Real_t> Metrics<Real_t>::compute(InputIterator begin, InputIterator end)
{
    assert(begin < end);
    size_t aSize {0};
    if (end > begin)
        aSize = end - begin;

    if (aSize == 0)
        return {};
    if (aSize == 1)
    {
        Real_t beginCnv {static_cast<Real_t>(*begin)};
        return {beginCnv, 1, beginCnv, 0, 0, beginCnv, beginCnv};
    }

    Real_t aSum {std::accumulate(begin, begin + aSize, static_cast<Real_t>(0.0))};
    Real_t aMean {aSum / static_cast<Real_t>(aSize)};
    Real_t squareSum {0};
    Real_t aMin {std::numeric_limits<Real_t>::max()};
    Real_t aMax {std::numeric_limits<Real_t>::min()};
    for (auto it {begin}; it != end; ++it)
    {
        Real_t currentElement {static_cast<Real_t>(*it)};
        squareSum += (currentElement - aMean) * (currentElement - aMean);
        if (currentElement < aMin)
            aMin = currentElement;
        if (currentElement > aMax)
            aMax = currentElement;
    }
    Real_t aVariance {squareSum / static_cast<Real_t>(aSize - 1)};
    Real_t aStdDev {static_cast<Real_t>(std::sqrtl(aVariance))};
    return {aSum, aSize, aMean, aVariance, aStdDev, aMin, aMax};
}

template <Real Real_t>
Metrics<Real_t>::Metrics(Real_t sum, size_t size, Real_t mean, Real_t variance, Real_t stdDev,
                         Real_t min, Real_t max)
    : sum {sum}, size {size}, mean {mean}, variance {variance}, stdDev {stdDev}, min {min},
      max {max}
{
}

template <typename T>
concept HasMetric = requires(T a) {
    { a.globalMetric() } -> std::convertible_to<Metrics<double>>;
};

#endif // CRYPTOLYSER_ATTACKER_METRICS_HPP
