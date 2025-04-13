#ifndef CRYPTOLYSER_ATTACKER_OLD_TIMINGDATA_HPP
#define CRYPTOLYSER_ATTACKER_OLD_TIMINGDATA_HPP

#include "Cryptolyser_Common/connection_data_types.h"
#include "DataProcessing/Metrics/SampleGroup.hpp"

#include <array>
namespace Old
{
template <bool KnownKey>
class TimingData
{
  private:
    struct empty_t
    {
    };

    size_t m_dataSize;
    [[no_unique_address]] std::conditional_t<KnownKey, std::array<std::byte, PACKET_KEY_BYTE_SIZE>,
                                             empty_t> m_key;

  public:
    std::vector<Old::SampleGroup<double>> blockTimings{};

    /**
     * @brief Constructor active only when KnownKey == true.
     * @param dataSize The size of the studied data.
     * @param key The key used when encrypting the studied data.
     */
    template <bool T = KnownKey>
    TimingData(size_t dataSize, size_t reserveSize,
               typename std::enable_if<T, std::array<std::byte, PACKET_KEY_BYTE_SIZE>>::type key);

    /**
     * @brief Constructor active only when KnownKey == false.
     * @param dataSize The size of the studied data.
     */
    template <bool T = not KnownKey>
    explicit TimingData(size_t dataSize, size_t reserveSize,
                        typename std::enable_if<T, empty_t>::type = {});

    TimingData(TimingData &&timingData) noexcept = default;
    TimingData &operator=(TimingData &&rhs) noexcept = default;
    ~TimingData() = default;

    TimingData(const TimingData &other) = delete;
    TimingData &operator=(const TimingData &other) = delete;

    [[nodiscard]] size_t dataSize() const;

    template <bool T = KnownKey>
    [[nodiscard]]
    typename std::enable_if<T, const std::array<std::byte, PACKET_KEY_BYTE_SIZE> &>::type
        key() const;
};

template <bool KnownKey>
template <bool T>
std::enable_if<T, const std::array<std::byte, PACKET_KEY_BYTE_SIZE> &>::type
    TimingData<KnownKey>::key() const
{
    return m_key;
}

template <bool KnownKey>
template <bool T>
TimingData<KnownKey>::TimingData(size_t dataSize, size_t reserveSize,
                                 std::enable_if<T, TimingData::empty_t>::type)
    : m_dataSize{dataSize},
      blockTimings{AES_BLOCK_BYTE_SIZE, Old::SampleGroup<double>(256, reserveSize)}
{
}

template <bool KnownKey>
template <bool T>
TimingData<KnownKey>::TimingData(
    size_t dataSize, size_t reserveSize,
    std::enable_if<T, std::array<std::byte, PACKET_KEY_BYTE_SIZE>>::type key)
    : m_dataSize{dataSize}, m_key{key},
      blockTimings{AES_BLOCK_BYTE_SIZE, Old::SampleGroup<double>(256, reserveSize)}
{
}
} // namespace Old
#endif // CRYPTOLYSER_ATTACKER_OLD_TIMINGDATA_HPP
