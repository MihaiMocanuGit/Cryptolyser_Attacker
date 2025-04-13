#ifndef CRYPTOLYSER_ATTACKER_DISTRIBUTIONDATA_HPP
#define CRYPTOLYSER_ATTACKER_DISTRIBUTIONDATA_HPP

#include "../SampleData/SampleData.hpp"

#include <algorithm>
#include <cstdio>

template <Real R>
class DistributionData
{
  private:
    R m_globalStart{}; // inclusive lower bound
    R m_globalStop{};  // inclusive upper bound
    R m_columnWidth{1};
    std::vector<size_t> m_frequency{};
    size_t m_noElements{};
    // Note that, as the globalMetric is recomputed each time from scratch, the elements inside this
    // vector can be modified from without needing as much rigor
  public:
    template <typename InputIterator>
    DistributionData(InputIterator begin, InputIterator end, R start, R stop, R columnWidth = 1);

    explicit DistributionData(const New::SampleData<R> &sampleData, R columnWidth = 1);

    size_t computeLocalIndex(ssize_t globalIndex) const;

    ssize_t computeGlobalIndex(size_t localIndex) const;

    [[nodiscard]] typename std::vector<size_t>::const_iterator begin() const noexcept;

    [[nodiscard]] typename std::vector<size_t>::const_iterator cbegin() const noexcept;

    [[nodiscard]] typename std::vector<size_t>::const_iterator end() const noexcept;

    [[nodiscard]] typename std::vector<size_t>::const_iterator cend() const noexcept;

    [[nodiscard]] Metrics<double> globalMetric() const noexcept;

    [[nodiscard]] const std::vector<size_t> &data() const noexcept;

    [[nodiscard]] size_t size() const noexcept;

    const size_t &operator[](size_t index) const;

    typename std::vector<size_t>::const_iterator insert(R dataValue);

    template <typename InputIterator>
    void insert(InputIterator begin, InputIterator end);

    void insert(const std::vector<R> &data);

    void update(size_t localIndex, size_t newFreq);

    [[nodiscard]] R globalStart() const noexcept;
    [[nodiscard]] R globalStop() const noexcept;
    [[nodiscard]] R columnWidth() const noexcept;
    struct Bounds
    {
        R lb;
        R ub;
    };
    [[nodiscard]] DistributionData<R>::Bounds bounds(double confidenceLB,
                                                     double confidenceUB) const noexcept;
};

template <Real R>
DistributionData<R>::Bounds DistributionData<R>::bounds(double confidenceLB,
                                                        double confidenceUB) const noexcept
{
    double lb{m_globalStart};
    double ub{m_globalStop};
    size_t partialCount{0};
    for (size_t localIndex{0}; localIndex < m_frequency.size(); localIndex++)
    {
        partialCount += m_frequency[localIndex];
        double ratio = static_cast<double>(partialCount) / static_cast<double>(m_noElements);
        if (ratio <= confidenceLB)
            lb = computeGlobalIndex(localIndex);
        ub = computeGlobalIndex(localIndex);
        if (1.0 - ratio <= confidenceUB)
            break;
    }
    return Bounds{.lb = lb, .ub = ub};
}

template <Real R>
void DistributionData<R>::update(size_t localIndex, size_t newFreq)
{
    assert(localIndex < size());
    m_noElements += newFreq - m_frequency[localIndex];
    m_frequency[localIndex] = newFreq;
}

template <Real R>
R DistributionData<R>::columnWidth() const noexcept
{
    return m_columnWidth;
}

template <Real R>
R DistributionData<R>::globalStop() const noexcept
{
    return m_globalStop;
}

template <Real R>
R DistributionData<R>::globalStart() const noexcept
{
    return m_globalStart;
}

template <Real R>
void DistributionData<R>::insert(const std::vector<R> &data)
{
    insert(data.begin(), data.end());
}

template <Real R>
template <typename InputIterator>
void DistributionData<R>::insert(InputIterator begin, InputIterator end)
{
    std::for_each(begin, end, [this](R elem) { insert(elem); });
}

template <Real R>
typename std::vector<size_t>::const_iterator DistributionData<R>::insert(R dataValue)
{
    return m_frequency.insert(begin() + computeLocalIndex(dataValue), dataValue);
}

template <Real R>
const size_t &DistributionData<R>::operator[](size_t index) const
{
    assert(index < size());
    return m_frequency[index];
}

template <Real R>
size_t DistributionData<R>::size() const noexcept
{
    return m_frequency.size();
}

template <Real R>
const std::vector<size_t> &DistributionData<R>::data() const noexcept
{
    return m_frequency;
}

template <Real R>
Metrics<double> DistributionData<R>::globalMetric() const noexcept
{
    return Metrics<double>::compute(m_frequency.begin(), m_frequency.end());
}

template <Real R>
typename std::vector<size_t>::const_iterator DistributionData<R>::cend() const noexcept
{
    return m_frequency.cend();
}

template <Real R>
typename std::vector<size_t>::const_iterator DistributionData<R>::end() const noexcept
{
    return m_frequency.end();
}

template <Real R>
typename std::vector<size_t>::const_iterator DistributionData<R>::cbegin() const noexcept
{
    return m_frequency.cbegin();
}

template <Real R>
typename std::vector<size_t>::const_iterator DistributionData<R>::begin() const noexcept
{
    return m_frequency.begin();
}

template <Real R>
DistributionData<R>::DistributionData(const New::SampleData<R> &sampleData, R columnWidth)
    : DistributionData{sampleData.begin(), sampleData.end(), sampleData.globalMetric().min,
                       sampleData.globalMetric().max, columnWidth}
{
}

template <Real R>
template <typename InputIterator>
DistributionData<R>::DistributionData(InputIterator begin, InputIterator end, R start, R stop,
                                      R columnWidth)
    : m_globalStart{start}, m_globalStop{stop}, m_columnWidth{columnWidth},
      // Note that computeLocalIndex uses m_globalStart/Stop and m_columnWidth
      m_frequency{std::vector<size_t>(computeLocalIndex(stop) - computeLocalIndex(start) + 1, 0)}
{
    assert(std::distance(begin, end) > 0);
    m_noElements = std::distance(begin, end);
    for (auto it{begin}; it != end; ++it)
        m_frequency[computeLocalIndex(*it)]++;
}

template <Real R>
size_t DistributionData<R>::computeLocalIndex(ssize_t globalIndex) const
{
    assert(m_globalStart <= globalIndex and globalIndex <= m_globalStop);
    assert(m_columnWidth > 0);
    size_t localIndex = (globalIndex - m_globalStart) / m_columnWidth;
    return localIndex;
}

template <Real R>
ssize_t DistributionData<R>::computeGlobalIndex(size_t localIndex) const
{
    assert(localIndex < size());

    return localIndex * m_columnWidth + m_globalStart;
}

#endif // CRYPTOLYSER_ATTACKER_DISTRIBUTIONDATA_HPP
