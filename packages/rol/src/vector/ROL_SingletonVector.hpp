
// @HEADER
// ************************************************************************
//
//               Rapid Optimization Library (ROL) Package
//                 Copyright (2014) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact lead developers:
//              Drew Kouri   (dpkouri@sandia.gov) and
//              Denis Ridzal (dridzal@sandia.gov)
//
// ************************************************************************
// @HEADER

#ifndef ROL_SINGLETONVECTOR_H
#define ROL_SINGLETONVECTOR_H

#include "ROL_Vector.hpp"

/** \class ROL::StdVector
    \brief Provides the ROL::Vector interface for scalar values, to be used,
           for example, with scalar constraints.
*/


namespace ROL {

template<class Real>
class SingletonVector : public Vector<Real> {

  template<class T> using RCP      = Teuchos::RCP<T>;

  using V  = Vector<Real>;

private:

  Real value_;

  Real getValue( const V& x ) { 
    return Teuchos::dyn_cast<const SingletonVector<Real>>(x).value(); 
  }

public:

  SingletonVector(const Real& value) : value_(value) {}

  const Real& value() const { return value; }
  Real& value() { return value; }

  void set( const V& x ) { 
    value_ = getValue(x);
  }

  void plus( const V& x ) {
    value_ += getValue(x);
  }

  void axpy( const Real alpha, const V& x ) {
     value_ *= alpha; value_ += getValue(x);
  }

  void scale( const Real alpha ) {
     value_ *= alpha;
  }

  Real dot( const V& x ) const {
    return value_*getValue(x);
  }
  
  RCP<V> clone() const {
    return Teuchos::rcp( new SingletonVector(0); );
  }
  
  RCP<V> basis() const {
    return Teuchos::rcp( new SingletonVector(1); );
  }

  int dimension() const { return 1; };

  void applyUnary( const Elementwise::UnaryFunction<Real> &f ) {
    f.apply(value_);
  }

  void applyBinary( const Elementwise::BinaryFunction<Real> &f, const V& x ) {
    f.apply(value_,getValue(x));
  }

  Real reduce( const Elementwise::ReductionOp<Real> &r ) const {
    return value_;
  }

  void print( std::ostream& os ) {
    os << value_ << std::endl;
  }

};


} // namespace ROL




#endif // ROL_SINGLETONVECTOR_H

