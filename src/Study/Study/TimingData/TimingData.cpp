#include "TimingData.hpp"

template <bool KnownKey>
size_t TimingData<KnownKey>::dataSize() const
{
    return m_dataSize;
}

template class TimingData<true>;
template class TimingData<false>;
