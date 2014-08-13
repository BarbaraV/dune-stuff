// This file is part of the dune-stuff project:
//   https://github.com/wwu-numerik/dune-stuff
// Copyright holders: Rene Milk, Felix Schindler
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)
//
// Contributors: Sven Kaulmann

#ifndef DUNE_STUFF_GRID_INTERSECTION_HH
#define DUNE_STUFF_GRID_INTERSECTION_HH

#include <dune/stuff/common/disable_warnings.hh>
# include <dune/common/fvector.hh>
#include <dune/stuff/common/reenable_warnings.hh>

#include <dune/stuff/common/disable_warnings.hh>
# include <dune/geometry/referenceelements.hh>
#include <dune/stuff/common/reenable_warnings.hh>

#if HAVE_DUNE_GRID
# include <dune/stuff/common/disable_warnings.hh>
#   include <dune/grid/common/gridview.hh>
# include <dune/stuff/common/reenable_warnings.hh>
#endif

#include <dune/stuff/common/string.hh>
#include <dune/stuff/aliases.hh>
#include <dune/stuff/common/type_utils.hh>
#include <dune/stuff/common/print.hh>

namespace Dune {
namespace Stuff {
namespace Grid {


#if HAVE_DUNE_GRID


template< class GridPartOrViewType >
class Intersection
{
  template< class GridViewType, bool is_view >
  struct Choose
  {
    typedef typename GridViewType::Intersection Type;
  };

  template< class GridPartType >
  struct Choose< GridPartType, false >
  {
    typedef typename GridPartType::IntersectionType Type;
  };

  static const bool this_is_a_grid_view =
      std::is_base_of< GridView< typename GridPartOrViewType::Traits >, GridPartOrViewType >::value;

public:
  typedef typename Choose< GridPartOrViewType, this_is_a_grid_view >::Type Type;
}; // class Intersection


#endif // HAVE_DUNE_GRID


/**
  \brief      prints some basic information about a Dune::Intersection, namely the number of its corners and the
              coordinates of those corners.
  \tparam     IntersectionType
              Dune::Intersection compatible
  \param[in]  intersection
              Dune::Intersection, whose information should be printed
  \param[out] stream
              std::ostream, into which the information is printed
  **/
template< class IntersectionType >
void printIntersection(const IntersectionType& intersection,
                       std::ostream& out = std::cout,
                       const std::string prefix = "")
{
  out << prefix << Common::Typename< IntersectionType >::value() << std::endl;
  const auto& geometry = intersection.geometry();
  for (int ii = 0; ii < geometry.corners(); ++ii)
    Common::print(geometry.corner(ii), "corner " + Common::toString(ii), out, prefix + "  ");
} // ... printIntersection(...)

/** Check whether a spatial point lies on an intersection.
*
* @param[in] intersection The intersection
* @param[in] globalPoint A Dune::FieldVector with the global coordinates of the point
* @return Returns true if the point lies on the intersection, false otherwise.
*/
template< class IntersectionType, class FieldType, int dim >
bool intersectionContains( const IntersectionType& intersection, const Dune::FieldVector< FieldType, dim >& globalPoint )
{
  // map global coordinates to local coordinates of the intersection
  const auto& intersectionGeometry = intersection.geometry();
  const auto& localPoint = intersectionGeometry.local(globalPoint);

  // get codim 1 reference element
#if DUNE_VERSION_NEWER(DUNE_GEOMETRY, 2, 3)
  const auto& refElement = ReferenceElements< FieldType, dim-1 >::general(intersectionGeometry.type());
#else
  const auto& refElement = GenericReferenceElements< FieldType, dim-1 >::general(intersectionGeometry.type());
#endif
  // check whether reference element contains the local coordinates
  return refElement.checkInside(localPoint);
} // end function intersectionContains

} // end namespace Grid
} // end of namespace Stuff
} // end namespace Dune

#endif // DUNE_STUFF_GRID_INTERSECTION_HH
