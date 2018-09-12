#pragma once

#include <array>
#include <memory>
#include <utility>
#include <string_view>

#include <boost/mp11.hpp>


namespace antoop
{
    
template<typename ContexType>
class BaseNode
{
public:
    virtual void operator()(ContexType & contex) const = 0;
    virtual ~BaseNode() = 0; 
};

template<typename ContexType>
BaseNode<ContexType>::~BaseNode(){}


template<typename ContexType, int ChildrenCount, char ... CTName>
class BaseNodeWithChildren : public BaseNode<ContexType>
{
public:    
    static constexpr char const name[] = {CTName..., '\0'}; 
    template<typename ... Args>
    BaseNodeWithChildren(Args && ...args)
        :children_{std::move(args)...}
    {
    }

    std::array<std::unique_ptr<BaseNode<ContexType>>, ChildrenCount> children_;
};


template<typename ContexType>
class Prog3 : public BaseNodeWithChildren<ContexType, 3, 'p', '3'>
{
public:
    using BaseNodeWithChildren<ContexType, 3, 'p', '3'>::BaseNodeWithChildren;
    
    virtual void operator()(ContexType & contex) const override
    {
        for(auto & c: BaseNodeWithChildren<ContexType, 3, 'p', '3'>::children_)
            (*c)(contex);
    }
};


template<typename ContexType>
class Prog2 : public BaseNodeWithChildren<ContexType, 2, 'p', '2'>
{
public:
    using BaseNodeWithChildren<ContexType, 2, 'p', '2'>::BaseNodeWithChildren;
    
    virtual void operator()(ContexType & contex) const override
    {
        for(auto & c: BaseNodeWithChildren<ContexType, 2, 'p', '2'>::children_)
            (*c)(contex);
    }
};


template<typename ContexType>
class IfFoodAhead : public BaseNodeWithChildren<ContexType, 2, 'i', 'f'>
{
public:
    using BaseNodeWithChildren<ContexType, 2, 'i', 'f'>::BaseNodeWithChildren;
    
    virtual void operator()(ContexType & contex) const override
    {
        if(contex.is_food_in_front())
            (*(BaseNodeWithChildren<ContexType, 2, 'i', 'f'>::children_[0]))(contex);
        else
            (*(BaseNodeWithChildren<ContexType, 2, 'i', 'f'>::children_[1]))(contex);
    }
};


template<typename ContexType>
class Move : public BaseNodeWithChildren<ContexType, 0, 'm'>
{
public:
    virtual void operator()(ContexType & contex) const override
    {
        contex.move();
    }
};


template<typename ContexType>
class Left : public BaseNodeWithChildren<ContexType, 0, 'l'>
{
public:
    virtual void operator()(ContexType & contex) const override
    {
        contex.left();
    }
};


template<typename ContexType>
class Right : public BaseNodeWithChildren<ContexType, 0, 'r'>
{
public:
    virtual void operator()(ContexType & contex) const override
    {
        contex.right();
    }
};

namespace detail
{
    template<typename ContexType, typename Iter>
    std::unique_ptr<BaseNode<ContexType>> factory_imp(Iter &);

    template<typename ContexType, typename Iter>
    using FactoryMap = std::unordered_map<std::string_view, std::function<std::unique_ptr<BaseNode<ContexType>>(Iter &)>>;

    
    
    template<typename ContexType, typename Iter>
    struct FactoryMapInsertHelper 
    {
        FactoryMap<ContexType, Iter> & factoryMap; 
        
        template<class T> 
        void operator()(T) 
        {
            factoryMap[T::name] = [](Iter & tokenIter) 
            {
                auto ret = std::make_unique<T>();
                
                for(auto & n: ret->children_)
                    n = std::move(factory_imp<ContexType>(++tokenIter));
                return std::move(ret); 
            };
        }
    };

    template<typename ContexType, typename Iter>
    FactoryMap<ContexType, Iter> makeFactoryMap()
    {
        FactoryMap<ContexType, Iter> factoryMap;
        auto insertHelper = FactoryMapInsertHelper<ContexType, Iter>{factoryMap};
        boost::mp11::mp_for_each<boost::mp11::mp_list<Prog3<ContexType>, Prog2<ContexType>, IfFoodAhead<ContexType>, Move<ContexType>, Left<ContexType>, Right<ContexType>>>(insertHelper);
        return factoryMap;
    }

    template<typename ContexType, typename Iter>
    std::unique_ptr<BaseNode<ContexType>> factory_imp(Iter & tokenIter)
    {
        static auto nodeCreateFunMap = makeFactoryMap<ContexType, Iter>();
        auto token = *tokenIter;
        if(!nodeCreateFunMap.count(token))
        {
            throw std::runtime_error{std::string{"cant find factory function for token >>"} + std::string{token} + "<<"};
        }
            
        return nodeCreateFunMap[token](tokenIter);
    }
}


template<typename ContexType, typename Iter>
std::unique_ptr<BaseNode<ContexType>> factory(Iter tokenIter)
{
    return detail::factory_imp<ContexType>(tokenIter);
}
}
