#include "SampleGroup.hpp"

#include <array>

template <typename Real_t>
void SampleGroup<Real_t>::m_updateGlobalMetrics(SampleMetrics<Real_t> oldLocalMetrics,
                                                SampleMetrics<Real_t> newLocalMetrics)
{
    // Original global metrics
    size_t n{m_globalMetrics.size};
    Real_t sum{m_globalMetrics.sum};
    Real_t u{m_globalMetrics.mean};
    Real_t v{m_globalMetrics.variance};

    // Old sample metrics.
    size_t pi_m{oldLocalMetrics.size};
    Real_t sum_m{oldLocalMetrics.sum};
    Real_t u_m{oldLocalMetrics.mean};
    Real_t v_m{oldLocalMetrics.variance};

    // New sample metrics.
    size_t pi_prime_m{newLocalMetrics.size};
    Real_t sum_prime_m{newLocalMetrics.sum};
    Real_t u_prime_m{newLocalMetrics.mean};
    Real_t v_prime_m{newLocalMetrics.variance};

    // Updated global metrics.
    size_t n_prime{n - pi_m + pi_prime_m};
    Real_t sum_prime{sum - sum_m + sum_prime_m};
    Real_t u_prime{(static_cast<Real_t>(n) * u + static_cast<Real_t>(pi_prime_m) * u_prime_m -
                    static_cast<Real_t>(pi_m) * u_m) /
                   static_cast<Real_t>(n_prime)};
    Real_t SS_m{v_m * static_cast<Real_t>(pi_m - 1) +
                static_cast<Real_t>(pi_m) * (u_m - u) * (u_m - u)};
    Real_t SS_prime_m{v_prime_m * static_cast<Real_t>(pi_prime_m - 1) +
                      static_cast<Real_t>(pi_prime_m) * (u_prime_m - u_prime) *
                          (u_prime_m - u_prime)};

    Real_t v_prime;
    if (n_prime - 1 == 0)
    {
        v_prime = v_prime_m;
    }
    else
    {
        v_prime =
            (static_cast<Real_t>(n - 1) * v + SS_prime_m - SS_m +
             (u - u_prime) * (2 * static_cast<Real_t>(n) * u - static_cast<Real_t>(pi_m) * u_m -
                              (u + u_prime) * static_cast<Real_t>(n - pi_m))) /
            static_cast<Real_t>(n_prime - 1);
    }

    Real_t s_prime{static_cast<Real_t>(std::sqrtl(v_prime))};

    m_globalMetrics = SampleMetrics<Real_t>{sum_prime, n_prime, u_prime, v_prime, s_prime};
}

template <typename Real_t>
SampleGroup<Real_t>::SampleGroup(size_t size) : m_samples{size}
{
}

template <typename Real_t>
SampleGroup<Real_t>::SampleGroup(size_t size, size_t reserveSize) : SampleGroup{size}
{
    this->reserveForAll(reserveSize);
}

template <typename Real_t>
void SampleGroup<Real_t>::reserveForAll(size_t size)
{
    for (auto &sample : m_samples)
        sample.reserve(size);
}

template <typename Real_t>
void SampleGroup<Real_t>::insert(size_t index, Real_t data)
{
    std::array<Real_t, 1> tmpArr{{data}};
    this->insert(index, std::begin(tmpArr), std::end(tmpArr));
}

template <typename Real_t>
const SampleMetrics<Real_t> &SampleGroup<Real_t>::globalMetrics() const noexcept
{
    return m_globalMetrics;
}

template <typename Real_t>
const SampleMetrics<Real_t> &SampleGroup<Real_t>::localMetrics(size_t index) const
{
    return m_samples[index].metrics();
}

template <typename Real_t>
SampleMetrics<Real_t> SampleGroup<Real_t>::standardizeLocalMetrics(size_t index) const
{
    SampleMetrics<Real_t> standardized{this->localMetrics(index)};
    standardized.mean = (standardized.mean - m_globalMetrics.mean) /
                        (m_globalMetrics.stdDev / std::sqrtl(m_globalMetrics.size));
    standardized.variance =
        (standardized.size - 1) * standardized.variance / m_globalMetrics.variance;
    standardized.stdDev = std::sqrtl(standardized.variance);
    return standardized;
}

template <typename Real_t>
const SampleData<Real_t> &SampleGroup<Real_t>::sampleAt(size_t index) const
{
    return m_samples[index];
}

template <typename Real_t>
const SampleData<Real_t> &SampleGroup<Real_t>::operator[](size_t index) const
{
    return m_samples[index];
}

template <typename Real_t>
size_t SampleGroup<Real_t>::size() const noexcept
{
    return m_samples.size();
}

template <typename Real_t>
void SampleGroup<Real_t>::resize(size_t size)
{
    m_samples.resize(size);
}

template <typename Real_t>
typename std::vector<SampleData<Real_t>>::const_iterator SampleGroup<Real_t>::begin() const noexcept
{
    return m_samples.begin();
}

template <typename Real_t>
typename std::vector<SampleData<Real_t>>::const_iterator SampleGroup<Real_t>::end() const noexcept
{
    return m_samples.end();
}

template class SampleGroup<int>;
template class SampleGroup<uint64_t>;
template class SampleGroup<float>;
template class SampleGroup<double>;
template class SampleGroup<long double>;
