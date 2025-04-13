#include "DistributionByteBlock.hpp"

const std::vector<DistributionByteValue> &DistributionByteBlock::distributions() const
{
    return m_valueDistributions;
}
