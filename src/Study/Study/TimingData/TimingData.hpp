#ifndef CRYPTOLYSER_ATTACKER_TIMINGDATA_HPP
#define CRYPTOLYSER_ATTACKER_TIMINGDATA_HPP

#include "Cryptolyser_Common/connection_data_types.h"
#include "DataProcessing/DataVector/DataVector.hpp"
#include "DataProcessing/SampleData/SampleData.hpp"

#include <array>
#include <cassert>

template <bool KnownKey, HasMetric DataType = SampleData<double>>
class TimingData
{
  private:
    struct empty_t
    {
    };

    size_t m_dataSize;
    [[no_unique_address]] std::conditional_t<KnownKey, std::array<std::byte, PACKET_KEY_SIZE>,
                                             empty_t> m_key;

    DataVector<DataVector<DataType>> m_timings {};

  public:
    /**
     * @brief Constructor active only when KnownKey == true.
     * @param dataSize The size of the studied data.
     * @param key The key used when encrypting the studied data.
     */
    TimingData(size_t dataSize, const std::array<std::byte, PACKET_KEY_SIZE> &key)
        requires(KnownKey);

    /**
     * @brief Constructor active only when KnownKey == false.
     * @param dataSize The size of the studied data.
     */
    explicit TimingData(size_t dataSize)
        requires(not KnownKey);

    TimingData(TimingData &&timingData) noexcept = default;
    TimingData &operator=(TimingData &&rhs) noexcept = default;
    ~TimingData() = default;

    TimingData(const TimingData &other) = delete;
    TimingData &operator=(const TimingData &other) = delete;

    const DataVector<DataType> &operator[](size_t index) const noexcept;

    const DataVector<DataVector<DataType>> &timing() const noexcept;
    DataVector<DataVector<DataType>> &timing();

    void insertTiming(const DataVector<DataVector<DataType>> &data);

    void reserveForEach(size_t reserveSize);

    [[nodiscard]] size_t dataSize() const noexcept;

    [[nodiscard]] const std::array<std::byte, PACKET_KEY_SIZE> &key() const noexcept
        requires(KnownKey);
};

template <bool KnownKey, HasMetric DataType>
void TimingData<KnownKey, DataType>::insertTiming(const DataVector<DataVector<DataType>> &data)
{
    //    assert(data.size() == m_timings.size());
    //    for (const auto &elem : data)
    //        assert(elem.size() == m_timings.size());

    joinDataVectors(m_timings, data);
}

template <bool KnownKey, HasMetric DataType>
DataVector<DataVector<DataType>> &TimingData<KnownKey, DataType>::timing()
{
    return m_timings;
}

template <bool KnownKey, HasMetric DataType>
const DataVector<DataVector<DataType>> &TimingData<KnownKey, DataType>::timing() const noexcept
{
    return m_timings;
}

template <bool KnownKey, HasMetric DataType>
const DataVector<DataType> &TimingData<KnownKey, DataType>::operator[](size_t index) const noexcept
{
    return m_timings[index];
}

template <bool KnownKey, HasMetric DataType>
void TimingData<KnownKey, DataType>::reserveForEach(size_t reserveSize)
{
    m_timings.update_foreach(
        [reserveSize](size_t index, auto &byteValues)
        {
            (void)index;
            byteValues.update_foreach(
                [reserveSize](size_t index, auto &sampleData)
                {
                    (void)index;
                    sampleData.reserve(reserveSize);
                });
        });
}

template <bool KnownKey, HasMetric DataType>
const std::array<std::byte, PACKET_KEY_SIZE> &
    TimingData<KnownKey, DataType>::key() const noexcept
    requires(KnownKey)
{
    return m_key;
}

template <bool KnownKey, HasMetric DataType>
TimingData<KnownKey, DataType>::TimingData(size_t dataSize)
    requires(not KnownKey)
    : m_dataSize {dataSize}, m_timings {PACKET_AES_BLOCK_SIZE, DataVector<DataType>(256)}
{
}

template <bool KnownKey, HasMetric DataType>
TimingData<KnownKey, DataType>::TimingData(size_t dataSize,
                                           const std::array<std::byte, PACKET_KEY_SIZE> &key)
    requires(KnownKey)
    : m_dataSize {dataSize}, m_key {key}, m_timings {PACKET_AES_BLOCK_SIZE, DataVector<DataType>(256)}
{
}

template <bool KnownKey, HasMetric DataType>
size_t TimingData<KnownKey, DataType>::dataSize() const noexcept
{
    return m_dataSize;
}
#endif // CRYPTOLYSER_ATTACKER_TIMINGDATA_HPP
