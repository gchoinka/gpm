#include <iostream>
#include <utility>
#define FMT_HEADER_ONLY
#include "fmt/format.h"

#include <initializer_list>
#include <tuple>

#include <gpm/utils/fmtutils.hpp>


/* IMPLEMENTATION */
// https://stackoverflow.com/questions/6713420/c-convert-integer-to-string-at-compile-time

/* calculate absolute value */
constexpr int abs_val (int x)
    { return x < 0 ? -x : x; }

/* calculate number of digits needed, including minus sign */
constexpr int num_digits (int x)
    { return x < 0 ? 1 + num_digits (-x) : x < 10 ? 1 : 1 + num_digits (x / 10); }

/* metaprogramming string type: each different string is a unique type */
template<char... args>
struct metastring {
    const char data[sizeof... (args)] = {args...};
};

/* recursive number-printing template, general case (for three or more digits) */
template<int size, int x, char... args>
struct numeric_builder {
    typedef typename numeric_builder<size - 1, x / 10, '0' + abs_val (x) % 10, args...>::type type;
};

/* special case for two digits; minus sign is handled here */
template<int x, char... args>
struct numeric_builder<2, x, args...> {
    typedef metastring<x < 0 ? '-' : '0' + x / 10, '0' + abs_val (x) % 10, args...> type;
};

/* special case for one digit (positive numbers only) */
template<int x, char... args>
struct numeric_builder<1, x, args...> {
    typedef metastring<'0' + x, args...> type;
};

/* convenience wrapper for numeric_builder */
template<int x>
class numeric_string
{
private:
    /* generate a unique string type representing this number */
    typedef typename numeric_builder<num_digits (x), x, '\0'>::type type;

    /* declare a static string of that type (instantiated later at file scope) */
    static constexpr type value {};

public:
    /* returns a pointer to the instantiated string */
    static constexpr const char * get ()
        { return value.data; }
};

/* instantiate numeric_string::value as needed for different numbers */



template<int x>
constexpr typename numeric_string<x>::type numeric_string<x>::value;


bool replacePlaceHolderSingel(std::string & str, std::string_view const & key, std::string_view const & to)
{
    std::string from = "{" + std::string{key} + "}";
    if(str.empty())
      return false;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos+1, from.length()-2, to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
    return true;
}



template<typename Tuple, size_t...Idx>
decltype(auto) fmtinvoke(std::string fstr, const Tuple& args, std::integer_sequence<size_t, Idx...>) 
{
    auto dummy = {(replacePlaceHolderSingel(fstr, std::get<Idx*2>(args), numeric_string<Idx>::get()), 0)..., };
    (void)dummy;
    return fmt::format(fstr, std::get<Idx*2+1>(args)...);
}


template<typename StrT, typename ...T>
decltype(auto) mapformat(StrT && fstr, T ... args)
{
  return fmtinvoke(std::forward<StrT>(fstr), std::tuple<T const &...>{args...}, std::make_integer_sequence<size_t, sizeof...(args)/2>() );
}


// template<typename StrT, typename Tuple, size_t...Idx>
// decltype(auto) formatNamed_imp(StrT && fstr, Tuple&& args, std::integer_sequence<size_t, Idx...>) 
// {
//     return fmt::format(std::forward<StrT>(fstr), fmt::arg(std::get<Idx*2>(std::forward<Tuple>(args)), std::get<Idx*2+1>(std::forward<Tuple>(args)))...);
// }
// 
// 
// template<typename StrT, typename ...T>
// decltype(auto) formatNamed(StrT && fstr, T && ... args)
// {
//   return formatNamed_imp(std::forward<StrT>(fstr), std::tuple<T...>{std::forward<T>(args)...}, std::make_integer_sequence<size_t, sizeof...(args)/2>() );
// }


int main() {

  std::cout << mapformat(R"""(
{indent}if({cond})
{indent}{{
{indent}    {true_case}
{indent}}}
{indent}else
{indent}{{
{indent}    {false_case}
{indent}}}
)""" 
    , "cond", 42 
    , "true_case", "p3"
    , "false_case", "Nope"
    , "indent", "    "
  ); 
  std::cout << gpm::utils::formatNamed(R"""(
{indent}if({cond})
{indent}{{
{indent}    {true_case}
{indent}}}
{indent}else
{indent}{{
{indent}    {false_case}
{indent}}}
)""" 
    , "cond", 42 
    , "true_case", "p3"
    , "false_case", "Nope"
    , "indent", "    "
    , "su"
  ); 
  
  
  std::cout << fmt::format("Hello, {name}! The answer is {number}. Goodbye, {name}.",
           fmt::arg("name", "World"), fmt::arg("number", 42)) << "\n";
}
