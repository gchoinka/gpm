/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once

#include <sstream>
#include <string>
#include <string_view>
#include <boost/variant.hpp>

namespace gpm {
    
    template<typename StringT>
    struct Printer : public boost::static_visitor<StringT>
    {    
        template<typename T>
        StringT operator()(T const & b) const
        {
            char const * delimiter = "";
            char const * begin_delimiter = "";
            char const * end_delimiter = "";
            StringT children;
            for(auto const & n: b.nodes)
            {
                children += delimiter + boost::apply_visitor( *this, n );
                delimiter = " , ";
                begin_delimiter = "( ";
                end_delimiter = " )";
            }
            return StringT{T::name} + begin_delimiter + children + end_delimiter;
        }
    };

    template<typename StringT>
    struct RPNPrinter : public boost::static_visitor<StringT>
    {
        template<typename T>
        StringT operator()(T const & b) const
        {
            StringT children;
            for(auto const & n: b.nodes)
            {
                children = boost::apply_visitor( *this, n ) + " " + children;
            }
            return children + T::name;
        }
    };
    
    template<typename StringT>
    struct PNPrinter : public boost::static_visitor<StringT>
    {
        template<typename T>
        StringT operator()(T const & b) const
        {
            StringT children;
            for(auto const & n: b.nodes)
            {
                children = children + " " + boost::apply_visitor( *this, n );
            }
            return T::name + children;
        }
    };
    
    
    class RPNToken_iterator
    {
        std::string_view sv_;
        std::string_view::const_iterator currentPos_;
        
    public:
        RPNToken_iterator(std::string_view sv):sv_{sv}
        {
            currentPos_ = sv_.end();
            --currentPos_;
            for(; currentPos_ > sv_.begin(); --currentPos_)
            {
                if(*currentPos_ == ' ')
                {
                    currentPos_++;
                    break;
                }
            }
        }
        std::string operator*()
        {
            auto endIter = currentPos_ + 1;
            for(; endIter != sv_.end(); ++endIter)
            {
                if(*endIter == ' ')
                {
                    break;
                }
            }
            return std::string{sv_.substr(currentPos_ - sv_.begin(), endIter - currentPos_)};
        }
        
        RPNToken_iterator& operator++()
        {
            
            --currentPos_ ;
            if(currentPos_ > sv_.begin())
            {
                --currentPos_ ;
                for(; currentPos_ > sv_.begin(); --currentPos_)
                {
                    if(*currentPos_ == ' ')
                    {
                        ++currentPos_;
                        break;
                    }
                }
            }
            return *this;
        }
    };
    
    class PNToken_iterator
    {
        std::istringstream iss_; 
        std::string t_;
    public:
        PNToken_iterator(std::string const & s):iss_{s}
        {
            ++*this;
        }
        std::string const & operator*() const
        {
            return t_;
        }
        
        PNToken_iterator& operator++()
        {
            iss_ >> t_;
            return *this;
        }
        
    };
    
}
