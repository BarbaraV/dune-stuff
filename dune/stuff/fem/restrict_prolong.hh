#ifndef DUNE_STUFF_RESTRICT_PROLONG_HH
#define DUNE_STUFF_RESTRICT_PROLONG_HH

#if HAVE_DUNE_FEM

#include <set>

#include <dune/fem/space/common/restrictprolonginterface.hh>

#include <dune/stuff/fem/namespace.hh>

namespace Dune {
namespace Stuff {
namespace Fem {

/** \brief an arbitrary size set of RestrictProlong operators of same type
   * This class is useful if you have more than a Pair of discrete functions that
   * need to be restricted/prolonged via some AdaptionManger.\n
   * All interface mandated functions are simply sequentially forwarded to each element in the set.
   * \note This set does not take ownership of added operators.
   */
template< class RestrictProlongOperatorPointerType >
class RestrictProlongOperatorSet
#if DUNE_FEM_IS_MULTISCALE_COMPATIBLE
  : public RestrictProlongInterface< RestrictProlongTraits<
#elif DUNE_FEM_IS_LOCALFUNCTIONS_COMPATIBLE
  : public Dune::Fem::RestrictProlongInterface< Dune::Fem::RestrictProlongTraits<
#else
  : public Dune::Fem::RestrictProlongInterface< Dune::Fem::RestrictProlongTraits<
#endif
                                       RestrictProlongOperatorSet< RestrictProlongOperatorPointerType >, double > >
{
public:
  typedef typename RestrictProlongOperatorPointerType::element_type::DomainFieldType
    DomainFieldType;

  RestrictProlongOperatorSet()
    : pair_set_( SetType() )
  {}

  void setFatherChildWeight(const DomainFieldType& val) const {
    for (auto el : pair_set_)
    {
      el->setFatherChildWeight(val);
    }
  } // setFatherChildWeight

  //! restrict data to father
  template< class EntityType>
  void restrictLocal(const EntityType& father,
                     const EntityType& son,
                     bool initialize) const {
    for (const auto& el : pair_set_)
    {
      el->restrictLocal(father, son, initialize);
    }
  } // restrictLocal

  //! prolong data to children
  template< class EntityType>
  void prolongLocal(const EntityType& father,
                    const EntityType& son,
                    bool initialize) const {
    for (const auto& el : pair_set_)
    {
      el->prolongLocal(father, son, initialize);
    }
  } // prolongLocal

  //! prolong data to children
  template< class CommunicatorImp >
  void addToList(CommunicatorImp& comm) {
    for (auto el : pair_set_)
    {
      el->addToList(comm);
    }
  } // addToList

  void add(RestrictProlongOperatorPointerType rpair) {
    pair_set_.insert(rpair);
  }

  bool remove(RestrictProlongOperatorPointerType rpair) {
    // we use this erase signature so that only one of possibly multiple instances gets remvod
    auto it = pair_set_.find(rpair);
    bool found = it != pair_set_.end();

    if (found)
      pair_set_.erase(it);
    return found;
  } // remove

protected:
  typedef std::set< RestrictProlongOperatorPointerType > SetType;
  SetType pair_set_;
};

} // namespace Fem
} // namespace Stuff
} // namespace Dune

#endif // HAVE_DUNE_FEM

#endif // DUNE_STUFF_RESTRICT_PROLONG_HH

/** Copyright (c) 2012, Rene Milk
   * All rights reserved.
   *
   * Redistribution and use in source and binary forms, with or without
   * modification, are permitted provided that the following conditions are met:
   *
   * 1. Redistributions of source code must retain the above copyright notice, this
   *    list of conditions and the following disclaimer.
   * 2. Redistributions in binary form must reproduce the above copyright notice,
   *    this list of conditions and the following disclaimer in the documentation
   *    and/or other materials provided with the distribution.
   *
   * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
   * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
   *
   * The views and conclusions contained in the software and documentation are those
   * of the authors and should not be interpreted as representing official policies,
   * either expressed or implied, of the FreeBSD Project.
   **/
