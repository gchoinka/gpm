#pragma once

#include <array>
#include <memory>
#include <utility>

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

template<typename ContexType>
class Prog3 : public BaseNode<ContexType>
{
public:
    template<typename ... Args>
    Prog3(Args && ...args)
        :children_{std::move(args)...}
    {
    }
    virtual void operator()(ContexType & contex) const override
    {
        for(auto & c: children_)
            (*c)(contex);
    }

    virtual ~Prog3() = default;
private:
    std::array<std::unique_ptr<BaseNode<ContexType>>, 3> children_;
};

template<typename ContexType>
class Prog2 : public BaseNode<ContexType>
{
public:
    template<typename ... Args>
    Prog2(Args && ...args)
        :children_{std::move(args)...}
    {
    }
    
    virtual void operator()(ContexType & contex) const override
    {
        for(auto & c: children_)
            (*c)(contex);
    }

    virtual ~Prog2() = default;
private:
    std::array<std::unique_ptr<BaseNode<ContexType>>, 2> children_;
};

template<typename ContexType>
class IfFoodAhead : public BaseNode<ContexType>
{
public:
    template<typename ... Args>
    IfFoodAhead(Args && ...args)
        :children_{std::move(args)...}
    {
    }
    virtual void operator()(ContexType & contex) const override
    {
        if(contex.is_food_in_front())
            (*(children_[0]))(contex);
        else
            (*(children_[1]))(contex);
    }

    virtual ~IfFoodAhead() = default;
private:
    std::array<std::unique_ptr<BaseNode<ContexType>>, 2> children_;
};

template<typename ContexType>
class Move : public BaseNode<ContexType>
{
public:
    virtual void operator()(ContexType & contex) const override
    {
        contex.move();
    }
    virtual ~Move() = default;
};

template<typename ContexType>
class Left : public BaseNode<ContexType>
{
public:
    virtual void operator()(ContexType & contex) const override
    {
        contex.left();
    }
    virtual ~Left() = default;
};

template<typename ContexType>
class Right : public BaseNode<ContexType>
{
public:
    virtual void operator()(ContexType & contex) const override
    {
        contex.right();
    }
    virtual ~Right() = default;
};
}
