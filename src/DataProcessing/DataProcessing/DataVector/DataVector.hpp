#ifndef CRYPTOLYSER_ATTACKER_DATAVECTOR_HPP
#define CRYPTOLYSER_ATTACKER_DATAVECTOR_HPP

#include "../Metrics/Metrics.hpp"

#include <algorithm>
#include <cassert>
#include <execution>
#include <vector>

template <HasMetric T>
class DataVector
{
  private:
    std::vector<T> m_data {};

    Metrics<double> m_globalMetric {};

    [[nodiscard]] Metrics<double> m_computeAfterAdd(const Metrics<double> &globalBefore,
                                                    const Metrics<double> &localAfter);

    [[nodiscard]] Metrics<double> m_computeAfterRemove(const Metrics<double> &globalBefore,
                                                       const Metrics<double> &localBefore);

    [[nodiscard]] Metrics<double> m_computeAfterUpdate(const Metrics<double> &globalBefore,
                                                       const Metrics<double> &localBefore,
                                                       const Metrics<double> &localAfter);

  public:
    using InnerType = T;

    DataVector() = default;

    explicit DataVector(std::vector<T> data);

    explicit DataVector(size_t count, const T &value = {});

    template <class Iterator>
    DataVector(Iterator begin, Iterator end);

    typename std::vector<T>::const_iterator cbegin() const noexcept;

    typename std::vector<T>::const_iterator begin() const noexcept;

    typename std::vector<T>::const_iterator cend() const noexcept;

    typename std::vector<T>::const_iterator end() const noexcept;

    [[nodiscard]] const Metrics<double> &globalMetric() const noexcept;

    [[nodiscard]] Metrics<double> standardizeMetric(size_t index) const;

    const std::vector<T> &data() const noexcept;

    const T &operator[](size_t index) const;

    [[nodiscard]] size_t size() const noexcept;

    void add(T element);

    void remove(size_t index);

    void update(size_t index, const std::invocable<size_t, T &> auto &modifyRule);

    void update_foreach(const std::invocable<size_t, T &> auto &modifyRule);
};

template <HasMetric T>
Metrics<double> DataVector<T>::standardizeMetric(size_t index) const
{
    Metrics<double> standardized {m_data[index].globalMetric()};
    standardized.mean = (standardized.mean - m_globalMetric.mean) /
                        (m_globalMetric.stdDev / std::sqrtl(m_globalMetric.size));
    standardized.variance /= m_globalMetric.variance;
    standardized.stdDev = std::sqrt(standardized.variance);
    return standardized;
}

template <typename T>
struct IsDataVectorChain : std::false_type
{
};

template <typename T>
struct IsDataVectorChain<DataVector<T>> : std::true_type
{
};

template <typename T>
concept DataVectorChain = IsDataVectorChain<T>::value;

template <HasMetric T>
void joinDataVectors(T &destination, const T &source)
{
    if constexpr (IsDataVectorChain<T>::value)
    {
        /// TODO: Define concepts to enforce the existence of .size(), .update_foreach(), .insert()
        while (destination.size() < source.size())
            destination.add({});
        assert(destination.size() == source.size());
        // We want to go join every sub-element from destination with the same element from source
        destination.update_foreach([&source](size_t index, auto &elem)
                                   { joinDataVectors(elem, source[index]); });
    }
    else
    {
        // T is MetricsData/SampleData<double>, so in this case, we just want to append the data at
        // the end of sample.
        destination.insert(source.data());
    }
}

template <HasMetric T>
typename std::vector<T>::const_iterator DataVector<T>::end() const noexcept
{
    return m_data.end();
}

template <HasMetric T>
typename std::vector<T>::const_iterator DataVector<T>::cend() const noexcept
{
    return m_data.cend();
}

template <HasMetric T>
typename std::vector<T>::const_iterator DataVector<T>::begin() const noexcept
{
    return m_data.begin();
}

template <HasMetric T>
typename std::vector<T>::const_iterator DataVector<T>::cbegin() const noexcept
{
    return m_data.cbegin();
}

template <HasMetric T>
size_t DataVector<T>::size() const noexcept
{
    return m_data.size();
}

template <HasMetric T>
template <class Iterator>
DataVector<T>::DataVector(Iterator begin, Iterator end)
{
    std::for_each(begin, end, [this](const auto &elem) { add(elem); });
}

template <HasMetric T>
const std::vector<T> &DataVector<T>::data() const noexcept
{
    return m_data;
}

template <HasMetric T>
DataVector<T>::DataVector(size_t count, const T &value) : DataVector(std::vector<T>(count, value))
{
}

template <HasMetric T>
DataVector<T>::DataVector(std::vector<T> data) : m_data {std::move(data)}
{
    std::for_each(m_data.begin(), m_data.end(),
                  [this](const auto &elem)
                  {
                      const Metrics<double> &globalBefore {globalMetric()};
                      const Metrics<double> &localAfter {elem.globalMetric()};
                      m_globalMetric = m_computeAfterAdd(globalBefore, localAfter);
                  });
}

template <HasMetric T>
const T &DataVector<T>::operator[](size_t index) const
{
    assert(index < m_data.size());
    return m_data[index];
}

template <HasMetric T>
void DataVector<T>::update(size_t index, const std::invocable<size_t, T &> auto &modifyRule)
{
    assert(index < m_data.size());
    Metrics<double> globalBefore {globalMetric()};
    Metrics<double> localBefore {m_data[index].globalMetric()};

    T &chosen {m_data[index]};
    modifyRule(index, chosen);

    Metrics<double> localAfter {chosen.globalMetric()};
    m_globalMetric = m_computeAfterUpdate(globalBefore, localBefore, localAfter);
}

template <HasMetric T>
void DataVector<T>::update_foreach(const std::invocable<size_t, T &> auto &modifyRule)
{
    // as the index is frequently a useful piece of information, a simple for loop is used.
    // TODO: Modify the signature of update(index, (void)(T&)) to accept the used modifyRule. This
    //  way there's no need for code duplication
    for (size_t i {0}; i < size(); ++i)
    {
        Metrics<double> globalBefore {globalMetric()};
        Metrics<double> localBefore {m_data[i].globalMetric()};

        T &chosen {m_data[i]};
        modifyRule(i, chosen);

        Metrics<double> localAfter {chosen.globalMetric()};
        m_globalMetric = m_computeAfterUpdate(globalBefore, localBefore, localAfter);
    }
}

template <HasMetric T>
void DataVector<T>::remove(size_t index)
{
    assert(index < m_data.size());
    Metrics<double> globalBefore {globalMetric()};
    Metrics<double> localBefore {m_data[index].globalMetric()};

    m_data.erase(m_data.begin() + index);
    m_globalMetric = m_computeAfterRemove(globalBefore, localBefore);
}

template <HasMetric T>
void DataVector<T>::add(T element)
{
    Metrics<double> globalBefore {globalMetric()};
    Metrics<double> localAfter {element.globalMetric()};

    m_data.push_back(std::move(element));
    m_globalMetric = m_computeAfterAdd(globalBefore, localAfter);
}

template <HasMetric T>
Metrics<double> DataVector<T>::m_computeAfterUpdate(const Metrics<double> &globalBefore,
                                                    const Metrics<double> &localBefore,
                                                    const Metrics<double> &localAfter)
{
    // Original global metrics
    size_t n {globalBefore.size};
    double sum {globalBefore.sum};
    double u {globalBefore.mean};
    double v {globalBefore.variance};
    double min {globalBefore.min};
    double max {globalBefore.max};

    // Old local metrics.
    size_t pi_m {localBefore.size};
    double sum_m {localBefore.sum};
    double u_m {localBefore.mean};
    double v_m {localBefore.variance};
    // As the logic becomes quite ugly, localBefore.min/max are used instead as it's easier to
    // understand.
    [[maybe_unused]] double min_m {localBefore.min};
    [[maybe_unused]] double max_m {localBefore.max};

    // New local metrics.
    size_t pi_prime_m {localAfter.size};
    double sum_prime_m {localAfter.sum};
    double u_prime_m {localAfter.mean};
    double v_prime_m {localAfter.variance};
    // As the logic becomes quite ugly, localAfter.min/max are used instead as it's easier to
    // understand.
    [[maybe_unused]] double min_prime_m {localAfter.min};
    [[maybe_unused]] double max_prime_m {localAfter.max};

    // TODO: Note the temporary downgrade from Real_t T to double. At the end of the current
    //  architectural restructure, the code will go back to the template type T.
    // Note: Famous last words

    // Updated global metrics.
    size_t n_prime {n - pi_m + pi_prime_m};
    // Helper conversions to double.
    double n_prime_d {static_cast<double>(n_prime)};
    double n_d {static_cast<double>(n)};
    double pi_prime_m_d {static_cast<double>(pi_prime_m)};
    double pi_m_d {static_cast<double>(pi_m)};
    // Back to updated global metrics
    double sum_prime {sum - sum_m + sum_prime_m};
    double u_prime {0.0};
    if (n_prime)
    {
        u_prime = (n_d * u + pi_prime_m_d * u_prime_m - pi_m_d * u_m) / n_prime_d;
    }
    double SS_m {v_m * (pi_m_d - 1) + pi_m_d * (u_m - u) * (u_m - u)};
    double SS_prime_m {v_prime_m * (pi_prime_m_d - 1) +
                       pi_prime_m_d * (u_prime_m - u_prime) * (u_prime_m - u_prime)};
    double v_prime {0.0};
    if (n_prime > 1)
    {
        v_prime = ((n_d - 1) * v + SS_prime_m - SS_m +
                   (u - u_prime) * (2 * n_d * u - pi_m_d * u_m - (u + u_prime) * (n_d - pi_m_d))) /
                  (n_prime_d - 1);
    }
    double s_prime {static_cast<double>(std::sqrtl(v_prime))};
    double min_prime {min};
    assert(globalBefore.min <= localBefore.min);
    if (localAfter.min < globalBefore.min)
        min_prime = localAfter.min;
    else if (globalBefore.min == localBefore.min and localBefore.min != localAfter.min)
    {
        if (localBefore.min > localAfter.min)
            min_prime = localAfter.min;
        else // iterating through the whole vector only as a last resort
            min_prime =
                std::min_element(std::execution::par_unseq, m_data.begin(), m_data.end(),
                                 [](const T &el1, const T &el2)
                                 { return el1.globalMetric().min < el2.globalMetric().min; })
                    ->globalMetric()
                    .min;
    }

    double max_prime {max};
    if (localAfter.max > globalBefore.max)
        max_prime = localAfter.max;
    if (globalBefore.max == localBefore.max and localBefore.max != localAfter.max)
    {
        if (localBefore.max < localAfter.max)
            max_prime = localAfter.max;
        else // iterating through the whole vector only as a last resort
            max_prime =
                std::max_element(std::execution::par_unseq, m_data.begin(), m_data.end(),
                                 [](const T &el1, const T &el2)
                                 { return el1.globalMetric().max < el2.globalMetric().max; })
                    ->globalMetric()
                    .max;
    }
    return {sum_prime, n_prime, u_prime, v_prime, s_prime, min_prime, max_prime};
}

template <HasMetric T>
Metrics<double> DataVector<T>::m_computeAfterRemove(const Metrics<double> &globalBefore,
                                                    const Metrics<double> &localBefore)
{
    return m_computeAfterUpdate(globalBefore, localBefore, {});
}

template <HasMetric T>
Metrics<double> DataVector<T>::m_computeAfterAdd(const Metrics<double> &globalBefore,
                                                 const Metrics<double> &localAfter)
{
    return m_computeAfterUpdate(globalBefore, {}, localAfter);
}

template <HasMetric T>
const Metrics<double> &DataVector<T>::globalMetric() const noexcept
{
    return m_globalMetric;
}

#endif // CRYPTOLYSER_ATTACKER_DATAVECTOR_HPP
