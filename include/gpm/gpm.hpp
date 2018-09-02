#pragma once 

#include <exception>
#include <array>
#include <unordered_map>
#include <string>
#include <boost/variant.hpp>
#include <boost/mpl/for_each.hpp>
#include <string_view>
#include <random>


namespace gpm
{
    class GPMException : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };
    
    namespace detail
    {
        template<typename VariantType, typename Iter>
        VariantType factory_imp(Iter &);

        template<typename VariantType, typename Iter>
        using FactoryMap = std::unordered_map<std::string, std::function<VariantType(Iter &)>>;

        
        template<typename VariantType, typename Iter>
        struct FactoryMapInsertHelper 
        {
            FactoryMap<VariantType, Iter> & factoryMap; 
            
            template<class T> 
            void operator()(T) 
            {
                factoryMap[T::name] = [](Iter & tokenIter) 
                {
                    T ret;
                    for(auto & n: ret.nodes)
                        n = factory_imp<VariantType>(++tokenIter);
                    return ret; 
                };
            }
        };

        template<typename VariantType, typename Iter>
        FactoryMap<VariantType, Iter> makeFactoryMap()
        {
            FactoryMap<VariantType, Iter> factoryMap;
            auto insertHelper = FactoryMapInsertHelper<VariantType, Iter>{factoryMap};
            boost::mpl::for_each<typename VariantType::types>(insertHelper);
            return factoryMap;
        }

        template<typename VariantType, typename Iter>
        VariantType factory_imp(Iter & tokenIter)
        {
            static auto nodeCreateFunMap = makeFactoryMap<VariantType, Iter>();
            auto token = *tokenIter;
            if(!nodeCreateFunMap.count(token))
            {
                throw GPMException{std::string{"cant find factory function for token >>"} + token + "<<"};
            }
                
            return nodeCreateFunMap[token](tokenIter);
        }
    }


    template<typename VariantType, typename Iter>
    VariantType factory(Iter tokenIter)
    {
        return detail::factory_imp<VariantType>(tokenIter);
    }

    
    template<typename VariantType>
    class BasicGenerator
    {
    public:
        BasicGenerator(int minHeight, int maxHeight, unsigned int rndSeed = 5489u)
            :minHeight_{minHeight}, maxHeight_{maxHeight}, rnd_{rndSeed}
        {
        }
        
        VariantType operator()()
        {
            return VariantType{};
        }
        

    private:
        int minHeight_;
        int maxHeight_;
        std::mt19937 rnd_;
        std::uniform_int_distribution<> rnd_dis_;
    };

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
    
    template<char ... ch> 
    struct NodeToken
    {
        constexpr static char name[] = {ch..., '\0'};
    };
    
    template<typename VariantType, int NodeCount, typename CTString>
    struct BaseNode : public CTString
    {
        template<typename ... Args>
        BaseNode(Args && ... args):nodes{std::forward<Args>(args)...}{}
            
        std::array<VariantType, NodeCount> nodes;
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
