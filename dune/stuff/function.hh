#ifndef DUNE_STUFF_FUNCTION_HH
#define DUNE_STUFF_FUNCTION_HH

#include <dune/common/shared_ptr.hh>
#include <dune/common/parametertree.hh>
#include <dune/common/exceptions.hh>

#include <dune/stuff/common/color.hh>

namespace Dune {
namespace Stuff {
namespace Function {


// some forwards, includes are below
template< class D, int d, class R, int r >
class Interface;
template< class D, int d, class R, int r >
class Checkerboard;
template< class D, int d, class R, int r >
class Expression;
template< class D, int d, class R, int r >
class SeparableDefault;
template< class D, int d, class R, int r >
class SeparableCheckerboard;


template< class D, int d, class R, int r >
Dune::ParameterTree createSampleDescription(const std::string type)
{
  if (type == "function.checkerboard") {
    typedef Stuff::Function::Checkerboard< D, d, R, r > FunctionType;
    return FunctionType::createSampleDescription();
  } else if (type == "function.expression") {
    typedef Stuff::Function::Expression< D, d, R, r > FunctionType;
    return FunctionType::createSampleDescription();
  } else if (type == "function.parametric.separable.default") {
    typedef Stuff::Function::SeparableDefault< D, d, R, r > FunctionType;
    return FunctionType::createSampleDescription();
  } else if (type == "function.parametric.separable.checkerboard") {
    typedef Stuff::Function::SeparableCheckerboard< D, d, R, r > FunctionType;
    return FunctionType::createSampleDescription();
  } else
    DUNE_THROW(Dune::RangeError,
               "\n" << Dune::Stuff::Common::colorStringRed("ERROR:")
               << " unknown function '" << type << "' requested!");
} // ... create(...)


template< class D, int d, class R, int r >
Dune::shared_ptr< Interface< D, d, R, r > > create(const std::string type,
                                                   const Dune::ParameterTree description = Dune::ParameterTree())
{
  if (type == "function.checkerboard") {
    typedef Stuff::Function::Checkerboard< D, d, R, r > FunctionType;
    return Dune::make_shared< FunctionType >(FunctionType::createFromDescription(description));
  } else if (type == "function.expression") {
    typedef Stuff::Function::Expression< D, d, R, r > FunctionType;
    return Dune::make_shared< FunctionType >(FunctionType::createFromDescription(description));
  } else if (type == "function.parametric.separable.default") {
    typedef Stuff::Function::SeparableDefault< D, d, R, r > FunctionType;
    return Dune::make_shared< FunctionType >(FunctionType::createFromDescription(description));
  } else if (type == "function.parametric.separable.checkerboard") {
    typedef Stuff::Function::SeparableCheckerboard< D, d, R, r > FunctionType;
    return Dune::make_shared< FunctionType >(FunctionType::createFromDescription(description));
  } else
    DUNE_THROW(Dune::RangeError,
               "\n" << Dune::Stuff::Common::colorStringRed("ERROR:")
               << " unknown function '" << type << "' requested!");
} // ... create(...)


} // namespace Function
} // namespace Stuff
} // namespace Dune

#include "function/interface.hh"
#include "function/checkerboard.hh"
#include "function/expression.hh"
#include "function/parametric/separable/default.hh"
#include "function/parametric/separable/checkerboard.hh"

#endif // DUNE_STUFF_FUNCTION_HH