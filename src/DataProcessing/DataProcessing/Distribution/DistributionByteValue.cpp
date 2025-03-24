#include "DistributionByteValue.hpp"

size_t DistributionByteValue::start() const { return m_start; }

size_t DistributionByteValue::stop() const { return m_stop; }

const SampleData<size_t> &DistributionByteValue::frequency() const { return m_frequency; }

size_t DistributionByteValue::peakValue() const { return m_peakIndex + m_start; }
