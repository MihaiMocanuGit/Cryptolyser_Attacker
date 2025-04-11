#include "TimingData.hpp"

template <bool KnownKey>
size_t Old::TimingData<KnownKey>::dataSize() const
{
    return m_dataSize;
}

template class Old::TimingData<true>;
template class Old::TimingData<false>;
