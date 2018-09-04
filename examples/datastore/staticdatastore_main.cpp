/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#include <iostream>

#include "staticdatastore.h"
#include "staticdatastoreaccesscontrol.h"

#include <boost/type_index.hpp>
#include <tuple>
#include <boost/any.hpp>

// helper function to print a tuple of any size
template<typename Tuple, typename CurrentType, std::size_t currentN>
void printHelper(std::ostream & out, const Tuple& t, std::string const & postfix)
{
    out << "\t{\"keyId\":" << size_t(std::get<currentN>(t).keyTag) 
        << ",\"key\":" << std::get<currentN>(t).keyStr() 
        << ",\"typename\":" << boost::typeindex::type_id<CurrentType>().pretty_name() 
        << ",\"isset\":" <<  bool(std::get<currentN>(t).value) << "}" << postfix;
}
    

template<typename Tuple, std::size_t N>
struct TuplePrinter 
{
    static void print(std::ostream & out, const Tuple& t) 
    {
        constexpr size_t currentN = N-1;
        using CurrentType = typename std::tuple_element<currentN,typename std::decay<Tuple>::type>::type::Type;
        TuplePrinter<Tuple, N-1>::print(out, t);
        printHelper<Tuple,CurrentType, currentN> ( out, t, ",\n");
    }
};
 
template<typename Tuple>
struct TuplePrinter<Tuple, 1>
{
    static void print(std::ostream & out, const Tuple& t) 
    {
        constexpr size_t currentN = 0;
        using CurrentType = typename std::tuple_element<currentN,typename std::decay<Tuple>::type>::type::Type;
        printHelper<Tuple, CurrentType, currentN> ( out, t, ",\n" );
    }
};


        
template<class... Args>
void printKeyValueStore(std::ostream & out, const std::tuple<Args...>& t) 
{
    out << "[";
    TuplePrinter<decltype(t), sizeof...(Args)>::print(out, t);
    out << "]\n";
}



namespace MyLib{
class MyClass
{
    using ClassT = MyClass;
public:
    void doSomething(sds::StaticDataStore & ds)
    {
        sds::ac::get<sds::KeyTag::AntDirection>(this, ds) = 1;
    }
        
};
    
}


int main()
{
    constexpr char newl = '\n';
    using namespace sds;
    StaticDataStore ds;

    std::cout << "hasAntDirection(std::get):" << bool(std::get<size_t(KeyTag::AntDirection)>( ds ).value) << "\n";
    std::cout << "\nEmpty Store\n"; 
    printKeyValueStore(std::cout, ds);
    std::cout << "\n";
    std::cout << "\n";

    boost::any toset = 42;
    std::cout << "called: setAny(ds, \"AntDirection\", toset)\n";
    unsafe::setAny(ds, "AntDirection", toset);
    printKeyValueStore(std::cout, ds);
    std::cout << "\n";
    
    boost::any b = unsafe::getAny(ds, "AntDirection");
    std::cout << "hasAntDirection(getAny):" << bool(boost::any_cast<boost::optional<int>>(b)) << newl;
    std::cout << "hasAntDirection(std::get):" << bool(std::get<size_t(KeyTag::AntDirection)>(ds).value) << newl;
    std::cout << "hasAntDirection(get):" << bool(unsafe::get<boost::optional<int>>(ds, "AntDirection")) << newl;
    
    std::cout << "hasAntPos(get):" << bool(get<KeyTag::AntPos>(ds)) << newl;
    auto d = std::make_tuple(10,42);
    std::cout << "called: get<KeyTag::AntPos>( ds ) = d\n";
    get<KeyTag::AntPos>(ds) = d;
    
    std::cout << "hasAntPos(get):" << bool(get<KeyTag::AntPos>(ds)) << std::endl;
    
    printKeyValueStore(std::cout, ds);
    std::cout << "\n";
    
    std::cout << "called: std::get<size_t(KeyTag::AntPos)>(ds).value = boost::none\n";
    std::get<size_t(KeyTag::AntPos)>( ds ).value = boost::none;
    printKeyValueStore(std::cout, ds);
    std::cout << "\n";
    

    return 0;
}
