// @HEADER
// ************************************************************************
//
//                           Intrepid2 Package
//                 Copyright (2007) Sandia Corporation
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
// Questions? Contact Kyungjoo Kim  (kyukim@sandia.gov), or
//                    Mauro Perego  (mperego@sandia.gov)
//
// ************************************************************************
// @HEADER


/** \file   Intrepid_OrientationToolsDef.hpp
    \brief  Definition file for the Intrepid2::OrientationTools class.
    \author Created by Kyungjoo Kim
*/
#ifndef __INTREPID2_ORIENTATIONTOOLS_DEF_COEFF_MATRIX_HPP__
#define __INTREPID2_ORIENTATIONTOOLS_DEF_COEFF_MATRIX_HPP__

// disable clang warnings
#if defined (__clang__) && !defined (__INTEL_COMPILER)
#pragma clang system_header
#endif

namespace Intrepid2 {

  namespace Impl {

    template<typename outputViewType,
             typename subcellBasisType,
             typename cellBasisType>
    inline
    void
    OrientationTools::
    getCoeffMatrix_HGRAD(outputViewType &output,
                         const subcellBasisType subcellBasis,
                         const cellBasisType cellBasis,
                         const ordinal_type subcellId,
                         const ordinal_type subcellOrt) {
      typedef typename outputViewType::execution_space space_type;
      typedef typename outputViewType::value_type value_type;

      // with shards, everything should be computed on host space
      typedef typename
        Kokkos::Impl::is_space<space_type>::host_mirror_space::execution_space host_space_type;

      // populate points on a subcell and map to subcell
      const shards::CellTopology cellTopo = cellBasis.getBaseCellTopology();
      const shards::CellTopology subcellTopo = subcellBasis.getBaseCellTopology();
#ifdef HAVE_INTREPID2_DEBUG
      INTREPID2_TEST_FOR_EXCEPTION( subcellTopo.getBaseKey() != shards::Line<>::key ||
                                    subcellTopo.getBaseKey() != shards::Quadrilateral<>::key ||
                                    subcellTopo.getBaseKey() != shards::Triangle<>::key,
                                    std::logic_error,
                                    ">>> ERROR (Intrepid::OrientationTools::getCoeffMatrix_HGRAD): " \
                                    "subcellBasis must have line, quad, or triangle topology.");
#endif

      // if node map has left handed system, orientation should be re-enumerated.
      ordinal_type ort = -1;
      switch (subcellTopo.getBaseKey()) {
      case shards::Line<>::key: {
        if (subcellOrt >= 0 && subcellOrt <  2)
          ort = subcellOrt;
        break;
      }
      case shards::Triangle<>::key: {
        if (subcellOrt >= 0 && subcellOrt <  6) {
          const ordinal_type leftHanded = cellTopo.getNodeMap(2, subcellId, 1) > cellTopo.getNodeMap(2, subcellId, 2);
          const ordinal_type leftOrt[] = { 0, 2, 1, 3, 5, 4 };
          ort = (leftHanded ? leftOrt[subcellOrt] : subcellOrt);
        }
        break;
      }
      case shards::Quadrilateral<>::key: {
        if (subcellOrt >= 0 && subcellOrt <  8) {
          const ordinal_type leftHanded = cellTopo.getNodeMap(2, subcellId, 1) > cellTopo.getNodeMap(2, subcellId, 3);
          const ordinal_type leftOrt[] = { 0, 3, 2, 1, 4, 7, 6, 5 };
          ort = (leftHanded ? leftOrt[subcellOrt] : subcellOrt);
        }
        break;
      }
      default: {
        INTREPID2_TEST_FOR_EXCEPTION( subcellTopo.getBaseKey() != shards::Line<>::key ||
                                      subcellTopo.getBaseKey() != shards::Quadrilateral<>::key ||
                                      subcellTopo.getBaseKey() != shards::Triangle<>::key,
                                      std::logic_error,
                                      ">>> ERROR (Intrepid::OrientationTools::getCoeffMatrix_HGRAD): " \
                                      "subcellBasis must have line, quad, or triangle topology.");
      }
      }
      INTREPID2_TEST_FOR_EXCEPTION( ort == -1,
                                    std::logic_error,
                                    ">>> ERROR (Intrepid::OrientationTools::getCoeffMatrix_HGRAD): " \
                                    "Orientation is not properly setup.");
      
      const ordinal_type cellDim = cellTopo.getDimension();
      const ordinal_type subcellDim = subcellTopo.getDimension();
#ifdef HAVE_INTREPID2_DEBUG
      INTREPID2_TEST_FOR_EXCEPTION( subcellDim != 1 ||
                                    subcellDim != 2,
                                    std::logic_error,
                                    ">>> ERROR (Intrepid::OrientationTools::getCoeffMatrix_HGRAD): " \
                                    "subcellBasis must have dimensions 1 or 2.");

      INTREPID2_TEST_FOR_EXCEPTION( subcellDim >= cellDim,
                                    std::logic_error,
                                    ">>> ERROR (Intrepid::OrientationTools::getCoeffMatrix_HGRAD): " \
                                    "cellDim must be greater than subcellDim.");
#endif

      const ordinal_type degree = cellBasis.getDegree();

      const ordinal_type numCellBasis = cellBasis.getCardinality();
      const ordinal_type numSubcellBasis = subcellBasis.getCardinality();

      const ordinal_type ordSubcell = cellBasis.getDofOrdinal(subcellDim, subcellId, 0);
#ifdef HAVE_INTREPID2_DEBUG
      INTREPID2_TEST_FOR_EXCEPTION( ordSubcell == -1,
                                    std::logic_error,
                                    ">>> ERROR (Intrepid::OrientationTools::getCoeffMatrix_HGRAD): " \
                                    "Invalid subcellId returns -1 ordSubcell.");
#endif

      const ordinal_type ndofSubcell = cellBasis.getDofTag(ordSubcell)(3);

#ifdef HAVE_INTREPID2_DEBUG
      INTREPID2_TEST_FOR_EXCEPTION( ndofSubcell != PointTools::getLatticeSize(subcellTopo, degree, 1),
                                    std::logic_error,
                                    ">>> ERROR (Intrepid::OrientationTools::getCoeffMatrix_HGRAD): " \
                                    "The number of DOFs does not match to the number of collocation points.");
#endif

      // reference points on a subcell
      Kokkos::DynRankView<value_type,host_space_type> refPtsSubcell("refPtsSubcell", ndofSubcell, subcellDim);
      PointTools::getLattice(refPtsSubcell,
                             subcellTopo,
                             degree,
                             1, // offset by 1 so the points are located inside
                             POINTTYPE_EQUISPACED);

      // modified points with orientation
      Kokkos::DynRankView<value_type,host_space_type> ortPtsSubcell("ortPtsSubcell", ndofSubcell, subcellDim);
      Impl::OrientationTools::mapToModifiedReference(ortPtsSubcell,
                                                     refPtsSubcell,
                                                     subcellTopo,
                                                     subcellOrt);

      // map to reference coordinates
      Kokkos::DynRankView<value_type,host_space_type> refPtsCell("refPtsCell", ndofSubcell, cellDim);
      CellTools<host_space_type>::mapToReferenceSubcell(refPtsCell,
                                                        refPtsSubcell,
                                                        subcellDim,
                                                        subcellId,
                                                        cellTopo);

      // evaluate values on the reference cell
      Kokkos::DynRankView<value_type,host_space_type> refValues("refValues", numCellBasis, ndofSubcell);
      cellBasis.getValues(refValues, refPtsCell, OPERATOR_VALUE);

      // evaluate values on the modified cell
      Kokkos::DynRankView<value_type,host_space_type> outValues("outValues", numSubcellBasis, ndofSubcell);
      subcellBasis.getValues(outValues, ortPtsSubcell, OPERATOR_VALUE);

      // construct collocation matrix; using lapack, it should be left layout
      Kokkos::View<value_type**,Kokkos::LayoutLeft,host_space_type>
        refMat("refMat", ndofSubcell, ndofSubcell),
        ortMat("ortMat", ndofSubcell, ndofSubcell),
        pivVec("pivVec", ndofSubcell, 1);

      for (ordinal_type i=0;i<ndofSubcell;++i) {
        const ordinal_type iref = cellBasis.getDofOrdinal(subcellDim, subcellId, i);
        const ordinal_type iout = subcellBasis.getDofOrdinal(subcellDim, 0, i);

        for (auto j=0;j<ndofSubcell;++j) {
          refMat(j,i) = refValues(iref,j);
          ortMat(j,i) = outValues(iout,j);
        }
      }

      // solve the system
      {
        Teuchos::LAPACK<ordinal_type,value_type> lapack;
        ordinal_type info = 0;

        lapack.GESV(ndofSubcell, ndofSubcell,
                    refMat.data(),
                    refMat.stride_1(),
                    (ordinal_type*)pivVec.data(),
                    ortMat.data(),
                    ortMat.stride_1(),
                    &info);

        if (info) {
          std::stringstream ss;
          ss << ">>> ERROR (Intrepid::OrientationTools::getCoeffMatrix_HGRAD): "
             << "LAPACK return with error code: "
             << info;
          INTREPID2_TEST_FOR_EXCEPTION( true, std::runtime_error, ss.str().c_str() );
        }
      }

      {
        // move the data to original device memory
        const Kokkos::pair<ordinal_type,ordinal_type> range(0, ndofSubcell);
        Kokkos::deep_copy(Kokkos::subview(output, range, range), ortMat);
      }
    }
  }

}
#endif
