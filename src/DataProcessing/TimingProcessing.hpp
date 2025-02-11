#ifndef CRYPTOLYSER_ATTACKER_TIMINGPROCESSING_HPP
#define CRYPTOLYSER_ATTACKER_TIMINGPROCESSING_HPP

#include <cstdint>
namespace TimingProcessing
{
template <typename Real_t>
Real_t computeDT(uint64_t inbound_sec, uint64_t inbound_nsec, uint64_t outbound_sec,
                 uint64_t outbound_nsec)
{
    uint64_t dSec = outbound_sec - inbound_sec;
    uint64_t dNsec = outbound_nsec - inbound_nsec;
    uint64_t uintDt = 1'000'000'000 * dSec + dNsec;
    auto dt = static_cast<Real_t>(uintDt);
    return dt;
}

} // namespace TimingProcessing
#endif // CRYPTOLYSER_ATTACKER_TIMINGPROCESSING_HPP
