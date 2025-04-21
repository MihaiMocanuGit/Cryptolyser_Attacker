#ifndef CRYPTOLYSER_ATTACKER_METRICSDATA_HPP
#define CRYPTOLYSER_ATTACKER_METRICSDATA_HPP

#include "../Metrics/Metrics.hpp"

#include <array>
#include <concepts>
#include <vector>

template <Real T>
class MetricsData
{
  private:
    // Storing the global metric into an array in order to mimick the container interface of other
    // types of Data containers (SampleData, DistributionData).
    std::array<Metrics<T>, 1> m_globalMetric {};

    template <typename InputIterator>
    void m_updateMetrics(InputIterator begin, InputIterator end);

    void m_updateMetrics(T dataValue) noexcept;

    void m_updateMetrics(const Metrics<T> &dataMetric) noexcept;

  public:
    MetricsData() noexcept = default;

    explicit MetricsData(const Metrics<T> &metric) noexcept;

    explicit MetricsData(const std::vector<T> &data) noexcept;

    template <typename InputIterator>
    MetricsData(InputIterator begin, InputIterator end);

    void insert(T dataValue) noexcept;

    void insert(const Metrics<T> &metric) noexcept;

    void insert(const std::vector<T> &data);

    template <typename InputIterator>
    void insert(InputIterator begin, InputIterator end);

    const std::array<Metrics<T>, 1>::const_iterator begin() const noexcept;

    const std::array<Metrics<T>, 1>::const_iterator end() const noexcept;

    const std::array<Metrics<T>, 1>::const_iterator cbegin() const noexcept;

    const std::array<Metrics<T>, 1>::const_iterator cend() const noexcept;

    const std::array<Metrics<T>, 1> &data() const noexcept;

    const Metrics<T> &globalMetric() const noexcept;

    [[nodiscard]] size_t size() const noexcept;

    // Structural function, doesn't do anything
    inline void reserve(size_t) const noexcept;
};

template <Real T>
const std::array<Metrics<T>, 1> &MetricsData<T>::data() const noexcept
{
    return m_globalMetric;
}

template <Real T>
const std::array<Metrics<T>, 1>::const_iterator MetricsData<T>::begin() const noexcept
{
    return m_globalMetric.begin();
}

template <Real T>
const std::array<Metrics<T>, 1>::const_iterator MetricsData<T>::end() const noexcept
{
    return m_globalMetric.end();
}

template <Real T>
const std::array<Metrics<T>, 1>::const_iterator MetricsData<T>::cbegin() const noexcept
{
    return m_globalMetric.cbegin();
}

template <Real T>
const std::array<Metrics<T>, 1>::const_iterator MetricsData<T>::cend() const noexcept
{
    return m_globalMetric.cend();
}

template <Real T>
void MetricsData<T>::reserve(size_t) const noexcept
{
}

template <Real T>
void MetricsData<T>::insert(const std::vector<T> &data)
{
    insert(data.begin(), data.end());
}

template <Real T>
size_t MetricsData<T>::size() const noexcept
{
    return m_globalMetric[0].size;
}

template <Real T>
void MetricsData<T>::insert(const Metrics<T> &metric) noexcept
{
    m_updateMetrics(metric);
}

template <Real T>
void MetricsData<T>::insert(T dataValue) noexcept
{
    m_updateMetrics(dataValue);
}

template <Real T>
template <typename InputIterator>
void MetricsData<T>::insert(InputIterator begin, InputIterator end)
{
    m_updateMetrics(begin, end);
}

template <Real T>
const Metrics<T> &MetricsData<T>::globalMetric() const noexcept
{
    return m_globalMetric[0];
}

template <Real T>
template <typename InputIterator>
MetricsData<T>::MetricsData(InputIterator begin, InputIterator end)
    : MetricsData(Metrics<T>::compute(begin, end))
{
}

template <Real T>
MetricsData<T>::MetricsData(const Metrics<T> &metric) noexcept : m_globalMetric {{metric}}
{
}

template <Real T>
MetricsData<T>::MetricsData(const std::vector<T> &data) noexcept
    : MetricsData(data.begin(), data.end())
{
}

template <Real T>
void MetricsData<T>::m_updateMetrics(T dataValue) noexcept
{
    std::array<T, 1> tmp = {dataValue};
    m_updateMetrics(tmp.begin(), tmp.end());
}

template <Real T>
template <typename InputIterator>
void MetricsData<T>::m_updateMetrics(InputIterator begin, InputIterator end)
{
    // Because MetricsData has a container like interface, even though it only contains one single
    // value - the globalMetrics, the following confusing situation can arrive:
    // begin and end can represent 2 cases:
    // 1. a list of Real values from which we need to manually compute the metrics associated to
    // this set.
    // 2. A single Metrics, disguished by the fake container interface. In this case, we no longer
    // need to compute any associated metrics as we already have it
    using ValueType = std::remove_cvref_t<decltype(*begin)>;
    Metrics<T> newMetrics {};
    if constexpr (std::is_same<ValueType, T>::value)
        newMetrics = Metrics<T>::compute(begin, end);
    else if constexpr (std::is_same<ValueType, Metrics<T>>::value)
        newMetrics = *begin;
    else // ERROR, unknown type
        static_assert(false, "Got unexpected type");
    m_updateMetrics(newMetrics);
}

template <Real T>
void MetricsData<T>::m_updateMetrics(const Metrics<T> &dataMetric) noexcept
{
    m_globalMetric[0] = Metrics<T>::combineMetrics(m_globalMetric[0], dataMetric);
}

#endif // CRYPTOLYSER_ATTACKER_METRICSDATA_HPP
