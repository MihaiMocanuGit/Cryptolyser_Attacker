#ifndef CRYPTOLYSER_ATTACKER_GATHERER_HPP
#define CRYPTOLYSER_ATTACKER_GATHERER_HPP

#include "DataProcessing/Metrics/SampleData.hpp"
#include "ServerConnection/ServerConnection.hpp"
#include "Study/OldTimingData/TimingData.hpp"

#include <type_traits>

template <bool KnownKey>
class Gatherer
{
  private:
    ServerConnection<KnownKey> m_connection{};

    Old::TimingData<KnownKey> m_timingData{};

    double m_lb{};
    Old::SampleData<double> m_sampleLB{};
    double m_ub{};
    Old::SampleData<double> m_sampleUB{};
    size_t m_lostPackages{};

    struct BorrowedData
    {
        ServerConnection<KnownKey> &&connection;
        Old::TimingData<KnownKey> &&timingData;
    };

    std::vector<std::byte> m_constructRandomVector();

  public:
    Gatherer(ServerConnection<KnownKey> &&connection, Old::TimingData<KnownKey> &&timingData);

    Gatherer(Gatherer &&gatherer) noexcept = default;
    Gatherer &operator=(Gatherer &&rhs) noexcept = default;
    ~Gatherer() = default;

    Gatherer(const Gatherer &other) = delete;
    Gatherer &operator=(const Gatherer &other) = delete;

    void init(double lb = 0, double ub = std::numeric_limits<double>::max());

    enum class ObtainStatus
    {
        success,
        lost,
        ignoredLB,
        ignoredUB,
    };
    ObtainStatus obtain(uint32_t id);

    double lb() const;
    const Old::SampleData<double> &sampleLB() const;

    double ub() const;
    const Old::SampleData<double> &sampleUB() const;

    size_t lostPackages() const;
    size_t validValuesCount() const;

    const Old::TimingData<KnownKey> &timingData() const;

    ServerConnection<KnownKey> &connection();

    BorrowedData release();
};

#endif // CRYPTOLYSER_ATTACKER_GATHERER_HPP
