#ifndef CRYPTOLYSER_SAMPLEGROUP_HPP
#define CRYPTOLYSER_SAMPLEGROUP_HPP
#include "SampleData.hpp"

namespace Old
{
template <typename Real_t>
class SampleGroup
{
  private:
    std::vector<SampleData<Real_t>> m_samples{};
    Metrics<Real_t> m_globalMetrics;
    void m_updateGlobalMetrics(Metrics<Real_t> oldLocalMetrics, Metrics<Real_t> newLocalMetrics);

  public:
    SampleGroup() = default;
    explicit SampleGroup(size_t size);
    SampleGroup(size_t size, size_t reserveSize);

    void reserveForAll(size_t size);

    void insert(size_t index, Real_t data);

    template <typename InputIterator>
    void insert(size_t index, InputIterator begin, InputIterator end)
    {
        Metrics<Real_t> oldMetrics = m_samples[index].metrics();
        m_samples[index].insert(begin, end);
        Metrics<Real_t> newMetrics = m_samples[index].metrics();
        m_updateGlobalMetrics(oldMetrics, newMetrics);
    }

    const Metrics<Real_t> &globalMetric() const noexcept;
    const Metrics<Real_t> &localMetrics(size_t index) const;
    [[nodiscard]] Metrics<Real_t> standardizeLocalMetrics(size_t index) const;

    const SampleData<Real_t> &sampleAt(size_t index) const;
    const SampleData<Real_t> &operator[](size_t index) const;

    size_t size() const noexcept;
    void resize(size_t size);
    typename std::vector<SampleData<Real_t>>::const_iterator begin() const noexcept;
    typename std::vector<Old::SampleData<Real_t>>::const_iterator end() const noexcept;
};
} // namespace Old

#endif // CRYPTOLYSER_SAMPLEGROUP_HPP
