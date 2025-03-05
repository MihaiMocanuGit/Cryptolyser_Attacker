#ifndef CRYPTOLYSER_ATTACKER_TIMINGPROCESSING_HPP
#define CRYPTOLYSER_ATTACKER_TIMINGPROCESSING_HPP

#include <cstdint>
namespace TimingProcessing
{
template <typename Real_t>
Real_t computeDT(uint64_t inbound_t1, uint64_t inbound_t2, uint64_t outbound_t1,
                 uint64_t outbound_t2)
{
    const uint64_t dT1 = outbound_t1 - inbound_t1;
    const uint64_t dT2 = outbound_t2 - inbound_t2;
    uint64_t uintDt = 1'000'000'000 * dT1 + dT2;
    return static_cast<Real_t>(uintDt);
}

} // namespace TimingProcessing
#endif // CRYPTOLYSER_ATTACKER_TIMINGPROCESSING_HPP
