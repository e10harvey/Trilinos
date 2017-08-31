/*
//@HEADER
// ************************************************************************
//
//               KokkosKernels 0.9: Linear Algebra and Graph Kernels
//                 Copyright 2017 Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
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
// Questions? Contact Siva Rajamanickam (srajama@sandia.gov)
//
// ************************************************************************
//@HEADER
*/
#ifndef KOKKOSBLAS1_NRM2W_IMPL_HPP_
#define KOKKOSBLAS1_NRM2W_IMPL_HPP_

#include <KokkosKernels_config.h>
#include <Kokkos_Core.hpp>
#include <Kokkos_InnerProductSpaceTraits.hpp>
#include <KokkosBlas1_nrm2w_spec.hpp>

namespace KokkosBlas {
namespace Impl {

//
// nrm2w_squared
//

/// \brief 2-norm (squared) functor for single vectors.
///
/// \tparam RV 0-D output View
/// \tparam XV 1-D input View
/// \tparam SizeType Index type.  Use int (32 bits) if possible.
template<class RV, class XV, class SizeType = typename XV::size_type>
struct V_Nrm2w_Functor
{
  typedef typename XV::execution_space              execution_space;
  typedef SizeType                                        size_type;
  typedef typename XV::non_const_value_type             xvalue_type;
  typedef Kokkos::Details::InnerProductSpaceTraits<xvalue_type> IPT;
  typedef Kokkos::Details::ArithTraits<typename IPT::mag_type>   AT;
  typedef typename IPT::mag_type                         value_type;

  typename XV::const_type m_x, m_w;
  bool m_take_sqrt;

  V_Nrm2w_Functor (const XV& x, const XV& w, bool take_sqrt) :
    m_x (x), m_w (w), m_take_sqrt (take_sqrt)
  {
    static_assert (Kokkos::Impl::is_view<RV>::value,
                   "KokkosBlas::Impl::V_Nrm2w_Functor: "
                   "R is not a Kokkos::View.");
    static_assert (Kokkos::Impl::is_view<XV>::value,
                   "KokkosBlas::Impl::V_Nrm2w_Functor: "
                   "X is not a Kokkos::View.");
    static_assert (Kokkos::Impl::is_same<typename RV::value_type,
                   typename RV::non_const_value_type>::value,
                   "KokkosBlas::Impl::V_Nrm2w_Functor: R is const.  "
                   "It must be nonconst, because it is an output argument "
                   "(we have to be able to write to its entries).");
    static_assert (RV::rank == 0 && XV::rank == 1,
                   "KokkosBlas::Impl::V_Nrm2w_Functor: "
                   "RV must have rank 0 and XV must have rank 1.");
  }

  KOKKOS_INLINE_FUNCTION
  void operator() (const size_type& i, value_type& sum) const
  {
    const value_type tmp =  IPT::norm (m_x(i))/IPT::norm (m_w(i));
    sum += tmp * tmp;;
  }

  KOKKOS_INLINE_FUNCTION void init (value_type& update) const
  {
    update = AT::zero ();
  }

  KOKKOS_INLINE_FUNCTION void
  join (value_type& update,
        const value_type& source) const
  {
    update += source;
  }

  KOKKOS_INLINE_FUNCTION void
  join (volatile value_type& update,
        const volatile value_type& source) const
  {
    update += source;
  }

  KOKKOS_INLINE_FUNCTION void
  final (value_type& update) const {
    if(m_take_sqrt)
      update = Kokkos::Details::ArithTraits<typename RV::non_const_value_type>::sqrt(update);
  }
};

/// \brief Column-wise 2-norm functor for multivectors; works for
///   any layout, but best performance with LayoutRight.
///
/// \tparam RV 1-D output View
/// \tparam XMV 2-D input View
/// \tparam SizeType Index type.  Use int (32 bits) if possible.
template<class RV, class XMV, class SizeType = typename XMV::size_type>
struct MV_Nrm2w_Right_FunctorVector
{
  typedef typename XMV::execution_space             execution_space;
  typedef SizeType                                        size_type;
  typedef typename XMV::non_const_value_type            xvalue_type;
  typedef Kokkos::Details::InnerProductSpaceTraits<xvalue_type> IPT;
  typedef Kokkos::Details::ArithTraits<typename IPT::mag_type>   AT;
  typedef typename IPT::mag_type                       value_type[];

  size_type value_count;
  typename XMV::const_type m_x, m_w;
  bool m_take_sqrt;

  MV_Nrm2w_Right_FunctorVector (const XMV& x, const XMV& w, const bool& take_sqrt) :
    value_count (x.dimension_1 ()), m_x (x), m_w (x), m_take_sqrt(take_sqrt)
  {
    static_assert (Kokkos::Impl::is_view<RV>::value,
                   "KokkosBlas::Impl::MV_Nrm2w_Right_FunctorVector: "
                   "R is not a Kokkos::View.");
    static_assert (Kokkos::Impl::is_view<XMV>::value,
                   "KokkosBlas::Impl::MV_Nrm2w_Right_FunctorVector: "
                   "X is not a Kokkos::View.");
    static_assert (Kokkos::Impl::is_same<typename RV::value_type,
                   typename RV::non_const_value_type>::value,
                   "KokkosBlas::Impl::MV_Nrm2w_Right_FunctorVector: "
                   "R is const.  It must be nonconst, because it is an output "
                   "argument (we must be able to write to its entries).");
    static_assert (RV::rank == 1 && XMV::rank == 2,
                   "KokkosBlas::Impl::MV_Nrm2w_Right_FunctorVector: "
                   "RV must have rank 1 and XMV must have rank 2.");
  }

  KOKKOS_INLINE_FUNCTION void
  operator() (const size_type i, value_type sum) const
  {
    const size_type numVecs = value_count;
#ifdef KOKKOS_HAVE_PRAGMA_IVDEP
#pragma ivdep
#endif
#ifdef KOKKOS_HAVE_PRAGMA_VECTOR
#pragma vector always
#endif
    for (size_type j = 0; j < numVecs; ++j) {
      const typename IPT::mag_type tmp =  IPT::norm (m_x(i,j))/IPT::norm (m_w(i,j));
      sum[j] += tmp * tmp;;
    }
  }

  KOKKOS_INLINE_FUNCTION void
  init (value_type update) const
  {
    const size_type numVecs = value_count;
#ifdef KOKKOS_HAVE_PRAGMA_IVDEP
#pragma ivdep
#endif
#ifdef KOKKOS_HAVE_PRAGMA_VECTOR
#pragma vector always
#endif
    for (size_type j = 0; j < numVecs; ++j) {
      update[j] = AT::zero ();
    }
  }

  KOKKOS_INLINE_FUNCTION void
  join (volatile value_type update,
        const volatile value_type source) const
  {
    const size_type numVecs = value_count;
#ifdef KOKKOS_HAVE_PRAGMA_IVDEP
#pragma ivdep
#endif
#ifdef KOKKOS_HAVE_PRAGMA_VECTOR
#pragma vector always
#endif
    for (size_type j = 0; j < numVecs; ++j) {
      update[j] += source[j];
    }
  }

  KOKKOS_INLINE_FUNCTION void
  join (value_type update,
        const value_type source) const
  {
    const size_type numVecs = value_count;
#ifdef KOKKOS_HAVE_PRAGMA_IVDEP
#pragma ivdep
#endif
#ifdef KOKKOS_HAVE_PRAGMA_VECTOR
#pragma vector always
#endif
    for (size_type j = 0; j < numVecs; ++j) {
      update[j] += source[j];
    }
  }

  KOKKOS_INLINE_FUNCTION void
  final (value_type update) const {
    if(m_take_sqrt) {
      const size_type numVecs = value_count;
#ifdef KOKKOS_HAVE_PRAGMA_IVDEP
#pragma ivdep
#endif
#ifdef KOKKOS_HAVE_PRAGMA_VECTOR
#pragma vector always
#endif
      for (size_type j = 0; j < numVecs; ++j) {
        update[j] = Kokkos::Details::ArithTraits<typename RV::non_const_value_type>::sqrt(update[j]);
      }
    }
  }
};


/// \brief Compute the 2-norm (or its square) of the single vector (1-D
///   View) X, and store the result in the 0-D View r.
template<class RV, class XV, class SizeType>
void
V_Nrm2w_Invoke (const RV& r, const XV& X, const XV& W, const bool& take_sqrt)
{
  typedef typename XV::execution_space execution_space;
  const SizeType numRows = static_cast<SizeType> (X.dimension_0 ());
  Kokkos::RangePolicy<execution_space, SizeType> policy (0, numRows);

  typedef V_Nrm2w_Functor<RV, XV, SizeType> functor_type;
  functor_type op (X, W, take_sqrt);
  Kokkos::parallel_reduce (policy, op, r);
}


/// \brief Compute the 2-norms (or their square) of the columns of the
///   multivector (2-D View) X, and store result(s) in the 1-D View r.
template<class RV, class XMV, class SizeType>
void
MV_Nrm2w_Invoke (const RV& r, const XMV& X, const XMV& W, const bool& take_sqrt)
{
  typedef typename XMV::execution_space execution_space;
  const SizeType numRows = static_cast<SizeType> (X.dimension_0 ());
  Kokkos::RangePolicy<execution_space, SizeType> policy (0, numRows);

  // If the input multivector (2-D View) has only one column, invoke
  // the single-vector version of the kernel.
  if (X.dimension_1 () == 1) {
    auto r_0 = Kokkos::subview (r, 0);
    auto X_0 = Kokkos::subview (X, Kokkos::ALL (), 0);
    auto W_0 = Kokkos::subview (W, Kokkos::ALL (), 0);
    typedef decltype (r_0) RV0D;
    typedef decltype (X_0) XV1D;
    V_Nrm2w_Invoke<RV0D, XV1D, SizeType> (r_0, X_0, W_0, take_sqrt);
  }
  else {
    typedef MV_Nrm2w_Right_FunctorVector<RV, XMV, SizeType> functor_type;
    functor_type op (X, W, take_sqrt);
    Kokkos::parallel_reduce (policy, op, r);
  }
}

} // namespace Impl
} // namespace KokkosBlas

#endif // KOKKOSBLAS1_NRM2W_IMPL_HPP_