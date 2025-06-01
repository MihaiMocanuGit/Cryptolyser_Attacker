#ifndef CRYPTOLYSER_ATTACKER_METRICSDATA_HPP
#define CRYPTOLYSER_ATTACKER_METRICSDATA_HPP

#include "../Metrics/Metrics.hpp"

#include <array>
#include <concepts>
#include <vector>

template <Real R>
class MetricsData
{
  private:
    // Storing the global metric into an array in order to mimick the container interface of other
    // types of Data containers (SampleData, DistributionData).
    std::array<Metrics<R>, 1> m_globalMetric {};

    template <typename InputIterator>
    void m_updateMetrics(InputIterator begin, InputIterator end);

    void m_updateMetrics(R dataValue) noexcept;

    void m_updateMetrics(const Metrics<R> &dataMetric) noexcept;

  public:
    MetricsData() noexcept = default;

    explicit MetricsData(const Metrics<R> &metric) noexcept;

    explicit MetricsData(const std::vector<R> &data) noexcept;

    template <typename InputIterator>
    MetricsData(InputIterator begin, InputIterator end);

    void insert(R dataValue) noexcept;

    void insert(const Metrics<R> &metric) noexcept;

    void insert(const std::vector<R> &data);

    template <typename InputIterator>
    void insert(InputIterator begin, InputIterator end);

    const std::array<Metrics<R>, 1>::const_iterator begin() const noexcept;

    const std::array<Metrics<R>, 1>::const_iterator end() const noexcept;

    const std::array<Metrics<R>, 1>::const_iterator cbegin() const noexcept;

    const std::array<Metrics<R>, 1>::const_iterator cend() const noexcept;

    const std::array<Metrics<R>, 1> &data() const noexcept;

    const Metrics<R> &globalMetric() const noexcept;

    [[nodiscard]] size_t size() const noexcept;

    // Structural function, doesn't do anything
    inline void reserve(size_t) const noexcept;
};

template <Real R>
const std::array<Metrics<R>, 1> &MetricsData<R>::data() const noexcept
{
    return m_globalMetric;
}

template <Real R>
const std::array<Metrics<R>, 1>::const_iterator MetricsData<R>::begin() const noexcept
{
    return m_globalMetric.begin();
}

template <Real R>
const std::array<Metrics<R>, 1>::const_iterator MetricsData<R>::end() const noexcept
{
    return m_globalMetric.end();
}

template <Real R>
const std::array<Metrics<R>, 1>::const_iterator MetricsData<R>::cbegin() const noexcept
{
    return m_globalMetric.cbegin();
}

template <Real R>
const std::array<Metrics<R>, 1>::const_iterator MetricsData<R>::cend() const noexcept
{
    return m_globalMetric.cend();
}

template <Real R>
void MetricsData<R>::reserve(size_t) const noexcept
{
}

template <Real R>
void MetricsData<R>::insert(const std::vector<R> &data)
{
    insert(data.begin(), data.end());
}

template <Real R>
size_t MetricsData<R>::size() const noexcept
{
    return m_globalMetric[0].size;
}

template <Real R>
void MetricsData<R>::insert(const Metrics<R> &metric) noexcept
{
    m_updateMetrics(metric);
}

template <Real R>
void MetricsData<R>::insert(R dataValue) noexcept
{
    m_updateMetrics(dataValue);
}

template <Real R>
template <typename InputIterator>
void MetricsData<R>::insert(InputIterator begin, InputIterator end)
{
    m_updateMetrics(begin, end);
}

template <Real R>
const Metrics<R> &MetricsData<R>::globalMetric() const noexcept
{
    return m_globalMetric[0];
}

template <Real R>
template <typename InputIterator>
MetricsData<R>::MetricsData(InputIterator begin, InputIterator end)
    : MetricsData(Metrics<R>::compute(begin, end))
{
}

template <Real R>
MetricsData<R>::MetricsData(const Metrics<R> &metric) noexcept : m_globalMetric {{metric}}
{
}

template <Real R>
MetricsData<R>::MetricsData(const std::vector<R> &data) noexcept
    : MetricsData(data.begin(), data.end())
{
}

template <Real R>
void MetricsData<R>::m_updateMetrics(R dataValue) noexcept
{
    std::array<R, 1> tmp = {dataValue};
    m_updateMetrics(tmp.begin(), tmp.end());
}

template <Real R>
template <typename InputIterator>
void MetricsData<R>::m_updateMetrics(InputIterator begin, InputIterator end)
{
    // Because MetricsData has a container like interface, even though it only contains one single
    // value - the globalMetrics, the following confusing situation can arrive:
    // begin and end can represent 2 cases:
    // 1. a list of Real values from which we need to manually compute the metrics associated to
    // this set.
    // 2. A single Metrics, disguished by the fake container interface. In this case, we no longer
    // need to compute any associated metrics as we already have it
    using ValueType = std::remove_cvref_t<decltype(*begin)>;
    Metrics<R> newMetrics {};
    if constexpr (std::is_same<ValueType, R>::value)
        newMetrics = Metrics<R>::compute(begin, end);
    else if constexpr (std::is_same<ValueType, Metrics<R>>::value)
        newMetrics = *begin;
    else // ERROR, unknown type
        static_assert(false, "Got unexpected type");
    m_updateMetrics(newMetrics);
}

template <Real R>
void MetricsData<R>::m_updateMetrics(const Metrics<R> &dataMetric) noexcept
{
    m_globalMetric[0] = Metrics<R>::combineMetrics(m_globalMetric[0], dataMetric);
}

#endif // CRYPTOLYSER_ATTACKER_METRICSDATA_HPP
