#ifndef CRYPTOLYSER_ATTACKER_FILTER_HPP
#define CRYPTOLYSER_ATTACKER_FILTER_HPP

#include "DataProcessing/DataVector/DataVector.hpp"
#include "DataProcessing/SampleData/SampleData.hpp"

#include <type_traits>
namespace Filter
{

template <typename T>
struct IsDataVectorChainOfSampleData : std::false_type
{
};

template <Real R>
struct IsDataVectorChainOfSampleData<DataVector<SampleData<R>>> : std::true_type
{
};

template <typename T>
    requires DataVectorChain<T>
struct IsDataVectorChainOfSampleData<T> : IsDataVectorChainOfSampleData<typename T::InnerType>
{
};

template <typename T>
concept DataVectorChainOfSampleData = IsDataVectorChainOfSampleData<T>::value;

template <Real R, typename T = SampleData<R>>
SampleData<R> filter(const SampleData<R> &sampleData, const std::predicate<R> auto &keepCriterion)
{
    SampleData<R> result {};
    for (const R elem : sampleData)
        if (keepCriterion(elem))
            result.insert(elem);
    return result;
}

template <Real R, DataVectorChainOfSampleData DataVectorType>
DataVectorType filter(const DataVectorType &dataVector, const std::predicate<R> auto &keepCriterion)
{
    DataVectorType result {};
    for (const typename DataVectorType::InnerType &elem : dataVector)
    {
        // elem can either be another DataVector, or it can be the final SampleData<R>, in which
        // case it will use the specific overload of filter for the SampleDataType
        result.add(filter<R, typename DataVectorType::InnerType>(elem, keepCriterion));
    }
    return result;
}

} // namespace Filter
#endif // CRYPTOLYSER_ATTACKER_FILTER_HPP
