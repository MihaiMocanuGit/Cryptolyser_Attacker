#ifndef CRYPTOLYSER_ATTACKER_SAMPLEDATA_HPP
#define CRYPTOLYSER_ATTACKER_SAMPLEDATA_HPP

#include "Metrics.hpp"

#include <vector>
namespace Old
{
template <typename Real_t>
class SampleData
{
  private:
    std::vector<Real_t> m_data{};

    Metrics<Real_t> m_currentMetrics{};

    template <typename InputIterator>
    void m_updateMetrics(InputIterator begin, InputIterator end)
    {
        Metrics<Real_t> newMetrics = Metrics<Real_t>::compute(begin, end);
        m_currentMetrics = Metrics<Real_t>::combineMetrics(m_currentMetrics, newMetrics);
    }

    void m_updateMetrics(const Real_t &dataValue);

  public:
    SampleData() = default;
    explicit SampleData(const std::vector<Real_t> &data);
    explicit SampleData(std::vector<Real_t> &&data);

    template <typename InputIterator>
    SampleData(InputIterator begin, InputIterator end)
        : m_data{begin, end},
          m_currentMetrics{Metrics<Real_t>::compute(m_data.begin(), m_data.end())}
    {
    }

    void reserve(std::size_t newCapacity);

    typename std::vector<Real_t>::const_iterator insert(const Real_t &dataValue);

    template <typename InputIterator>
    typename std::vector<Real_t>::const_iterator insert(InputIterator begin, InputIterator end)
    {
        m_updateMetrics(begin, end);
        return m_data.insert(m_data.end(), begin, end);
    }

    typename std::vector<Real_t>::const_iterator begin() const noexcept;

    typename std::vector<Real_t>::const_iterator end() const noexcept;

    [[nodiscard]] const Metrics<Real_t> &metrics() const noexcept;

    const std::vector<Real_t> &data() const noexcept;

    size_t size() const noexcept;

    const Real_t &operator[](size_t index) const;
};
} // namespace Old
#endif // CRYPTOLYSER_ATTACKER_SAMPLEDATA_HPP
