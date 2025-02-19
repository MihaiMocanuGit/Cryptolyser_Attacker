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

    Real_t sum = metric1.sum + metric2.sum;
    size_t size = metric1.size + metric2.size;
    Real_t mean = (static_cast<Real_t>(metric1.size) * metric1.mean +
                   static_cast<Real_t>(metric2.size) * metric2.mean) /
                  static_cast<Real_t>(metric1.size + metric2.size);
    // https://math.stackexchange.com/questions/2971315/how-do-i-combine-standard-deviations-of-two-groups
    Real_t variance = (static_cast<Real_t>(metric1.size - 1) * metric1.variance +
                       static_cast<Real_t>(metric2.size - 1) * metric2.variance) /
                      static_cast<Real_t>(metric1.size + metric2.size - 1);
    variance += static_cast<Real_t>(metric1.size * metric2.size) * (metric1.mean - metric2.mean) *
                (metric1.mean - metric2.mean) / (metric1.size + metric2.size);
    Real_t stdDev = std::sqrt(variance);

    return {sum, size, mean, variance, stdDev};
}

template struct SampleMetrics<int>;
template struct SampleMetrics<uint64_t>;
template struct SampleMetrics<float>;
template struct SampleMetrics<double>;
template struct SampleMetrics<long double>;
