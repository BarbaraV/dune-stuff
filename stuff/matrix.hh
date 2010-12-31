#ifndef STUFF_MATRIX_HH
#define STUFF_MATRIX_HH

#include <dune/fem/operator/matrix/spmatrix.hh>

namespace Dune {
	//! obsolete,dysfunctional Matrixoperator
	template< class MatrixType >
	class SaneSparseRowMatrixOperator
	{

		const MatrixType& object_;

		public:
			typedef SaneSparseRowMatrixOperator< MatrixType >
				ThisType;
			SaneSparseRowMatrixOperator (	const MatrixType& object )
				: object_( object )
			{}

			template< class DomainFunction, class RangeFunction >
			void operator() ( const DomainFunction &arg, RangeFunction &dest ) const
			{
				object_.apply( arg, dest );
			}

			#ifdef USE_BFG_CG_SCHEME
			template <class VectorType, class IterationInfo>
			void multOEM(const VectorType * x, VectorType * ret, const IterationInfo& info ) const
			{
				// call multOEM of the matrix
				object_.multOEM(x,ret );
			}
			#endif

			template <class VectorType>
			void multOEM(const VectorType * x, VectorType * ret ) const
			{
				// call multOEM of the matrix
				object_.multOEM(x,ret );
			}

			const ThisType& systemMatrix() const {
				return *this;
			}
	};

	/** \brief get the diagonal of a fieldMatrix into a fieldvector
	  * \note While in principle this might do for SparseRowMatrix as well, don't do it! SparseRowMatrix has specialised functions for this (putting the diagonal into DiscreteFunctions tho
	  **/
	template <class FieldMatrixType>
	class MatrixDiagonal
			: public FieldVector<	typename FieldMatrixType::field_type,
									FieldMatrixType::rows >
	{
		public:
			MatrixDiagonal( const FieldMatrixType& matrix )
			{
				CompileTimeChecker< FieldMatrixType::rows == FieldMatrixType::cols > matrix_must_be_square;
				for ( size_t i = 0; i < FieldMatrixType::rows; i++)
					(*this)[i] = matrix[i][i];
			}

	};

	//! returns Sum of matrix' diagonal entries
	template <class FieldMatrixType>
	typename FieldMatrixType::field_type matrixTrace( const FieldMatrixType& matrix )
	{
		MatrixDiagonal<FieldMatrixType> diag( matrix );
		typename FieldMatrixType::field_type trace = typename FieldMatrixType::field_type(0);
		for ( size_t i = 0; i < FieldMatrixType::rows; i++)
			trace += diag[i];
		return trace;
	}

	//! produces a NxN Identity matrix compatible with parent type
	template < class MatrixType >
	class IdentityMatrix : public MatrixType {
	public:
		IdentityMatrix ( size_t N ):
			MatrixType( N, N, 1 )
		{
			for( size_t i = 1; i < N; ++i)
				MatrixType::set( i, i, 1.0 );
		}

		const MatrixType& matrix() const { return *this;}
	};

	//! produces a NxN Identity matrix object compatible with parent type
	template < class MatrixObjectType >
	class IdentityMatrixObject : public MatrixObjectType {
	public:
		IdentityMatrixObject(	const typename MatrixObjectType::DomainSpaceType& domain_space,
								const typename MatrixObjectType::RangeSpaceType& range_space):
			MatrixObjectType( domain_space, range_space )
		{
			MatrixObjectType::reserve();
//			dune_static_assert( true );
			for( int i = 0; i < MatrixObjectType::matrix().rows(); ++i)
				MatrixObjectType::matrix().set( i, i, 1.0 );
		}
	};

	//! adds the missing setDiag function to SparseRowMatrix
	template <class DiscFuncType, class MatrixType>
	void setMatrixDiag(MatrixType& matrix,  DiscFuncType &diag)
	{
	  typedef typename DiscFuncType::DofIteratorType DofIteratorType;

	  //! we assume that the dimension of the functionspace of f is the same as
	  //! the size of the matrix
	  DofIteratorType it = diag.dbegin();

	  for(int row=0; row < matrix.rows(); row++)
	  {
		  if (*it != 0.0 )
			matrix.set(row,row,*it);
	    ++it;
	  }
	  return;
	}

	//! return false if <pre>abs( a(row,col) - b(col,row) ) > tolerance<pre> for any col,row
	template < class MatrixType >
	bool areTransposed( const MatrixType& a, const MatrixType& b, const double tolerance = 1e-8 )
	{
		if ( a.rows() != b.cols() || b.rows() != a.cols() )
			return false;
		for(int row=0; row < a.rows(); ++row) {
			for(int col=0; col < a.cols(); ++col) {
				if ( std::fabs( a(row,col) - b(col,row) ) > tolerance )
					return false;
			}
		}
		return true;
	}
} //namespace Dune


namespace Stuff {
	//! extern matrix addition that ignore 0 entries
	template <class MatrixType>
	void addMatrix( MatrixType& dest, const MatrixType& arg, const double eps = 1e-14 )
	{
		for( int i = 0; i < arg.rows(); ++i )
			for( int j = 0; j < arg.cols(); ++j ) {
			const double value = arg(i,j);
			if	( std::fabs( value ) > eps )
				dest.add( i, j, value );
			}
	}

     /** @brief Write sparse matrix to given outgoing stream
     *
     *  @param[in]  matrix The matrix to be written
     *  @param[in] out    The outgoing stream
     */
    template<class SparseMatrixImpl, class Output>
    void writeSparseMatrix(const SparseMatrixImpl& matrix,
                           Output& out) {
      const unsigned int nRows = matrix.rows();
      const unsigned int nCols = matrix.cols();
      for (int i=0; i!=nRows; ++i) {
        for (int j=0; j!=nCols; ++j) {
          if (matrix.find(i,j)) {
            out << i << "," << j << ",";
            out.precision(12);
            out.setf(std::ios::scientific);
            out << matrix(i,j) << std::endl;
          }
        }
      }
      return;
    }

    /** @brief Read sparse matrix from given ingoing stream
     *
     *  @param[out] matrix The matrix to be read
     *  @param[in]  in     The ingoing stream
     */
    template<class SparseMatrixImpl, class Input>
    void readSparseMatrix(SparseMatrixImpl& matrix,
                          Input& in) {
      matrix.clear();
      std::string row;
      while (std::getline(in, row)) {
        size_t last_found = 0;
        size_t found = row.find_first_of(",");
        double temp;
        std::vector<double> entryLine;
        do
          {
            if(found == std::string::npos)
              found = row.size();
            std::string subs = row.substr(last_found, found-last_found);
            last_found = found+1;
            std::istringstream in( subs );
            if (entryLine.size()==2) {
              in.precision(12);
              in.setf(std::ios::scientific);
            }
            else {
              in.precision(10);
              in.setf(std::ios::fixed);
            }
            in >> temp;
            entryLine.push_back(temp);
            found = row.find_first_of(",", last_found);
          } while(last_found != row.size()+1);
        assert(entryLine.size()==3);
        matrix.add(entryLine[0],entryLine[1],entryLine[2]);
      }
    }  
} // namespace Stuff 

#endif // MATRIX_HH
