/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include <boost/any.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <boost/mpl/string.hpp>
#include <memory>
#include <tuple>
#include <stdexcept>
#include <typeinfo>

namespace sds {

template<char... letters>
struct string_t
{
    static char const * c_str() 
    {
        static constexpr char string[]={letters... , '\0'};
        return string;
    }
};

#define MACRO_GET_1(str, i) \
(sizeof(str)-2 > (i) ? str[(i)] : 0)

#define MACRO_GET_4(str, i) \
MACRO_GET_1(str, i+0),  \
MACRO_GET_1(str, i+1),  \
MACRO_GET_1(str, i+2),  \
MACRO_GET_1(str, i+3)

#define MACRO_GET_16(str, i) \
MACRO_GET_4(str, i+0),   \
MACRO_GET_4(str, i+4),   \
MACRO_GET_4(str, i+8),   \
MACRO_GET_4(str, i+12)

#define MACRO_GET_64(str, i) \
MACRO_GET_16(str, i+0),  \
MACRO_GET_16(str, i+16), \
MACRO_GET_16(str, i+32), \
MACRO_GET_16(str, i+48)

#define CT_STR(str) string_t<MACRO_GET_64(#str, 1), 0 >

template< typename ValueType , typename KeyType , KeyType keyTag_ , typename StaticKeyStr > 
class KeyDataPair
{
public:
    using Type = ValueType;
    using Key = StaticKeyStr;
    boost::optional<ValueType> value;
    static constexpr KeyType keyTag = keyTag_;
    inline const char * keyStr() const { return StaticKeyStr::c_str();  }
};

enum class KeyTag : size_t 
{ 
    AntDirection, 
    AntPos, 
    FoodEaten, 
    StepsDone , 
    
    KeyTagSize // do not remove this last value, the template meta function skiping the last element (this), 
               // if you remove this value you have to rewrite the conditions  
};

template<typename ValueType, KeyTag keyTag_, typename StaticKeyStr>
using Entry = KeyDataPair<ValueType, KeyTag, keyTag_, StaticKeyStr>;


using StaticDataStore = std::tuple
<
    Entry<int,                  KeyTag::AntDirection,  CT_STR("AntDirection")>,
    Entry<std::tuple<int,int>,  KeyTag::AntPos,        CT_STR("AntPos")>,
    Entry<int,                  KeyTag::FoodEaten,     CT_STR("FoodEaten")> ,
    Entry<int,                  KeyTag::StepsDone,     CT_STR("StepsDone")>
>;

constexpr auto ValueKeyStoreSize = std::tuple_size<StaticDataStore>::value;

static_assert( size_t(KeyTag::KeyTagSize) == ValueKeyStoreSize, "StaticDataStore has less or more KeyDataPair than KeyTag" );

#undef CT_STR
#undef MACRO_GET_64
#undef MACRO_GET_16
#undef MACRO_GET_4
#undef MACRO_GET_1
        

class StaticDataStoreException : public std::runtime_error { public: using std::runtime_error::runtime_error; };
class StaticDataStoreKeyNotFound : public StaticDataStoreException { public: using StaticDataStoreException::StaticDataStoreException; };
class StaticDataStoreTypIsNotConvertiable : public StaticDataStoreException , boost::bad_any_cast
{
public: 
        using StaticDataStoreException::StaticDataStoreException;
};

namespace detail
{
    // helper for setAny
    template<typename Tuple, std::size_t N>
    struct TupleAnySetter 
    {
        static void set(Tuple& t, std::string const & key, boost::any a) 
        {
            constexpr size_t currentN = N-1;
            using CurrentType = typename std::tuple_element<currentN, typename std::decay<Tuple>::type>::type::Type;
            if( key == std::get<currentN>(t).keyStr() )
                std::get<currentN>(t).value = boost::any_cast<CurrentType>(a);
            else
                TupleAnySetter<Tuple, N-1>::set(t, key, a);
        }
    };
    
    template<typename Tuple>
    struct TupleAnySetter<Tuple, 0>
    {
        static void set(Tuple& , std::string const & key, boost::any ) 
        {
            throw StaticDataStoreKeyNotFound("Key \"" + key +"\" is not known in DataStore");
        }
    };
    
    
    // helper for getAny
    template<typename Tuple, std::size_t N>
    struct TupleAnyGetter 
    {
        static boost::any get(Tuple& t, std::string const & key) 
        {
            constexpr size_t currentN = N-1;
            if( key == std::get<currentN>(t).keyStr() )
                return boost::any(std::get<currentN>(t).value);
            else
                return TupleAnyGetter<Tuple, N-1>::get(t, key);
        }
    };
    
    template<typename Tuple>
    struct TupleAnyGetter<Tuple, 0>
    {
        static boost::any get(Tuple& , std::string const & key) 
        {
            throw StaticDataStoreKeyNotFound("Key \"" + key +"\" is not known in DataStore");
        }
    };

    
    
    template<typename ReturnType, typename CurrentType, bool is_convertible = std::is_convertible<CurrentType, ReturnType>::value>
    struct EnsureIsConvertiable
    {
        static ReturnType forward(boost::optional<CurrentType> & ) 
        {
            return ReturnType();
        }
    };

    template<typename ReturnType, typename CurrentType>
    struct EnsureIsConvertiable<ReturnType, CurrentType, true>
    {
        static ReturnType forward(boost::optional<CurrentType> & toForward) 
        {
            return toForward;
        }
    };

    template<typename ReturnType, typename CurrentType>
    struct EnsureIsConvertiable<ReturnType, CurrentType, false>
    {
        static ReturnType forward(boost::optional<CurrentType> & ) 
        {
            throw StaticDataStoreTypIsNotConvertiable("is not is_convertible");
            return ReturnType();
        }
    };

    template<typename ReturnType, typename Tuple, std::size_t N>
    struct TupleGetter 
    {
        static ReturnType getByKeyStr(Tuple& t, std::string const & key) 
        {
            constexpr size_t currentN = N-1;
            using CurrentType = typename std::tuple_element<currentN,typename std::decay<Tuple>::type>::type::Type;
            if( key == std::get<currentN>(t).keyStr() )
                return EnsureIsConvertiable<ReturnType, CurrentType>::forward( std::get<currentN>(t).value );
            else
                return TupleGetter<ReturnType, Tuple, N-1>::getByKeyStr(t, key);
        }
    };
    
    template<typename ReturnType, typename Tuple>
    struct TupleGetter<ReturnType, Tuple, 0>
    {
        static ReturnType getByKeyStr(Tuple& , std::string const & key) 
        {
            throw StaticDataStoreKeyNotFound("Key \"" + key +"\" is not known in DataStore");
        }
    };
}

namespace unsafe
{
    template<class... Args>
    void setAny(std::tuple<Args...>& t, std::string const & key, boost::any a) 
    {
        try
        {
            detail::TupleAnySetter<decltype(t), sizeof...(Args)>::set(t, key, a);
        }
        catch(boost::bad_any_cast const & exp)
        {
            throw StaticDataStoreTypIsNotConvertiable(exp.what());
        }
    }
    
            
    template<class... Args>
    boost::any getAny(std::tuple<Args...>& t, std::string const & key) 
    {
        try
        {
            return detail::TupleAnyGetter<decltype(t), sizeof...(Args)>::get(t, key);
        }
        catch(boost::bad_any_cast const & exp)
        {
            throw StaticDataStoreTypIsNotConvertiable(exp.what());
        }
    }

            
    template<typename ReturnType, class... Args>
    ReturnType get([[gnu::unused]]std::tuple<Args...>& t, std::string const & key) 
    {
        return detail::TupleGetter<ReturnType, decltype(t), sizeof...(Args)>::getByKeyStr( t , key );
    }
}


template<KeyTag keyTag,  class... Args>
auto get([[gnu::unused]]std::tuple<Args...>& t) -> boost::optional<typename std::tuple_element<size_t(keyTag), typename std::decay<std::tuple<Args...>>::type>::type::Type>&
{
    return std::get<size_t(keyTag)>( t ).value;
}

}
