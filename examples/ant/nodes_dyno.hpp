#include <dyno.hpp>
#include <array>
#include <functional>

#include <boost/container/flat_map.hpp>
#include <boost/mp11.hpp>


using namespace dyno::literals;


template<typename ContexT>
struct AntBoardSimEvaluable : decltype(dyno::requires(
  "eval"_s = dyno::method<void (ContexT&) const>
)) { };


// Define how concrete types can fulfill that interface
template <typename ContexT, typename T>
auto const dyno::default_concept_map<AntBoardSimEvaluable<ContexT>, T> = dyno::make_concept_map(
  "eval"_s = [](T const& self, ContexT& c) { self.eval(c); }
);


template <typename ContexT>
struct evalable {
  template <typename T>
  evalable(T x) : poly_{x} { }
  
  void eval(ContexT& c) const
  { poly_.virtual_("eval"_s)(c); }
  
private:
  using Storage = dyno::shared_remote_storage;
  using VTable = dyno::vtable<dyno::remote<dyno::everything>>;
  dyno::poly<AntBoardSimEvaluable<ContexT>, Storage, VTable> poly_;
};

namespace antdyno
{
  
  template<typename ContexT>
  struct Move  
  {
    static constexpr char name[] = {"m"};
    std::array<evalable<ContexT>, 0> children;
    void eval(ContexT & c) const{
      c.move();
    }
  };
  
  template<typename ContexT>
  struct Left  
  {
    static constexpr char name[] = {"l"};
    std::array<evalable<ContexT>, 0> children;
    void eval(ContexT & c) const{
      c.left();
    }
  };
  
  template<typename ContexT>
  struct Right  
  {
    static constexpr char name[] = {"r"};
    std::array<evalable<ContexT>, 0> children;
    void eval(ContexT & c) const{
      c.right();
    }
  };
  
  
  template<typename ContexT>
  struct IfFood  
  {
    static constexpr char name[] = {"if"};
    std::array<evalable<ContexT>, 2> children;
    IfFood(evalable<ContexT> && trueCase, evalable<ContexT> && falseCase):children{trueCase, falseCase} {}
    IfFood():children{Move<ContexT>{}, Move<ContexT>{}} {}
    
    
    void eval(ContexT & c) const{
      if(c.is_food_in_front())
        children[0].eval(c);
      else
        children[1].eval(c);
    }
  };
  
  template<typename ContexT>
  struct Prog2  
  {
    static constexpr char name[] = {"p2"};
    std::array<evalable<ContexT>, 2> children;
    Prog2(evalable<ContexT> && child0, evalable<ContexT> && child1):children{child0, child1} {}
    Prog2():children{Move<ContexT>{}, Move<ContexT>{}} {}
    
    void eval(ContexT & c) const{
      children[0].eval(c);
      children[1].eval(c);
    }
  };
  
  template<typename ContexT>
  struct Prog3  
  {
    static constexpr char name[] = {"p3"};
    std::array<evalable<ContexT>, 3> children;
    Prog3(evalable<ContexT> && child0, evalable<ContexT> && child1, evalable<ContexT> && child2):children{child0, child1, child2} {}
    Prog3():children{Move<ContexT>{}, Move<ContexT>{}, Move<ContexT>{}} {}
    
    void eval(ContexT & c) const{
      children[0].eval(c);
      children[1].eval(c);
      children[2].eval(c);
    }
  };
  

  
  
  
  namespace detail {
    template <typename ContexT, typename Iter>
    evalable<ContexT> factory_imp(Iter &);
    
    template <typename ContexT, typename Iter>
    using FactoryMap = boost::container::flat_map<
    std::string_view,
    std::function<evalable<ContexT>(Iter &)>>;
    
    template <typename ContexT, typename Iter>
    struct FactoryMapInsertHelper {
      FactoryMap<ContexT, Iter> &factoryMap;
      
      template <class T>
      void operator()(T) {
        factoryMap[T::name] = [](Iter &tokenIter) {
          auto ret = T{};
          
          for (auto &n : ret.children)
            n = factory_imp<ContexT>(++tokenIter);
          return ret;
        };
      }
    };
    
    template <typename ContexT, typename Iter>
    FactoryMap<ContexT, Iter> makeFactoryMap() {
      FactoryMap<ContexT, Iter> factoryMap;
      auto insertHelper = FactoryMapInsertHelper<ContexT, Iter>{factoryMap};
      boost::mp11::mp_for_each<boost::mp11::mp_list<
      IfFood<ContexT>, Prog2<ContexT>, Prog3<ContexT>,
      Move<ContexT>, Left<ContexT>, Right<ContexT>>>(insertHelper);
      return factoryMap;
    }
    
    template <typename ContexT, typename Iter>
    evalable<ContexT> factory_imp(Iter &tokenIter) {
      static auto nodeCreateFunMap = makeFactoryMap<ContexT, Iter>();
      auto token = *tokenIter;
      if (!nodeCreateFunMap.count(token)) {
        throw std::runtime_error{
          std::string{"cant find factory function for token >>"} +
          std::string{token} + "<<"};
      }
      
      return nodeCreateFunMap[token](tokenIter);
    }
  }  // namespace detail
  
  template <typename ContexT, typename Iter>
  evalable<ContexT> factory(Iter tokenIter) {
    return detail::factory_imp<ContexT>(tokenIter);
  }
}
