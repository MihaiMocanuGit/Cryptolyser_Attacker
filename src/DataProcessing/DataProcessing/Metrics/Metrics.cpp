#include "Metrics.hpp"

#include <cstdint>

template <Real R>
Metrics<R> Metrics<R>::combineMetrics(const Metrics &metric1, const Metrics &metric2) noexcept
{
    if (metric1.size == 0)
        return metric2;
    if (metric2.size == 0)
        return metric1;

    R sum{metric1.sum + metric2.sum};
    size_t size{metric1.size + metric2.size};
    R mean{(static_cast<R>(metric1.size) * metric1.mean +
            static_cast<R>(metric2.size) * metric2.mean) /
           static_cast<R>(metric1.size + metric2.size)};
    // https://math.stackexchange.com/questions/2971315/how-do-i-combine-standard-deviations-of-two-groups
    R numerator1{static_cast<R>(metric1.size - 1) * metric1.variance +
                 static_cast<R>(metric2.size - 1) * metric2.variance};
    R denominator1{static_cast<R>(metric1.size + metric2.size - 1)};

    R numerator2{static_cast<R>(metric1.size * metric2.size) * (metric1.mean - metric2.mean) * (metric1.mean - metric2.mean)};
    R denominator2{
        static_cast<R>((metric1.size + metric2.size) * (metric1.size + metric2.size - 1))};
    R variance{numerator1 / denominator1 + numerator2 / denominator2};
    R stdDev{static_cast<R>(std::sqrtl(variance))};
    R min{std::min(metric1.min, metric2.min)};
    R max{std::max(metric1.max, metric2.max)};
    return {sum, size, mean, variance, stdDev, min, max};
}

template <Real R>
const Metrics<R> &Metrics<R>::globalMetric() const noexcept
{
    return *this;
}

template struct Metrics<int>;
template struct Metrics<uint64_t>;
template struct Metrics<float>;
template struct Metrics<double>;
template struct Metrics<long double>;
