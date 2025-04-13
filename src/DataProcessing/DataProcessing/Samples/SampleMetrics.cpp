#include "SampleMetrics.hpp"

#include <cstdint>

template <typename Real_t>
SampleMetrics<Real_t> SampleMetrics<Real_t>::combineMetrics(const SampleMetrics &metric1,
                                                            const SampleMetrics &metric2) noexcept
{
    if (metric1.size == 0)
        return metric2;
    if (metric2.size == 0)
        return metric1;

    Real_t sum{metric1.sum + metric2.sum};
    size_t size{metric1.size + metric2.size};
    Real_t mean{(static_cast<Real_t>(metric1.size) * metric1.mean +
                 static_cast<Real_t>(metric2.size) * metric2.mean) /
                static_cast<Real_t>(metric1.size + metric2.size)};
    // https://math.stackexchange.com/questions/2971315/how-do-i-combine-standard-deviations-of-two-groups
    Real_t numerator1{static_cast<Real_t>(metric1.size - 1) * metric1.variance +
                      static_cast<Real_t>(metric2.size - 1) * metric2.variance};
    Real_t denominator1{static_cast<Real_t>(metric1.size + metric2.size - 1)};

    Real_t numerator2{static_cast<Real_t>(metric1.size * metric2.size) *
                      (metric1.mean - metric2.mean) * (metric1.mean - metric2.mean)};
    Real_t denominator2{
        static_cast<Real_t>((metric1.size + metric2.size) * (metric1.size + metric2.size - 1))};
    Real_t variance{numerator1 / denominator1 + numerator2 / denominator2};
    Real_t stdDev{static_cast<Real_t>(std::sqrtl(variance))};
    Real_t min{std::min(metric1.min, metric2.min)};
    Real_t max{std::max(metric1.max, metric2.max)};
    return {sum, size, mean, variance, stdDev, min, max};
}

template <typename Real_t>
const SampleMetrics<Real_t> &SampleMetrics<Real_t>::globalMetric() const noexcept
{
    return *this;
}

template struct SampleMetrics<int>;
template struct SampleMetrics<uint64_t>;
template struct SampleMetrics<float>;
template struct SampleMetrics<double>;
template struct SampleMetrics<long double>;
