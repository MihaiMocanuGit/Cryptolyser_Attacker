#ifndef CRYPTOLYSER_ATTACKER_DISTRIBUTIONBYTEBLOCK_HPP
#define CRYPTOLYSER_ATTACKER_DISTRIBUTIONBYTEBLOCK_HPP

#include "../Samples/SampleGroup.hpp"
#include "DistributionByteValue.hpp"

#include <vector>

class DistributionByteBlock
{
    std::vector<DistributionByteValue> m_valueDistributions;

  public:
    template <typename Real_t>
    explicit DistributionByteBlock(const SampleGroup<Real_t> &sampleGroup);

    [[nodiscard]] const std::vector<DistributionByteValue> &distributions() const;
};

template <typename Real_t>
DistributionByteBlock::DistributionByteBlock(const SampleGroup<Real_t> &sampleGroup)
    : m_valueDistributions{sampleGroup.begin(), sampleGroup.end()}
{
}

#endif // CRYPTOLYSER_ATTACKER_DISTRIBUTIONBYTEBLOCK_HPP
