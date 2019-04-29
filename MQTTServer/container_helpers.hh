#ifndef CONTAINER_HELPERS_HH
#define CONTAINER_HELPERS_HH


#include <type_traits>
#include <numeric>
#include <optional>
#include <algorithm>

#include <iostream>


namespace Terryn
{

namespace Detail
{

/// has_size
/// used to determine if the type has a size() call
/// this dictates if this is NOT a std::forward_list
/// or similar type.
template<class , class = void>
struct has_size : std::false_type {};

template<class T>
struct has_size<T,
                std::void_t<decltype(&T::size) > > :
    std::is_same<std::size_t, decltype(std::declval<T>().size())> {};


template<typename T>
class has_back_method
{
private:

    using yes = std::true_type;
    using no = std::false_type;

    template<typename U> static auto test(int) -> decltype (std::declval<U>().back(), yes());
    template<typename> static no test(...);

public:
    static constexpr bool value = std::is_same<decltype (test<T>(0)), yes>::value;
};


template<typename T>
class has_front_method
{
private:

    using yes = std::true_type;
    using no = std::false_type;

    template<typename U> static auto test(int) -> decltype (std::declval<U>().front(), yes());
    template<typename> static no test(...);

public:
    static constexpr bool value = std::is_same<decltype (test<T>(0)), yes>::value;
};



template<typename T>
class has_at_method
{
private:

    using size_type = typename T::size_type;

    using yes = std::true_type;
    using no = std::false_type;

    template<typename U> static auto test(int) -> decltype (std::declval<U>().at(std::declval<size_type>()), yes());
    template<typename> static no test(...);

public:
    static constexpr bool value = std::is_same<decltype (test<T>(0)), yes>::value;
};

template <class, class = void>
struct has_random_access : std::false_type {};

template<class T>
struct has_random_access<T,
                         std::void_t<decltype(&T::at, T::sizeType) > > :
    std::is_same<typename T::const_reference, decltype(std::declval<T>().at(T::size_type))> {};

/// dynamic_array_type
/// Returns a true_type if the type has a clear() function
/// call, which is indicitive of it being a Dynamiclly Allocated Array.
/// This allows compile time switches to handle std::array<T> or T[]
/// types as needed.
template<class, class = void>
struct dynamic_array_type : std::false_type {};

template<class T>
struct dynamic_array_type<T,
                          std::void_t<decltype(&T::clear) > > :
    std::is_same<void, decltype(std::declval<T>().clear())> {};

enum class MinMax
{
    Min, Max
};

template<typename T, MinMax MinOrMax>
void ResetValue(T &t)
{
    if constexpr(MinOrMax == MinMax::Max)
    {
        t = std::numeric_limits<T>::max();
    }
    else
    {
        t = std::numeric_limits<T>::min();
    }
}

}

template<typename CollectionType,
          typename CollectionValueType = typename CollectionType::value_type
          >
bool HasRandomAccess(const CollectionType &coll)
{
    static_assert(Detail::has_at_method<CollectionType>::value/* &&
                  Detail::has_front_value<CollectionType>::value*/,
                  "Expected type does not have random access");
    return true;
}

template<typename CollectionType,
          typename CollectionValueType = typename CollectionType::value_type
          >
bool DoesNotHaveRandomAccess(const CollectionType &coll)
{
    static_assert(!Detail::has_random_access<CollectionType>::value,
                  "Expected type does not have random access");
    return true;
}



template <typename CollectionType,
          typename CollectionValueType = typename CollectionType::value_type
          >
std::optional<CollectionValueType> GetValueAtIndex(const CollectionType &collection, uint32_t index)
{
    if constexpr(Detail::has_random_access<CollectionType>::value)
    {
        std::cout << "Running O(1) detection" << std::endl;
        if(index > collection.size())
            return std::nullopt;
        return collection.at(index);
    }
    else
    {
        std::cout << "running O(N) detection" << std::endl;
        auto itr = collection.begin();
        while(index -- > 0)
        {
            if(itr == collection.end())
                return std::nullopt;
            itr ++;
        }
        return *itr;
    }
}


template<typename CollectionType>
auto Count(const CollectionType &collection)
{
    if constexpr(Detail::has_size<CollectionType>::value)
    {
        return std::size(collection);
    }
    return static_cast<size_t>(std::distance(std::begin(collection), std::end(collection)));
}

template<typename CollectionType,
          typename CollectionValueType = typename CollectionType::value_type
          >
auto Push(CollectionType &collection, const CollectionValueType &value)
{
    if constexpr(Detail::has_size<CollectionType>::value)
    {
        collection.push_back(value);
    }
    else
    {
        collection.push_front(value);
    }
}

template<typename CollectionType,
          typename CollectionValueType = typename CollectionType::value_type,
          typename SortingFunction
          >
void Sort(CollectionType &collection, SortingFunction &&funct)
{
    if constexpr(Detail::has_size<CollectionType>::value)
    {
        std::sort(std::begin(collection), std::end(collection), std::forward<SortingFunction>(funct));
    }
    else
    {
        collection.sort(std::forward<SortingFunction>(funct));
    }
}


template<typename CollectionType,
          Detail::MinMax MinOrMax = Detail::MinMax::Min,
          typename CollectionValueType = typename CollectionType::value_type
          >
void ResetCollection(CollectionType & collection)
{
    if constexpr(Detail::dynamic_array_type<CollectionType>::value)
    {
        collection.clear();
    }
    else
    {
        for(auto & c : collection)
        {
            c = std::numeric_limits<CollectionValueType>::min();
        }
    }
}

template<typename CollectionType,
          typename ReturnType = typename CollectionType::value_type>
auto Avg(const CollectionType &collection) -> ReturnType
{
    using CollectionValueType = typename CollectionType::value_type;

    static_assert(std::is_fundamental_v<CollectionValueType>,
                  "Average can only be ran with fundamental types");

    return std::accumulate(std::begin(collection), std::end(collection), ReturnType(0))
        / Count(collection);
}


/// contains
template<typename CollectionType,
          typename ItemType
          >
auto Contains(const CollectionType& collection, const ItemType &item)
{
    return std::find(std::begin(collection), std::end(collection), item) != std::end(collection);
}


/// UNTESTED
template<typename CollectionType,
          typename ItemType = typename CollectionType::value_type
          >
std::optional<uint64_t> IndexOfSmaller(const CollectionType &collection, ItemType value)
{
    static_assert(std::is_same_v<ItemType,  typename CollectionType::value_type>,
                  "ItemType must be of the same type as the collection's value_type");

    uint64_t s = 0;
    for(const auto &v : collection)
    {
        if(v < value)
        {
            return s;
        }
        s++;
    }
    return std::nullopt;
}

/// UNTESTED
template<typename CollectionType,
          typename ItemType = typename CollectionType::value_type
          >
void ReplaceAtIndex(CollectionType &collection, ItemType value, uint64_t index)
{
    if constexpr(Detail::has_size<CollectionType>::value)
    {
        collection.at(index) = value;
    }
    else
    {
        size_t z = 0;
        for(auto &c : collection)
        {
            if (z == index)
            {
                c = value;
                return;
            }
            z++;
        }
    }
}


template<typename CollectionType,
          typename CollectionValueType = typename CollectionType::value_type
          >
CollectionType LargestN(const CollectionType &collection, uint32_t count)
{
    static_assert(Detail::dynamic_array_type<CollectionType>::value,
                  "Collection type must be dynamic array for the return value to work."
                  "If you want to use a static array, please use: "
                  " LargestN(const CollectionType&, OtherCollectionType&, count");

    CollectionType largestCollection;
    for(auto t : collection)
    {
        if(!Contains(largestCollection, t))
        {
            if constexpr(Detail::dynamic_array_type<CollectionType>::value)
            {
                if((Count(largestCollection) < count))
                {
                    Push(largestCollection, t);
                }
                else
                {
                    auto index = IndexOfSmaller(largestCollection, t);
                    if(index)
                    {
                        ReplaceAtIndex(largestCollection, t, index.value());
                    }
                }
            }
            else
            {
                auto index = IndexOfSmaller(largestCollection, t);
                if(index)
                {
                    ReplaceAtIndex(largestCollection, t, index.value());
                }
            }
        }
    }

    Sort(largestCollection, std::greater<CollectionValueType>());

    //    std::sort(begin(largestCollection), end(largestCollection),
    //              std::greater<CollectionValueType>());

    return largestCollection;
}

/// UNTESTED
template<typename CollectionType,
          typename OtherCollectionType,
         typename CollectionValueType = typename CollectionType::value_type,
         typename OtherCollectionValueType = typename OtherCollectionType::value_type
          >
void LargestN(const CollectionType& collection, OtherCollectionType &otherCollection,
              uint32_t count)
{
    static_assert(std::is_same_v<CollectionValueType, OtherCollectionValueType>,
                  "Collection types must be of the same base type");
    static_assert(std::is_fundamental_v<CollectionValueType>,
                  "Collection type must be fundamental type");
    static_assert(std::is_fundamental_v<OtherCollectionValueType>,
                  "Collection type must be fundamental type");

    ResetCollection(otherCollection);

    for(auto t : collection)
    {
        if(!Contains(otherCollection, t))
        {
            if constexpr(Detail::dynamic_array_type<OtherCollectionType>::value)
            {
                if((Count(otherCollection) < count))
                {
                    Push(otherCollection, t);
                }
                else
                {
                    auto index = IndexOfSmaller(otherCollection, t);
                    if(index)
                    {
                        ReplaceAtIndex(otherCollection, t, index.value());
                    }
                }
            }
            else
            {
                auto index = IndexOfSmaller(otherCollection, t);
                if(index)
                {
                    ReplaceAtIndex(otherCollection, t, index.value());
                }
            }
        }
    }

    Sort(otherCollection, std::greater<OtherCollectionValueType>());
}



template<typename CollectionTypeOne, typename CollectionTypeTwo>
bool CompareCollections(const CollectionTypeOne &one, const CollectionTypeTwo &two)
{
    if(Count(one) != Count(two))
    {
        return false;
    }
    auto a = std::begin(one);
    auto b = std::begin(two);

    while(a != std::end(one) && b != std::end(two))
    {
        if(*a != *b)
        {
            return false;
        }
        a++;
        b++;
    }
    return true;
}


template<typename CollectionType, typename T>
std::optional<size_t> FirstIndexOf(const CollectionType &collection, const T &value)
{
    auto found = std::find(std::begin(collection), std::end(collection), value);
    if(found == std::end(collection))
        return std::nullopt;

    return std::distance(std::begin(collection), found);
}


}

#endif // CONTAINER_HELPERS_HH
