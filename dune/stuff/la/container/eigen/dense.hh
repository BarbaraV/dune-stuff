// This file is part of the dune-stuff project:
//   https://github.com/wwu-numerik/dune-stuff/
// Copyright holders: Rene Milk, Felix Schindler
// License: BSD 2-Clause License (http://opensource.org/licenses/BSD-2-Clause)

#ifndef DUNE_STUFF_LA_CONTAINER_EIGEN_DENSE_HH
#define DUNE_STUFF_LA_CONTAINER_EIGEN_DENSE_HH

#include <memory>
#include <type_traits>
#include <vector>
#include <initializer_list>

#if HAVE_EIGEN
# include <dune/stuff/common/disable_warnings.hh>
#   include <Eigen/Core>
# include <dune/stuff/common/reenable_warnings.hh>
#endif // HAVE_EIGEN

#include <dune/stuff/common/disable_warnings.hh>
# include <dune/common/typetraits.hh>
# include <dune/common/densematrix.hh>
#include <dune/stuff/common/reenable_warnings.hh>

#include <dune/stuff/aliases.hh>
#include <dune/stuff/common/exceptions.hh>
#include <dune/stuff/common/crtp.hh>
#include <dune/stuff/common/float_cmp.hh>

#include "dune/stuff/la/container/interfaces.hh"
#include "dune/stuff/la/container/pattern.hh"

#include "base.hh"

namespace Dune {
namespace Stuff {
namespace LA {


// forwards
template< class ScalarImp >
class EigenDenseVector;

template< class T>
class EigenMappedDenseVector;

template< class ScalarImp >
class EigenDenseMatrix;


#if HAVE_EIGEN


namespace internal {


/**
 *  \brief Traits for EigenDenseVector.
 */
template< class ScalarImp = double >
class EigenDenseVectorTraits
{
public:
  typedef ScalarImp                       ScalarType;
  typedef EigenDenseVector< ScalarType >  derived_type;
  typedef typename ::Eigen::Matrix< ScalarType, ::Eigen::Dynamic, 1 > BackendType;
}; // class EigenDenseVectorTraits


/**
 *  \brief Traits for EigenMappedDenseVector.
 */
template< class ScalarImp = double >
class EigenMappedDenseVectorTraits
{
  typedef typename ::Eigen::Matrix< ScalarImp, ::Eigen::Dynamic, 1 > PlainBackendType;
public:
  typedef ScalarImp                             ScalarType;
  typedef EigenMappedDenseVector< ScalarType >  derived_type;
  typedef Eigen::Map< PlainBackendType >        BackendType;
}; // class EigenMappedDenseVectorTraits


/**
 *  \brief Traits for EigenDenseMatrix.
 */
template< class ScalarImp = double >
class EigenDenseMatrixTraits
{
public:
  typedef ScalarImp                       ScalarType;
  typedef EigenDenseMatrix< ScalarType >  derived_type;
  typedef typename ::Eigen::Matrix< ScalarType, ::Eigen::Dynamic, ::Eigen::Dynamic > BackendType;
}; // class EigenDenseMatrixTraits


} // namespace internal


/**
 *  \brief A dense vector implementation of VectorInterface using the eigen backend.
 */
template< class ScalarImp = double>
class EigenDenseVector
  : public EigenBaseVector< internal::EigenDenseVectorTraits< ScalarImp > >
  , public ProvidesDataAccess< internal::EigenDenseVectorTraits< ScalarImp > >
{
  typedef EigenDenseVector< ScalarImp >                                               ThisType;
  typedef VectorInterface< internal::EigenDenseVectorTraits< ScalarImp >, ScalarImp > VectorInterfaceType;
  typedef EigenBaseVector< internal::EigenDenseVectorTraits< ScalarImp > >            BaseType;
  static_assert(!std::is_same< DUNE_STUFF_SSIZE_T, int >::value,
                "You have to manually disable the constructor below which uses DUNE_STUFF_SSIZE_T!");
public:
  typedef internal::EigenDenseVectorTraits< ScalarImp > Traits;
  typedef typename Traits::ScalarType         ScalarType;
  typedef typename Traits::BackendType        BackendType;

  explicit EigenDenseVector(const size_t ss = 0, const ScalarType value = ScalarType(0))
  {
    this->backend_ = std::make_shared< BackendType >(ss);
    if (FloatCmp::eq(value, ScalarType(0)))
      this->backend_->setZero();
    else {
      this->backend_->setOnes();
      this->backend_->operator*=(value);
    }
  }

  /// This constructor is needed for the python bindings.
  explicit EigenDenseVector(const DUNE_STUFF_SSIZE_T ss, const ScalarType value = ScalarType(0))
  {
    this->backend_ = std::make_shared< BackendType >(VectorInterfaceType::assert_is_size_t_compatible_and_convert(ss));
    if (FloatCmp::eq(value, ScalarType(0)))
      this->backend_->setZero();
    else {
      this->backend_->setOnes();
      this->backend_->operator*=(value);
    }
  }

  explicit EigenDenseVector(const int ss, const ScalarType value = ScalarType(0))
  {
    this->backend_ = std::make_shared< BackendType >(VectorInterfaceType::assert_is_size_t_compatible_and_convert(ss));
    if (FloatCmp::eq(value, ScalarType(0)))
      this->backend_->setZero();
    else {
      this->backend_->setOnes();
      this->backend_->operator*=(value);
    }
  }

  explicit EigenDenseVector(const std::vector< ScalarType >& other)
  {
    this->backend_ = std::make_shared< BackendType >(other.size());
    for (size_t ii = 0; ii < other.size(); ++ii)
      this->backend_->operator[](ii) = other[ii];
  }

  explicit EigenDenseVector(const std::initializer_list< ScalarType >& other)
  {
    this->backend_ = std::make_shared< BackendType >(other.size());
    size_t ii = 0;
    for (auto element : other) {
      this->backend_->operator[](ii) = element;
      ++ii;
    }
  }

  explicit EigenDenseVector(const BackendType& other)
  {
    this->backend_ = std::make_shared< BackendType >(other);
  }

  /**
   *  \note Takes ownership of backend_ptr in the sense that you must not delete it afterwards!
   */
  explicit EigenDenseVector(BackendType* backend_ptr)
  {
    this->backend_ = std::shared_ptr< BackendType >(backend_ptr);
  }

  explicit EigenDenseVector(std::shared_ptr< BackendType > backend_ptr)
  {
    this->backend_ = backend_ptr;
  }

  /**
   *  \note Does a deep copy.
   */
  ThisType& operator=(const BackendType& other)
  {
    this->backend_ = std::make_shared< BackendType >(other);
    return *this;
  } // ... operator=(...)

  using VectorInterfaceType::add;
  using VectorInterfaceType::sub;
  using BaseType::backend;

private:
  inline void ensure_uniqueness() const
  {
    if (!this->backend_.unique())
      this->backend_ = std::make_shared< BackendType >(*(this->backend_));
  } // ... ensure_uniqueness(...)

  friend class EigenBaseVector< internal::EigenDenseVectorTraits< ScalarType > >;
}; // class EigenDenseVector
/**
 *  \brief  A dense metrix implementation of MatrixInterface using the eigen backend.
 */


/**
 *  \brief  A dense vector implementation of VectorInterface using the eigen backend which wrappes a raw array.
 */
template< class ScalarImp = double >
class EigenMappedDenseVector
  : public EigenBaseVector< internal::EigenMappedDenseVectorTraits< ScalarImp > >
  , public ProvidesBackend< internal::EigenMappedDenseVectorTraits< ScalarImp > >
{
  typedef EigenMappedDenseVector< ScalarImp >                                               ThisType;
  typedef VectorInterface< internal::EigenMappedDenseVectorTraits< ScalarImp >, ScalarImp > VectorInterfaceType;
  typedef EigenBaseVector< internal::EigenMappedDenseVectorTraits< ScalarImp > >  BaseType;
  static_assert(std::is_same< ScalarImp, double >::value, "Undefined behaviour for non-double data!");
  static_assert(!std::is_same< DUNE_STUFF_SSIZE_T, int >::value,
                "You have to manually disable the constructor below which uses DUNE_STUFF_SSIZE_T!");
public:
  typedef internal::EigenMappedDenseVectorTraits< ScalarImp > Traits;
  typedef typename Traits::BackendType              BackendType;
  typedef typename Traits::ScalarType               ScalarType;

  /**
   *  \brief  This is the constructor of interest which wrappes a raw array.
   */
  EigenMappedDenseVector(ScalarType* data, size_t data_size)
  {
    this->backend_ = std::make_shared< BackendType >(data, data_size);
  }

  /**
   *  \brief  This constructor allows to create an instance of this type just like any other vector.
   */
  explicit EigenMappedDenseVector(const size_t ss = 0, const ScalarType value = ScalarType(0))
  {
    this->backend_ = std::make_shared< BackendType >(new ScalarType[ss], ss);
    if (FloatCmp::eq(value, ScalarType(0)))
      this->backend_->setZero();
    else {
      this->backend_->setOnes();
      this->backend_->operator*=(value);
    }
  }

  /// This constructor is needed for the python bindings.
  explicit EigenMappedDenseVector(const DUNE_STUFF_SSIZE_T ss, const ScalarType value = ScalarType(0))
  {
    const size_t ss_size_t = VectorInterfaceType::assert_is_size_t_compatible_and_convert(ss);
    this->backend_ = std::make_shared< BackendType >(new ScalarType[ss_size_t], ss_size_t);
    if (FloatCmp::eq(value, ScalarType(0)))
      this->backend_->setZero();
    else {
      this->backend_->setOnes();
      this->backend_->operator*=(value);
    }
  }

  explicit EigenMappedDenseVector(const int ss, const ScalarType value = ScalarType(0))
  {
    const size_t ss_size_t = VectorInterfaceType::assert_is_size_t_compatible_and_convert(ss);
    this->backend_ = std::make_shared< BackendType >(new ScalarType[ss_size_t], ss_size_t);
    if (FloatCmp::eq(value, ScalarType(0)))
      this->backend_->setZero();
    else {
      this->backend_->setOnes();
      this->backend_->operator*=(value);
    }
  }

  explicit EigenMappedDenseVector(const std::vector< ScalarType >& other)
  {
    this->backend_ = std::make_shared< BackendType >(new ScalarType[other.size()], other.size());
    for (size_t ii = 0; ii < other.size(); ++ii)
      this->backend_->operator[](ii) = other[ii];
  }

  explicit EigenMappedDenseVector(const std::initializer_list< ScalarType >& other)
  {
    this->backend_ = std::make_shared< BackendType >(new ScalarType[other.size()], other.size());
    size_t ii = 0;
    for (auto element : other) {
      this->backend_->operator[](ii) = element;
      ++ii;
    }
  }

  /**
   *  \brief  This constructor does not do a deep copy.
   */
  EigenMappedDenseVector(const ThisType& other)
  {
    this->backend_ = other.backend_;
  }

  /**
   * \brief This constructor does a deep copy.
   */
  explicit EigenMappedDenseVector(const BackendType& other)
  {
    this->backend_ = std::make_shared< BackendType >(new ScalarType[other.size()], other.size());
    this->backend_->operator=(other);
  }

  /**
   *  \note Takes ownership of backend_ptr in the sense that you must not delete it afterwards!
   */
  explicit EigenMappedDenseVector(BackendType* backend_ptr)
  {
    this->backend_ = std::shared_ptr< BackendType >(backend_ptr);
  }

  explicit EigenMappedDenseVector(std::shared_ptr< BackendType > backend_ptr)
  {
    this->backend_ = backend_ptr;
  }

  /**
   * \brief does a deep copy;
   */
  ThisType& operator=(const BackendType& other)
  {
    this->backend_ = std::make_shared< BackendType >(new ScalarType[other.size()], other.size());
    this->backend_->operator=(other);
    return *this;
  }

  using VectorInterfaceType::add;
  using VectorInterfaceType::sub;
  using BaseType::backend;

private:
  inline void ensure_uniqueness() const
  {
    if (!this->backend_.unique()) {
      auto new_backend = std::make_shared< BackendType >(new ScalarType[this->backend_->size()],
          this->backend_->size());
      new_backend->operator=(*(this->backend_));
      this->backend_ = new_backend;
    }
  } // ... ensure_uniqueness(...)

  friend class EigenBaseVector< internal::EigenMappedDenseVectorTraits< ScalarType > >;
}; // class EigenMappedDenseVector


template< class ScalarImp = double >
class EigenDenseMatrix
  : public MatrixInterface< internal::EigenDenseMatrixTraits< ScalarImp >, ScalarImp >
  , public ProvidesBackend< internal::EigenDenseMatrixTraits< ScalarImp > >
  , public ProvidesDataAccess< internal::EigenDenseMatrixTraits< ScalarImp > >
{
  typedef EigenDenseMatrix< ScalarImp >                                                ThisType;
  typedef MatrixInterface< internal::EigenDenseMatrixTraits< ScalarImp >, ScalarImp >  MatrixInterfaceType;
  static_assert(!std::is_same< DUNE_STUFF_SSIZE_T, int >::value,
                "You have to manually disable the constructor below which uses DUNE_STUFF_SSIZE_T!");
public:
  typedef internal::EigenDenseMatrixTraits< ScalarImp > Traits;
  typedef typename Traits::BackendType                  BackendType;
  typedef typename Traits::ScalarType                   ScalarType;

  explicit EigenDenseMatrix(const size_t rr = 0, const size_t cc = 0, const ScalarType value = ScalarType(0))
    : backend_(new BackendType(rr, cc))
  {
    if (FloatCmp::eq(value, ScalarType(0)))
      backend_->setZero();
    else {
      backend_->setOnes();
      backend_->operator*=(value);
    }
  }

  /// This constructor is needed for the python bindings.
  explicit EigenDenseMatrix(const DUNE_STUFF_SSIZE_T rr, const DUNE_STUFF_SSIZE_T cc = 0, const ScalarType value = ScalarType(0))
    : backend_(new BackendType(MatrixInterfaceType::assert_is_size_t_compatible_and_convert(rr),
                               MatrixInterfaceType::assert_is_size_t_compatible_and_convert(cc)))
  {
    if (FloatCmp::eq(value, ScalarType(0)))
      backend_->setZero();
    else {
      backend_->setOnes();
      backend_->operator*=(value);
    }
  }

  explicit EigenDenseMatrix(const int rr, const int cc = 0, const ScalarType value = ScalarType(0))
    : backend_(new BackendType(MatrixInterfaceType::assert_is_size_t_compatible_and_convert(rr),
                               MatrixInterfaceType::assert_is_size_t_compatible_and_convert(cc)))
  {
    if (FloatCmp::eq(value, ScalarType(0)))
      backend_->setZero();
    else {
      backend_->setOnes();
      backend_->operator*=(value);
    }
  }

  /// This constructors ignores the given pattern and initializes the matrix with 0.
  EigenDenseMatrix(const size_t rr, const size_t cc, const SparsityPatternDefault& /*pattern*/)
    : backend_(new BackendType(MatrixInterfaceType::assert_is_size_t_compatible_and_convert(rr),
                               MatrixInterfaceType::assert_is_size_t_compatible_and_convert(cc)))
  {
    backend_->setZero();
  }

  EigenDenseMatrix(const ThisType& other) = default;

  explicit EigenDenseMatrix(const BackendType& other)
    : backend_(new BackendType(other))
  {}

  template< class M >
  EigenDenseMatrix(const MatrixInterface< M, ScalarType >& other)
    : backend_(new BackendType(other.rows(), other.cols()))
  {
    for (size_t ii = 0; ii < other.rows(); ++ii)
      for (size_t jj = 0; jj < other.cols(); ++jj)
        set_entry(ii, jj, other.get_entry(ii, jj));
  } // EigenDenseMatrix(...)

  template< class T >
  EigenDenseMatrix(const DenseMatrix< T >& other)
    : backend_(new BackendType(other.rows(), other.cols()))
  {
    for (size_t ii = 0; ii < other.rows(); ++ii)
      for (size_t jj = 0; jj < other.cols(); ++jj)
        set_entry(ii, jj, other[ii][jj]);
  } // EigenDenseMatrix(...)

  /**
   *  \note Takes ownership of backend_ptr in the sense that you must not delete it afterwards!
   */
  explicit EigenDenseMatrix(BackendType* backend_ptr)
    : backend_(backend_ptr)
  {}

  explicit EigenDenseMatrix(std::shared_ptr< BackendType > backend_ptr)
    : backend_(backend_ptr)
  {}

  ThisType& operator=(const ThisType& other)
  {
    backend_ = other.backend_;
    return *this;
  } // ... operator=(...)

  /**
   *  \note Does a deep copy.
   */
  ThisType& operator=(const BackendType& other)
  {
    backend_ = std::make_shared< BackendType >(other);
    return *this;
  } // ... operator=(...)

  /**
   * \defgroup backend ´´These methods are required by the ProvidesBackend interface.``
   * \{
   */

  BackendType& backend()
  {
    ensure_uniqueness();
    return *backend_;
  } // ... backend(...)

  const BackendType& backend() const
  {
    ensure_uniqueness();
    return *backend_;
  } // ... backend(...)
  /**
   * \}
   */

  /**
   * \defgroup data ´´These methods are required by the ProvidesDataAccess interface.``
   * \{
   */

  ScalarType* data()
  {
    return backend_->data();
  }

  /**
   * \}
   */

  /**
   * \defgroup container ´´These methods are required by ContainerInterface.``
   * \{
   */

  ThisType copy() const
  {
    return ThisType(*backend_);
  }

  void scal(const ScalarType& alpha)
  {
    backend() *= alpha;
  } // ... scal(...)

  void axpy(const ScalarType& alpha, const ThisType& xx)
  {
    if (!has_equal_shape(xx))
      DUNE_THROW(Exceptions::shapes_do_not_match,
                 "The shape of xx (" << xx.rows() << "x" << xx.cols()
                 << ") does not match the shape of this (" << rows() << "x" << cols() << ")!");
    const auto& xx_ref= *(xx.backend_);
    backend() += alpha * xx_ref;
  } // ... axpy(...)

  bool has_equal_shape(const ThisType& other) const
  {
    return (rows() == other.rows()) && (cols() == other.cols());
  }
  /**
   * \}
   */

  /**
   * \defgroup matrix_required ´´These methods are required by MatrixInterface.``
   * \{
   */

  inline size_t rows() const
  {
    return backend_->rows();
  }

  inline size_t cols() const
  {
    return backend_->cols();
  }

  template< class T1, class T2 >
  inline void mv(const EigenBaseVector< T1, ScalarType >& xx, EigenBaseVector< T2, ScalarType >& yy) const
  {
    yy.backend().transpose() = backend_->operator*(*xx.backend_);
  }

  void add_to_entry(const size_t ii, const size_t jj, const ScalarType& value)
  {
    assert(ii < rows());
    assert(jj < cols());
    backend()(ii, jj) += value;
  } // ... add_to_entry(...)

  void set_entry(const size_t ii, const size_t jj, const ScalarType& value)
  {
    assert(ii < rows());
    assert(jj < cols());
    backend()(ii, jj) = value;
  } // ... set_entry(...)

  ScalarType get_entry(const size_t ii, const size_t jj) const
  {
    assert(ii < rows());
    assert(jj < cols());
    return backend_->operator()(ii, jj);
  } // ... get_entry(...)

  void clear_row(const size_t ii)
  {
    if (ii >= rows())
      DUNE_THROW(Exceptions::index_out_of_range,
                 "Given ii (" << ii << ") is larger than the rows of this (" << rows() << ")!");
    ensure_uniqueness();
    for (size_t jj = 0; jj < cols(); ++jj)
      backend_->operator()(ii, jj) = ScalarType(0);
  } // ... clear_row(...)

  void clear_col(const size_t jj)
  {
    if (jj >= cols())
      DUNE_THROW(Exceptions::index_out_of_range,
                 "Given jj (" << jj << ") is larger than the cols of this (" << cols() << ")!");
    ensure_uniqueness();
    for (size_t ii = 0; ii < rows(); ++ii)
      backend_->operator()(ii, jj) = ScalarType(0);
  } // ... clear_col(...)

  void unit_row(const size_t ii)
  {
    if (ii >= rows())
      DUNE_THROW(Exceptions::index_out_of_range,
                 "Given ii (" << ii << ") is larger than the rows of this (" << rows() << ")!");
    ensure_uniqueness();
    for (size_t jj = 0; jj < cols(); ++jj)
      backend_->operator()(ii, jj) = ScalarType(0);
    backend_->operator()(ii, ii) = ScalarType(1);
  } // ... unit_row(...)

  void unit_col(const size_t jj)
  {
    if (jj >= cols())
      DUNE_THROW(Exceptions::index_out_of_range,
                 "Given jj (" << jj << ") is larger than the cols of this (" << cols() << ")!");
    ensure_uniqueness();
    for (size_t ii = 0; ii < rows(); ++ii)
      backend_->operator()(ii, jj) = ScalarType(0);
    backend_->operator()(jj, jj) = ScalarType(1);
  } // ... unit_col(...)

  /**
   * \}
   */

private:
  inline void ensure_uniqueness() const
  {
    if (!backend_.unique())
      backend_ = std::make_shared< BackendType >(*backend_);
  } // ... ensure_uniqueness(...)

  mutable std::shared_ptr< BackendType > backend_;
}; // class EigenDenseMatrix


#else // HAVE_EIGEN


template< class ScalarImp >
class EigenDenseVector{ static_assert(Dune::AlwaysFalse< ScalarImp >::value, "You are missing Eigen!"); };

template< class ScalarImp >
class EigenMappedDenseVector{ static_assert(Dune::AlwaysFalse< ScalarImp >::value, "You are missing Eigen!"); };

template< class ScalarImp >
class EigenDenseMatrix{ static_assert(Dune::AlwaysFalse< ScalarImp >::value, "You are missing Eigen!"); };


#endif // HAVE_EIGEN

} // namespace LA
} // namespace Stuff
} // namespace Dune

#endif // DUNE_STUFF_LA_CONTAINER_EIGEN_DENSE_HH