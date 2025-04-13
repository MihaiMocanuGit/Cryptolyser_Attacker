#ifndef CRYPTOLYSER_ATTACKER_METRICSDATA_HPP
#define CRYPTOLYSER_ATTACKER_METRICSDATA_HPP

#include "../Metrics/Metrics.hpp"

#include <concepts>
#include <vector>

template <Real T>
class MetricsData
{
  private:
    Metrics<T> m_globalMetrics{};

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

    const Metrics<T> &globalMetric() const noexcept;

    [[nodiscard]] size_t size() const noexcept;

    // Structural function, doesn't do anything
    inline void reserve(size_t) const noexcept;
};

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
    return m_globalMetrics.size;
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
    return m_globalMetrics;
}

template <Real T>
template <typename InputIterator>
MetricsData<T>::MetricsData(InputIterator begin, InputIterator end)
    : MetricsData(Metrics<T>::compute(begin, end))
{
}

template <Real T>
MetricsData<T>::MetricsData(const Metrics<T> &metric) noexcept : m_globalMetrics{metric}
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
    Metrics<T> newMetrics = Metrics<T>::compute(begin, end);
    m_updateMetrics(newMetrics);
}

template <Real T>
void MetricsData<T>::m_updateMetrics(const Metrics<T> &dataMetric) noexcept
{
    m_globalMetrics = Metrics<T>::combineMetrics(m_globalMetrics, dataMetric);
}

#endif // CRYPTOLYSER_ATTACKER_METRICSDATA_HPP
