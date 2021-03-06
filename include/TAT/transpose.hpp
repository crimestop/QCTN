/**
 * \file transpose.hpp
 *
 * Copyright (C) 2019-2021 Hao Zhang<zh970205@mail.ustc.edu.cn>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once
#ifndef TAT_TRANSPOSE_HPP
#define TAT_TRANSPOSE_HPP

#include <tuple>

#include "basic_type.hpp"

#ifndef TAT_DOXYGEN_SHOULD_SKIP_THIS
#ifdef TAT_USE_MKL_TRANSPOSE
extern "C" {
void mkl_somatcopy_(const char*, const char*, const int*, const int*, const float*, const float*, const int*, float*, const int*);
void mkl_domatcopy_(const char*, const char*, const int*, const int*, const double*, const double*, const int*, double*, const int*);
void mkl_comatcopy_(
      const char*,
      const char*,
      const int*,
      const int*,
      const std::complex<float>*,
      const std::complex<float>*,
      const int*,
      std::complex<float>*,
      const int*);
void mkl_zomatcopy_(
      const char*,
      const char*,
      const int*,
      const int*,
      const std::complex<double>*,
      const std::complex<double>*,
      const int*,
      const std::complex<double>*,
      const int*);
}
#endif
#endif

namespace TAT {
#ifndef TAT_DOXYGEN_SHOULD_SKIP_THIS
   template<typename ScalarType>
   void mkl_transpose(
         int dimension_of_M,
         int dimension_of_N,
         const ScalarType* data_source,
         ScalarType* data_destination,
         int leading_source,
         int leading_destination,
         ScalarType alpha);

#ifdef TAT_USE_MKL_TRANSPOSE
   template<>
   void mkl_transpose<float>(
         const int dimension_of_M,
         const int dimension_of_N,
         const float* const data_source,
         float* const data_destination,
         const int leading_source,
         const int leading_destination,
         const float alpha) {
      mkl_somatcopy_("R", "T", &dimension_of_M, &dimension_of_N, &alpha, data_source, &leading_source, data_destination, &leading_destination);
   }
   template<>
   void mkl_transpose<double>(
         const int dimension_of_M,
         const int dimension_of_N,
         const double* const data_source,
         double* const data_destination,
         const int leading_source,
         const int leading_destination,
         const double alpha) {
      mkl_domatcopy_("R", "T", &dimension_of_M, &dimension_of_N, &alpha, data_source, &leading_source, data_destination, &leading_destination);
   }
   template<>
   void mkl_transpose<std::complex<float>>(
         const int dimension_of_M,
         const int dimension_of_N,
         const std::complex<float>* const data_source,
         std::complex<float>* const data_destination,
         const int leading_source,
         const int leading_destination,
         const std::complex<float> alpha) {
      mkl_comatcopy_("R", "T", &dimension_of_M, &dimension_of_N, &alpha, data_source, &leading_source, data_destination, &leading_destination);
   }
   template<>
   void mkl_transpose<std::complex<double>>(
         const int dimension_of_M,
         const int dimension_of_N,
         const std::complex<double>* const data_source,
         std::complex<double>* const data_destination,
         const int leading_source,
         const int leading_destination,
         const std::complex<double> alpha) {
      mkl_zomatcopy_("R", "T", &dimension_of_M, &dimension_of_N, &alpha, data_source, &leading_source, data_destination, &leading_destination);
   }
#endif

   // ??????????????????????????????????????????????????????????????????numpy?????????????????????????????????dimension?????????leading?????????????????????????????????
   // numpy??????destination???leading??????????????? simple_transpose??????????????????tensor_transpose_kernel???
   // ????????????????????????inturn_transpose???src dst?????????, ????????????cache????????????, ??????????????????cache??????split????????????????????????????????????????????????
   template<typename ScalarType, bool parity>
   void tensor_transpose_kernel(
         const ScalarType* const __restrict data_source,
         ScalarType* const __restrict data_destination,
         const Size* const __restrict dimension,
         const Size* const __restrict leading_source,
         const Size* const __restrict leading_destination,
         const Rank rank) {
      auto timer_guard = transpose_kernel_core_guard();

      // ??????????????????mkl???transpose???????????????
#if 0
#ifdef TAT_USE_MKL_TRANSPOSE
      if (rank == 2) {
         if (leading_source[1] == 1 && leading_destination[0] == 1) {
            mkl_transpose<ScalarType>(
                  dimension[0], dimension[1], data_source, data_destination, leading_source[0], leading_destination[1], parity ? -1 : 1);
            return;
         } else if (leading_source[0] == 1 && leading_destination[1] == 1) {
            mkl_transpose<ScalarType>(
                  dimension[1], dimension[0], data_source, data_destination, leading_source[1], leading_destination[0], parity ? -1 : 1);
            return;
         }
      }
#endif
#endif

      const ScalarType* current_source = data_source;
      ScalarType* current_destination = data_destination;
      pmr::vector<Size> index_list(rank, 0);
      while (true) {
         if constexpr (parity) {
            *current_destination = -*current_source;
         } else {
            *current_destination = *current_source;
         }

         Rank active_position = rank - 1;

         index_list[active_position]++;
         current_source += leading_source[active_position];
         current_destination += leading_destination[active_position];

         while (index_list[active_position] == dimension[active_position]) {
            index_list[active_position] = 0;
            current_source -= dimension[active_position] * leading_source[active_position];
            current_destination -= dimension[active_position] * leading_destination[active_position];

            if (active_position == 0) {
               return;
            }
            active_position--;

            index_list[active_position]++;
            current_source += leading_source[active_position];
            current_destination += leading_destination[active_position];
         }
      }
   }

   // TODO ??????????????????????
   // TODO: l3?????????, ???????????????l2???l1?????????, ???????????????
   // ????????????????????????????????????????????????
   //
   // TODO ???????????????????????????????????????????????????
   // O...O...O.O..XXX
   //  X..X..X.....OOOO
   //              ^
   // ?????????????????????????????????????????????????????????????????????????????????????????????cut??????????????????
   //
   // TODO block transpose
   // ????????????????????????block????????????????????????????????????block????????????????????????????????????????????????????????????????????????
   // ??????cache???split???
   // ?????????iter dim
   // ???????????????????????????????????????

   template<typename ScalarType, bool parity>
   void simple_transpose(
         const ScalarType* const __restrict data_source,
         ScalarType* const __restrict data_destination,
         const pmr::vector<Rank>& plan_source_to_destination,
         const pmr::vector<Rank>& plan_destination_to_source,
         const pmr::vector<Size>& dimensions_source,
         const pmr::vector<Size>& dimensions_destination,
         const pmr::vector<Size>& leadings_source,
         const pmr::vector<Size>& leadings_destination,
         const Rank rank) {
      auto leadings_source_by_destination = pmr::vector<Size>();
      leadings_source_by_destination.reserve(rank);
      for (auto i = 0; i < rank; i++) {
         auto j = plan_destination_to_source[i];
         leadings_source_by_destination.push_back(leadings_source[j]);
      }

      tensor_transpose_kernel<ScalarType, parity>(
            data_source, data_destination, dimensions_destination.data(), leadings_source_by_destination.data(), leadings_destination.data(), rank);
   }

   template<typename ScalarType>
   void do_transpose(
         const ScalarType* data_source,
         ScalarType* data_destination,
         const pmr::vector<Rank>& plan_source_to_destination,
         const pmr::vector<Rank>& plan_destination_to_source,
         const pmr::vector<Size>& dimensions_source,
         const pmr::vector<Size>& dimensions_destination,
         const pmr::vector<Size>& leadings_source,
         const pmr::vector<Size>& leadings_destination,
         Rank rank,
         Size total_size,
         bool parity) {
      auto timer_guard = transpose_kernel_guard();

      if (total_size == 0) {
         return;
      }
      if (total_size == 1) {
         if (parity) {
            *data_destination = -*data_source;
         } else {
            *data_destination = *data_source;
         }
         return;
      }
      // rank != 0, dimension != 0

      if (parity) {
         simple_transpose<ScalarType, true>(
               data_source,
               data_destination,
               plan_source_to_destination,
               plan_destination_to_source,
               dimensions_source,
               dimensions_destination,
               leadings_source,
               leadings_destination,
               rank);
      } else {
         simple_transpose<ScalarType, false>(
               data_source,
               data_destination,
               plan_source_to_destination,
               plan_destination_to_source,
               dimensions_source,
               dimensions_destination,
               leadings_source,
               leadings_destination,
               rank);
      }
   }

   template<typename ScalarType>
   void matrix_transpose(Size m, Size n, const ScalarType* const source, ScalarType* const destination) {
      auto dimension = pmr::vector<Size>{m, n};
      auto leading_source = pmr::vector<Size>{n, 1};
      auto leading_destination = pmr::vector<Size>{1, m};
      tensor_transpose_kernel<ScalarType, false>(source, destination, dimension.data(), leading_source.data(), leading_destination.data(), 2);
   }
#endif
} // namespace TAT
#endif
