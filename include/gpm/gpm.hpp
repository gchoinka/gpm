/*
 * Copyright: 2018 Gerard Choinka (gerard.choinka@gmail.com)
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or
 * copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#pragma once 

#include <exception>
#include <array>
#include <unordered_map>
#include <random>
#include <vector>

#include <boost/variant.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/count_if.hpp>
#include <boost/type_index.hpp>


#include <gpm/io.hpp>


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
    struct Builder : public boost::static_visitor<void>
    { 
        Builder(int height, std::function<VariantType()> termialGen, std::function<VariantType()> noneTermialGen)
        :height_{height}, termialGen_{termialGen}, noneTermialGen_{noneTermialGen}
        {
            
        }
        
        int height_ = 0;
        std::function<VariantType()> termialGen_;
        std::function<VariantType()> noneTermialGen_;
        
        template<typename T>
        void operator()(T & node) const
        {

            if(height_ >= 0)
            { 

                for(auto & childNode: node.nodes)
                {
                    childNode = noneTermialGen_();
                    auto sub = Builder<VariantType>{height_-1, termialGen_, noneTermialGen_};
                    boost::apply_visitor(sub, childNode);
                }
            }
            else
            {
                for(auto & childNode: node.nodes)
                {
                    childNode = termialGen_();
                }
            }
        }
    };
    
    template<typename VariantType>
    class BasicGenerator 
    {
    public:
        BasicGenerator(int minHeight, int maxHeight, unsigned int rndSeed = 5489u)
            :minHeight_{minHeight}, maxHeight_{maxHeight}, rnd_{rndSeed}
        {
            boost::mpl::for_each<typename VariantType::types>([&, index = 0]([[gnu::unused]]auto arg) mutable{
                if constexpr(std::tuple_size<decltype(arg.nodes)>::value == 0)
                    terminalNodes_.push_back(std::move(arg));
                else
                    noneTerminalNodes_.push_back(std::move(arg));
            });
            
            
        }
        
//         void fill(VariantType & toFill, int hight, std::uniform_int_distribution<size_t> & dist)
//         {
//             if(hight > 0)
//             {
//                 toFill = noneTerminalNodes_[dist(rnd_)];
//                 for(auto & childNode: toFill.nodes)
//                     fill(childNode, hight-1, dist);
//             }
//             else
//             {
//                 toFill = terminalNodes_[dist(rnd_)];
//             }
//                 
//         }
        
        VariantType operator()()
        {
            std::uniform_int_distribution<> height{minHeight_, maxHeight_};
            std::uniform_int_distribution<size_t> randomNoneTermSelector{0, noneTerminalNodes_.size()-1};
            std::uniform_int_distribution<size_t> randomTermSelector{0, terminalNodes_.size()-1};
            
            
            auto makeTermial = [&](){ return terminalNodes_[randomTermSelector(rnd_)]; };
            auto makeNoneTermial = [&](){ return noneTerminalNodes_[randomNoneTermSelector(rnd_)]; };
            
            VariantType rootNode = makeNoneTermial();
            
//             std::function<void(VariantType&, int)> fillerFunction = [&]( VariantType& toFill, int height){
//                 if(height > 0)
//                 {
//                     toFill = makeNoneTermial();
//                     
//                 }
//                 
//             };
            boost::apply_visitor( [&](auto rootNode)
                {
                    for(auto & childNode: rootNode.nodes)
                    {
                        Builder<VariantType> b{height(rnd_)-1, makeTermial, makeNoneTermial};
                        childNode = makeNoneTermial();
                        boost::apply_visitor( b, childNode );
                    }
                },
                rootNode
                   );

             std::cout << terminalNodes_.size() << " " << noneTerminalNodes_.size() << " " << height(rnd_) << " " << 
                 boost::apply_visitor([](auto val) { return boost::typeindex::type_id_runtime(val).pretty_name(); }, rootNode)  << " " <<
                 boost::apply_visitor(gpm::RPNPrinter<std::string>{}, rootNode) << "\n";
            return rootNode;
        }
        

    private:
        int minHeight_;
        int maxHeight_;
        std::vector<VariantType> terminalNodes_;
        std::vector<VariantType> noneTerminalNodes_;
        std::mt19937 rnd_;
        //std::uniform_int_distribution<> rnd_dis_;
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
        
        constexpr char const * nameStr() const
        {
            return CTString::name;
        }
    };
    

}
