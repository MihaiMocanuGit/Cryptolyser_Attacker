#ifndef CRYPTOLYSER_ATTACKER_CORRELATE_HPP
#define CRYPTOLYSER_ATTACKER_CORRELATE_HPP

#include "DataProcessing/Samples/SampleData.hpp"
#include "DataProcessing/Samples/SampleGroup.hpp"

#include <stdexcept>

namespace Correlate
{
template <typename Real_t>
Real_t computeFactorSampleData(const SampleData<Real_t> &sample1, const SampleData<Real_t> &sample2)
{
    Real_t result{sample1.metrics().mean - sample2.metrics().mean};
    return result * result;
}

template <typename Real_t>
Real_t computeFactorSampleGroup(const SampleGroup<Real_t> &sampleGroup1,
                                const SampleGroup<Real_t> &sampleGroup2)
{
    if (sampleGroup1.size() != sampleGroup2.size())
        throw std::logic_error("SampleGroups must have the same size");
    size_t maxDim = sampleGroup1.size();
    Real_t result{0};
    for (size_t dim{0}; dim < maxDim; ++dim)
        result += computeFactorSampleData(sampleGroup1[dim], sampleGroup2[dim]);
    return result;
}

template <typename Real_t>
Real_t computeFactorStdSampleGroup(const SampleGroup<Real_t> &sampleGroup1,
                                   const SampleGroup<Real_t> &sampleGroup2)
{
    if (sampleGroup1.size() != sampleGroup2.size())
        throw std::logic_error("SampleGroups must have the same size");
    size_t maxDim = sampleGroup1.size();
    Real_t result{0};
    for (size_t dim{0}; dim < maxDim; ++dim)
    {
        Real_t difference = sampleGroup1.standardizeLocalMetrics(dim).mean -
                            sampleGroup2.standardizeLocalMetrics(dim).mean;
        result += difference * difference;
    }
    return result;
}

template <typename Real_t>
Real_t computeFactorSampleBlock(const std::vector<SampleGroup<Real_t>> &sampleBlock1,
                                const std::vector<SampleGroup<Real_t>> &sampleBlock2)
{
    if (sampleBlock1.size() != sampleBlock2.size())
        throw std::logic_error("SampleGroups must have the same size");
    size_t maxDim = sampleBlock1.size();

    Real_t result{0.0};
    for (unsigned block = 0; block < maxDim; ++block)
    {
        result += computeFactorSampleGroup(sampleBlock1[block], sampleBlock2[block]);
    }
    return result;
}

template <typename Real_t>
Real_t computeFactorStdSampleBlock(const std::vector<SampleGroup<Real_t>> &sampleBlock1,
                                   const std::vector<SampleGroup<Real_t>> &sampleBlock2)
{
    if (sampleBlock1.size() != sampleBlock2.size())
        throw std::logic_error("SampleGroups must have the same size");
    size_t maxDim = sampleBlock1.size();

    Real_t result{0.0};
    for (unsigned block = 0; block < maxDim; ++block)
    {
        result += computeFactorStdSampleGroup(sampleBlock1[block], sampleBlock2[block]);
    }
    return result;
}
} // namespace Correlate
#endif // CRYPTOLYSER_ATTACKER_CORRELATE_HPP
