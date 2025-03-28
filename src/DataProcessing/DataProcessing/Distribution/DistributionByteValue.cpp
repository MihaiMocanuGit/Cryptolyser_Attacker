#include "DistributionByteValue.hpp"

size_t DistributionByteValue::start() const { return m_start; }

size_t DistributionByteValue::stop() const { return m_stop; }

const SampleData<size_t> &DistributionByteValue::frequency() const { return m_frequency; }

size_t DistributionByteValue::peakValue() const { return m_peakIndex + m_start; }

DistributionByteValue::Bounds DistributionByteValue::bounds(double confidenceLB,
                                                            double confidenceUB)
{
    size_t lb{m_start};
    size_t ub{m_stop};
    size_t partialSum{0};
    for (size_t pos{0}; pos < m_frequency.size(); pos++)
    {
        partialSum += m_frequency[pos];
        double ratio = static_cast<double>(partialSum) / static_cast<double>(m_sum);
        if (ratio <= confidenceLB)
            lb = pos + m_start;
        ub = pos + m_start;
        if (1.0 - ratio <= confidenceUB)
            break;
    }

    return {.lb = lb, .ub = ub};
}
