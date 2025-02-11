#ifndef CRYPTOLYSER_ATTACKER_SAMPLEDATA_HPP
#define CRYPTOLYSER_ATTACKER_SAMPLEDATA_HPP

#include <cmath>
#include <numeric>
#include <vector>

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
    template <typename InputIterator>
    SampleMetrics(InputIterator begin, InputIterator end)
    {
        size_t aSize =
            std::distance(begin, end); // I do not like that std::distance can be negative
        if (aSize == 0)
        {
            *this = {};
            return;
        }
        else if (aSize == 1)
        {
            *this = {*begin, 1, *begin, 0, 0};
            return;
        }

        Real_t aSum = std::accumulate(begin, end, 0); // overflow?
        Real_t aMean = aSum / aSize;
        Real_t squareSum = 0;
        for (auto it = begin; it != end; ++it)
        {
            squareSum += (*it - aMean) * (*it - aMean);
        }
        Real_t aVariance = squareSum / (aSize - 1);
        Real_t sStdDev = std::sqrt(aVariance);
        *this = {aSum, aSize, aMean, aVariance, sStdDev};
    }
    static SampleMetrics<Real_t> combineMetrics(const SampleMetrics &metric1,
                                                const SampleMetrics &metric2) noexcept;

};

template <typename Real_t>
class SampleData
{
  private:
    std::vector<Real_t> m_data{};

    SampleMetrics<Real_t> m_currentMetrics{};

    template <typename InputIterator>
    void m_updateMetrics(InputIterator begin, InputIterator end)
    {
        SampleMetrics<Real_t> newMetrics = SampleMetrics<Real_t>{begin, end};
        m_currentMetrics = SampleMetrics<Real_t>::combineMetrics(m_currentMetrics, newMetrics);
    }

    void m_updateMetrics(const Real_t &dataValue);

  public:
    SampleData() = default;
    explicit SampleData(const std::vector<Real_t> &data);
    explicit SampleData(std::vector<Real_t> &&data);

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

    [[nodiscard]] const SampleMetrics<Real_t> &metrics() const noexcept;
};
#endif // CRYPTOLYSER_ATTACKER_SAMPLEDATA_HPP
