/**
 * \file edge_operator.hpp
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
#ifndef TAT_EDGE_OPERATOR_HPP
#define TAT_EDGE_OPERATOR_HPP

#include "pmr_resource.hpp"
#include "tensor.hpp"
#include "timer.hpp"
#include "transpose.hpp"

namespace TAT {
   template<typename Vector1, typename Vector2>
   bool is_same_vector(const Vector1& vector1, const Vector2& vector2) {
      auto size1 = vector1.size();
      auto size2 = vector2.size();
      if (size1 != size2) {
         return false;
      }
      for (auto i = 1; i < size1; i++) {
         if (vector1[i] != vector2[i]) {
            return false;
         }
      }
      return true;
   }

   template<typename ScalarType, typename Symmetry, typename Name>
   template<
         typename MapNameName,
         typename MapNameVectorNameAndEdge,
         typename SetName1,
         typename MapNameVectorName,
         typename VectorName,
         typename SetName2,
         typename MapNameMapSymmetrySize>
   [[nodiscard]] Tensor<ScalarType, Symmetry, Name> Tensor<ScalarType, Symmetry, Name>::edge_operator(
         const MapNameName& rename_map,
         const MapNameVectorNameAndEdge& split_map,
         const SetName1& reversed_name,
         const MapNameVectorName& merge_map,
         const VectorName& new_names,
         const bool apply_parity,
         const std::array<SetName2, 4>& parity_exclude_name,
         const MapNameMapSymmetrySize& edge_and_symmetries_to_cut_before_all) const {
      auto timer_guard = transpose_guard();
      auto pmr_guard = scope_resource<>();
      // step 1: rename and cut
      // step 2: split
      // step 3: reverse
      // step 4: transpose
      // step 5: reverse before merge
      // step 6: merge
      // ????????????????????????????????????, ???????????????????????????merge????????????name?????????????????????
      // ???????????????, ?????????????????????transpose??????

      // parity_exclude_name?????????????????????split, reverse, reverse_for_merge, merge
      // name ??????rename???????????????split???split?????? merge???merge???

      // is_fermi
      constexpr auto is_fermi = is_fermi_symmetry_v<Symmetry>;

      // ???????????????edge??????????????????
      //
      //                     rank    name    edge
      // before_split        O       O       D
      // after_split         ???       O       O
      // before_transpose    ???       ???       O
      // at_transpose        O       -       -
      // after_transpose     ???       ???       O
      // before_merge        ???       O       O
      // after_merge         O       O       D
      //
      //                     flag                offset
      // split               rank[after_split]   (symmetry[]->(symmetry, offset))[before_split]
      // merge               rank[after_split]   (symmetry[]->(symmetry, offset))[after_merge]
      //
      // reversed            flag
      // before_transpose    bool[at_transpose]
      // after_transpose     bool[at_transpose]

      // 1. ??????rank, name, edge??????????????????
      //
      // 1.1 ???????????? status 0???status 1, ???????????????transpose?????????????????????
      //
      // 1.1.1 status 0
      // status 0 origin
      // rank_0 and name_0
      // create edge 0

      // 1.1.2 status 1
      // status 1 rename
      // rank_1 and edge_1
      const Rank rank_before_split = names.size();
      // create name_1
      auto real_name_before_split = decltype(names)(); // length = rank_before_split
      if (!rename_map.empty()) {
         real_name_before_split.reserve(rank_before_split);
         for (Rank i = 0; i < rank_before_split; i++) {
            auto name = names[i];
            if (auto position = rename_map.find(name); position != rename_map.end()) {
               real_name_before_split.push_back(position->second);
            } else {
               real_name_before_split.push_back(name);
            }
         }
      }
      const auto& name_before_split = !rename_map.empty() ? real_name_before_split : names;

      // 1.2 ???????????????????????????????????? -> rename_edge
      // check no change
      if (is_same_vector(name_before_split, new_names) && split_map.empty() && reversed_name.empty() && merge_map.empty() &&
          edge_and_symmetries_to_cut_before_all.empty()) {
         // share the core
         auto result = Tensor<ScalarType, Symmetry, Name>();
         result.names = {new_names.begin(), new_names.end()};
         result.name_to_index = construct_name_to_index<decltype(name_to_index)>(result.names);
         result.core = core; // ?????????rename edge???????????????
         // check_valid_name(result.names, result.core->edges.size());
         return result;
      }

      auto real_edge_before_split = decltype(core->edges)();
      if (!edge_and_symmetries_to_cut_before_all.empty()) {
         real_edge_before_split.reserve(rank_before_split);
         for (auto i = 0; i < rank_before_split; i++) {
            // ?????????cut, ???????????????rename????????????
            if (auto found = edge_and_symmetries_to_cut_before_all.find(names[i]); found != edge_and_symmetries_to_cut_before_all.end()) {
               const auto& symmetry_to_cut_dimension = found->second;
               auto& this_edge = real_edge_before_split.emplace_back();
               if constexpr (is_fermi) {
                  this_edge.arrow = core->edges[i].arrow;
               }
               for (const auto& [symmetry, dimension] : core->edges[i].map) {
                  if (auto cut_iterator = symmetry_to_cut_dimension.find(symmetry); cut_iterator != symmetry_to_cut_dimension.end()) {
                     if (auto new_dimension = cut_iterator->second; new_dimension != 0) {
                        // auto new_dimension = cut_iterator->second;
                        this_edge.map[symmetry] = new_dimension < dimension ? new_dimension : dimension;
                     }
                     // ??????new_dimension=0????????????????????????symmetry
                     // ???????????????leadings, ????????????????????????offset??????????????????, ?????????core->edges
                  } else {
                     this_edge.map[symmetry] = dimension;
                  }
               }
            } else {
               real_edge_before_split.push_back(core->edges[i]);
            }
         }
      }
      const auto& edge_before_split = !edge_and_symmetries_to_cut_before_all.empty() ? real_edge_before_split : core->edges;

      // 1.3 ????????????????????????transpose????????? rank, name, edge
      // status 2 split
      // create name_2 and edge_2 and split_flag
      auto split_flag = pmr::vector<Rank>();
      auto split_offset = pmr::vector<pmr::map<pmr::vector<Symmetry>, std::tuple<Symmetry, Size>>>();
      auto real_name_after_split = decltype(names)();
      auto edge_after_split = pmr::vector<EdgePointer<Symmetry>>();
      if (!split_map.empty()) {
         // ??????????????????rank, ??????reserve????????????, ???????????????, ?????????????????????, ??????????????????new????????????
         split_flag.reserve(rank_before_split);            // rank_at_transpose
         split_offset.reserve(rank_before_split);          // ?????????
         real_name_after_split.reserve(rank_before_split); // rank_at_transpose
         edge_after_split.reserve(rank_before_split);      // rank_at_transpose
         for (Rank position_before_split = 0, total_split_index = 0; position_before_split < rank_before_split; position_before_split++) {
            if (auto position = split_map.find(name_before_split[position_before_split]); position != split_map.end()) {
               const auto& this_split_begin_position_in_edge_after_split = edge_after_split.size();
               // split??????edge????????????????????????, ?????????????????????
               for (const auto& [split_name, split_edge] : position->second) {
                  real_name_after_split.push_back(split_name);
                  if constexpr (is_fermi) {
                     edge_after_split.push_back({edge_before_split[position_before_split].arrow, split_edge.map});
                  } else {
                     edge_after_split.push_back({split_edge.map});
                  }
                  split_flag.push_back(total_split_index);
               }
               const auto edge_list_after_split = edge_after_split.data() + this_split_begin_position_in_edge_after_split;
               const auto split_rank = edge_after_split.size() - this_split_begin_position_in_edge_after_split;
               // loop between begin and end, get a map push_Back into split_offset
               // this map is sym -> [sym] -> offset
               auto& this_offset = split_offset.emplace_back();
               auto offset_bank = pmr::map<Symmetry, Size>();
               for (const auto& [sym, dim] : edge_before_split[position_before_split].map) {
                  // ?????????symmetry??????, edge_before_split???core->edge??????
                  offset_bank[sym] = 0;
               }
               auto accumulated_symmetries = pmr::vector<Symmetry>(split_rank);
               auto accumulated_dimensions = pmr::vector<Size>(split_rank);
               auto current_symmetries = pmr::vector<Symmetry>(split_rank);
               loop_edge(
                     edge_list_after_split,
                     split_rank,
                     [&this_offset]() {
                        this_offset[pmr::vector<Symmetry>{}] = {Symmetry(), 0};
                     },
                     []() {},
                     [&](const auto& symmetry_iterator_list, Rank minimum_changed) {
                        for (auto i = minimum_changed; i < split_rank; i++) {
                           const auto& symmetry_iterator = symmetry_iterator_list[i];
                           accumulated_symmetries[i] = symmetry_iterator->first + (i ? accumulated_symmetries[i - 1] : Symmetry());
                           accumulated_dimensions[i] = symmetry_iterator->second * (i ? accumulated_dimensions[i - 1] : 1);
                           // do not check dim=0, because in constructor, it didn't check
                           current_symmetries[i] = symmetry_iterator->first;
                        }
                        auto target_symmetry = accumulated_symmetries.back();
                        auto target_dimension = accumulated_dimensions.back();
                        // ????????????????????????, ??????????????????????????????target_symmetry????????????block
                        // split????????????symmetry???????????????????????????split???symmetry
                        if (auto found = offset_bank.find(target_symmetry); found != offset_bank.end()) {
                           this_offset[current_symmetries] = {target_symmetry, found->second};
                           found->second += target_dimension;
                        }
                        return split_rank;
                     });
               total_split_index++;
            } else {
               real_name_after_split.push_back(name_before_split[position_before_split]);
               if constexpr (is_fermi) {
                  edge_after_split.push_back({edge_before_split[position_before_split].arrow, edge_before_split[position_before_split].map});
               } else {
                  edge_after_split.push_back({edge_before_split[position_before_split].map});
               }
               split_flag.push_back(total_split_index++);
               split_offset.emplace_back();
               // auto& this_offset = split_offset.emplace_back();
               // ???????????????????????????????????????split????????????this_offset
               // for (const auto& [symmetry, dimension] : edge_before_split[position_before_split].map) {
               //   this_offset[{symmetry}] = {symmetry, 0};
               // }
            }
         }
      } else {
         edge_after_split.reserve(rank_before_split);
         split_flag.reserve(rank_before_split);
         split_offset.reserve(rank_before_split);
         for (auto i = 0; i < rank_before_split; i++) {
            const auto& edge = edge_before_split[i];
            if constexpr (is_fermi) {
               edge_after_split.push_back({edge.arrow, edge.map});
            } else {
               edge_after_split.push_back({edge.map});
            }
            split_flag.push_back(i);
            // ???????????????????????????????????????split????????????this_offset, ??????????????????split?????????????????????emplace_back
            // auto& this_offset = split_offset.emplace_back();
            // for (const auto& [symmetry, dimension] : edge.map) {
            //    this_offset[{symmetry}] = {symmetry, 0};
            // }
         }
      }
      const auto& name_after_split = !split_map.empty() ? real_name_after_split : name_before_split;
      // rank_2
      const Rank rank_at_transpose = name_after_split.size();

      // status 3 reverse
      // rank_3 and name_3
      // create reversed_flag_src and edge_3
      auto reversed_before_transpose_flag = pmr::vector<bool>(); // length = rank_at_transpose
      auto fermi_edge_before_transpose = pmr::vector<EdgePointer<Symmetry>>();
      if constexpr (is_fermi) {
         if (!reversed_name.empty()) {
            reversed_before_transpose_flag.reserve(rank_at_transpose);
            fermi_edge_before_transpose.reserve(rank_at_transpose);
            for (auto i = 0; i < rank_at_transpose; i++) {
               fermi_edge_before_transpose.push_back(edge_after_split[i]);
               if (reversed_name.find(name_after_split[i]) != reversed_name.end()) {
                  fermi_edge_before_transpose.back().arrow ^= true;
                  reversed_before_transpose_flag.push_back(true);
               } else {
                  reversed_before_transpose_flag.push_back(false);
               }
            }
         } else {
            reversed_before_transpose_flag = pmr::vector<bool>(rank_at_transpose, false);
         }
      }
      const auto& edge_before_transpose = is_fermi && !reversed_name.empty() ? fermi_edge_before_transpose : edge_after_split;

      // create res names
      auto result = Tensor<ScalarType, Symmetry, Name>();
      result.names = {new_names.begin(), new_names.end()};
      result.name_to_index = construct_name_to_index<decltype(name_to_index)>(result.names);

      // 1.4 transpose?????????rank, name

      // status 4 transpose and status 6 merge and status 5 reverse before merge
      // ????????????name, ???????????????edge???data
      // name and rank
      // name_6 and rank_6
      const auto& name_after_merge = result.names;
      const Rank rank_after_merge = name_after_merge.size();
      // create merge_flag and name_5
      auto merge_flag = pmr::vector<Rank>();
      auto real_name_before_merge = decltype(names)();
      if (!merge_map.empty()) {
         merge_flag.reserve(rank_at_transpose);
         real_name_before_merge.reserve(rank_at_transpose);
         for (Rank position_after_merge = 0, total_merge_index = 0; position_after_merge < rank_after_merge; position_after_merge++) {
            const auto& merged_name = name_after_merge[position_after_merge];
            if (auto position = merge_map.find(merged_name); position != merge_map.end()) {
               for (const auto& merging_names : position->second) {
                  real_name_before_merge.push_back(merging_names);
                  merge_flag.push_back(total_merge_index);
               }
               total_merge_index++;
            } else {
               real_name_before_merge.push_back(merged_name);
               merge_flag.push_back(total_merge_index++);
            }
         }
      } else {
         merge_flag.reserve(rank_after_merge);
         for (auto i = 0; i < rank_after_merge; i++) {
            merge_flag.push_back(i);
         }
      }
      const auto& name_before_merge = !merge_map.empty() ? real_name_before_merge : name_after_merge;
      // rank_4 and name_5 and rank_5
      // name build by merge may contain some edge not exist
      if (rank_at_transpose != name_before_merge.size()) {
         TAT_error("Tensor to transpose with Different Rank");
      }

      // 1.5 ????????????
      // to be easy, create name_to_index for name_3
      auto name_to_index_after_split = construct_name_to_index<pmr::map<Name, Rank>>(name_after_split);
      // create plan of two way
      auto plan_source_to_destination = pmr::vector<Rank>(rank_at_transpose);
      auto plan_destination_to_source = pmr::vector<Rank>(rank_at_transpose);

      // edge
      // create edge_4
      auto edge_after_transpose = pmr::vector<EdgePointer<Symmetry>>();
      edge_after_transpose.reserve(rank_at_transpose);
      for (auto i = 0; i < rank_at_transpose; i++) {
         if (auto found = name_to_index_after_split.find(name_before_merge[i]); found != name_to_index_after_split.end()) {
            plan_destination_to_source[i] = found->second;
         } else {
            TAT_error("Tensor to transpose with incompatible name list");
         }
         plan_source_to_destination[plan_destination_to_source[i]] = i;
         edge_after_transpose.push_back(edge_before_transpose[plan_destination_to_source[i]]);
      }
      // 1.6 ??????????????????edge
      // reverse ??? merge ????????????????????????, ??????fermi, ??????reverse???edge????????????edge???????????????, ??????????????????????????????

      // the following code is about merge
      // dealing with edge_5 and res_edge and reversed_flag_dst

      // prepare edge_5
      // if no merge, edge_5 is reference of edge_4, else is copy of edge_4
      auto fermi_edge_before_merge = pmr::vector<EdgePointer<Symmetry>>();
      if constexpr (is_fermi) {
         if (!merge_map.empty()) {
            fermi_edge_before_merge.reserve(rank_at_transpose);
            for (const auto& edge : edge_after_transpose) {
               fermi_edge_before_merge.push_back(edge);
            }
         }
      }
      auto& edge_before_merge = is_fermi && !merge_map.empty() ? fermi_edge_before_merge : edge_after_transpose;
      // fermi????????????????????????merge???

      // prepare reversed_flag_dst
      auto reversed_after_transpose_flag = pmr::vector<bool>();
      // ??????reversed_after_transpose?????????????????????if empty?????????, ?????????merge
      // prepare res_edge
      // res_edge means edge_6 but type is different
      auto result_edge = decltype(core->edges)();
      auto merge_offset = pmr::vector<pmr::map<pmr::vector<Symmetry>, std::tuple<Symmetry, Size>>>();
      if (!merge_map.empty()) {
         if constexpr (is_fermi) {
            reversed_after_transpose_flag.reserve(rank_at_transpose);
         }
         result_edge.reserve(rank_after_merge);
         merge_offset.reserve(rank_after_merge);
         for (Rank position_after_merge = 0, start_of_merge = 0, end_of_merge = 0; position_after_merge < rank_after_merge; position_after_merge++) {
            // [start, end) need be merged
            while (end_of_merge < rank_at_transpose && merge_flag[end_of_merge] == position_after_merge) {
               end_of_merge++;
            }
            // arrow begin
            Arrow arrow;
            bool arrow_fixed = false;
            if constexpr (is_fermi) {
               for (auto merge_group_position = start_of_merge; merge_group_position < end_of_merge; merge_group_position++) {
                  if (edge_before_merge[merge_group_position].arrow_valid()) {
                     if (arrow_fixed) {
                        if (arrow == edge_before_merge[merge_group_position].arrow) {
                           reversed_after_transpose_flag.push_back(false);
                        } else {
                           edge_before_merge[merge_group_position].arrow ^= true;
                           reversed_after_transpose_flag.push_back(true);
                        }
                     } else {
                        arrow_fixed = true;
                        arrow = edge_before_merge[merge_group_position].arrow;
                        reversed_after_transpose_flag.push_back(false);
                     }
                  } else {
                     reversed_after_transpose_flag.push_back(false);
                  }
               }
            }
            // arrow end

            // merge edge begin
            auto& merged_edge = result_edge.emplace_back();
            auto& this_offset = merge_offset.emplace_back();

            const Rank merge_rank = end_of_merge - start_of_merge;
            auto accumulated_symmetries = pmr::vector<Symmetry>(merge_rank);
            auto accumulated_dimensions = pmr::vector<Size>(merge_rank);
            auto current_symmetries = pmr::vector<Symmetry>(merge_rank);

            if (merge_rank != 1) {
               loop_edge(
                     edge_before_merge.data() + start_of_merge,
                     merge_rank,
                     [&merged_edge, &this_offset]() {
                        merged_edge.map[Symmetry()] = 1;
                        this_offset[pmr::vector<Symmetry>{}] = {Symmetry(), 0};
                     },
                     []() {},
                     [&](const auto& symmetry_iterator_list, const Rank minimum_changed) {
                        for (auto i = minimum_changed; i < merge_rank; i++) {
                           const auto& symmetry_iterator = symmetry_iterator_list[i];
                           accumulated_symmetries[i] = symmetry_iterator->first + (i ? accumulated_symmetries[i - 1] : Symmetry());
                           accumulated_dimensions[i] = symmetry_iterator->second * (i ? accumulated_dimensions[i - 1] : 1);
                           // do not check dim=0, because in constructor, i didn't check
                           current_symmetries[i] = symmetry_iterator->first;
                        }
                        auto target_symmetry = accumulated_symmetries.back();
                        this_offset[current_symmetries] = {target_symmetry, merged_edge.map[target_symmetry]};
                        merged_edge.map[target_symmetry] += accumulated_dimensions.back();
                        return merge_rank;
                     });
               if constexpr (is_fermi) {
                  merged_edge.arrow = arrow;
               }
            } else {
               const auto& target_edge = edge_before_merge[start_of_merge];
               merged_edge.map = target_edge.map;
               if constexpr (is_fermi) {
                  merged_edge.arrow = target_edge.arrow;
               }
               // ???????????????this_offset?????????????????????
            }
            // merge edge end
            start_of_merge = end_of_merge;
         }
      } else {
         if constexpr (is_fermi) {
            reversed_after_transpose_flag = pmr::vector<bool>(rank_at_transpose, false);
         }
         result_edge.reserve(rank_after_merge);
         for (auto i = 0; i < rank_after_merge; i++) {
            // ??????merge?????????reversed after transpose, ????????????auto& &edge_before_merge = edge_after_transpose
            // edge_before_merge?????????????????????
            const auto& edge = edge_before_merge[i];
            if constexpr (is_fermi) {
               result_edge.push_back({edge.arrow, edge.map});
            } else {
               result_edge.push_back({edge.map});
            }
            // ???????????????????????????????????????merge????????????this_offset, ??????????????????merge?????????????????????emplace_back
            // auto& this_offset = merge_offset.emplace_back();
            // for (const auto& [symmetry, dimension] : edge.map) {
            //    this_offset[{symmetry}] = {symmetry, 0};
            // }
         }
      }
      // the code above is dealing with edge_5 and res_Edge and reversed_flag_dst

      // put res_edge into res
      result.core = std::make_shared<Core<ScalarType, Symmetry>>(result_edge);
      check_valid_name(result.names, result.core->edges.size());
      // edge_6
      const auto& edge_after_merge = result.core->edges;
      // 2. ????????????data????????????

      // ???????????????data??????????????????
      //
      // before->source          symmetry[at_transpose]->(symmetry[before_split], offset[before_split])
      // after->destination      symmetry[at_transpose]->(symmetry[after_merge], offset[after_merge])
      //
      // marks:
      // auto split_flag_mark
      // reversed_before_transpose_flag_mark
      // reversed_after_transpose_flag_mark
      // merge_flag_mark

      using VectorSymmetryOfTensor = typename decltype(core->blocks)::key_type;

      using NormalMapFromTransposeToSourceDestination = pmr::map<VectorSymmetryOfTensor, std::tuple<VectorSymmetryOfTensor, pmr::vector<Size>>>;
      using FakeMapFromTransposeToSourceDestination = fake_map<VectorSymmetryOfTensor, std::tuple<VectorSymmetryOfTensor, pmr::vector<Size>>>;
#ifdef TAT_USE_SIMPLE_NOSYMMETRY
      using MapFromTransposeToSourceDestination = std::
            conditional_t<std::is_same_v<Symmetry, NoSymmetry>, FakeMapFromTransposeToSourceDestination, NormalMapFromTransposeToSourceDestination>;
#else
      using MapFromTransposeToSourceDestination = NormalMapFromTransposeToSourceDestination;
#endif
      auto data_before_transpose_to_source = MapFromTransposeToSourceDestination();
      if (!split_map.empty() || (is_fermi && !reversed_name.empty())) {
         // ????????????reversed??????symmetry?????????
         // 1. ?????????????????????reversed???????????????else??????edge????????? 2.????????????edge_after_split?????????edge_before_transpose
         for (auto& [symmetries_before_transpose, size] : initialize_block_symmetries_with_check(edge_after_split)) {
            // convert sym -> target_sym and offsets
            // and add to map
            auto symmetries = VectorSymmetryOfTensor();
            auto offsets = pmr::vector<Size>();
            symmetries.reserve(rank_before_split);
            offsets.reserve(rank_before_split);
            bool success = true;
            for (Rank position_before_split = 0, position_after_split = 0; position_before_split < rank_before_split; position_before_split++) {
               // [start, end) be merged
               auto split_group_symmetries = pmr::vector<Symmetry>(); // ???????????????split???edge???
               while (position_after_split < rank_at_transpose && split_flag[position_after_split] == position_before_split) {
                  split_group_symmetries.push_back(symmetries_before_transpose[position_after_split]);
                  position_after_split++;
               }
               // ????????????split, ????????????split_offset
               if (split_group_symmetries.size() != 1) {
                  if (auto found = split_offset[position_before_split].find(split_group_symmetries);
                      found != split_offset[position_before_split].end()) {
                     const auto& [this_symmetry, this_offset] = found->second;
                     symmetries.push_back(this_symmetry);
                     offsets.push_back(this_offset);
                  } else {
                     success = false;
                     break;
                  }
               } else {
                  symmetries.push_back(split_group_symmetries.front());
                  offsets.push_back(0);
               }
            }
            if (success) {
               data_before_transpose_to_source[{symmetries_before_transpose.begin(), symmetries_before_transpose.end()}] = {
                     std::move(symmetries), std::move(offsets)};
            }
         }
      } else {
         for (const auto& [symmetries, block] : core->blocks) {
            data_before_transpose_to_source[symmetries] = {symmetries, pmr::vector<Size>(rank_before_split, 0)};
         }
      }
      auto data_after_transpose_to_destination = MapFromTransposeToSourceDestination();
      if (!merge_map.empty()) {
         for (auto& [symmetries_after_transpose, size] : initialize_block_symmetries_with_check(edge_before_merge)) {
            // convert sym -> target_sym and offsets
            // and add to map
            auto symmetries = VectorSymmetryOfTensor();
            auto offsets = pmr::vector<Size>();
            symmetries.reserve(rank_after_merge);
            offsets.reserve(rank_after_merge);
            bool success = true;
            for (Rank position_after_merge = 0, position_before_merge = 0; position_after_merge < rank_after_merge; position_after_merge++) {
               // [start, end) be merged
               auto merge_group_symmetries = pmr::vector<Symmetry>(); // ???????????????merge???edge???
               while (position_before_merge < rank_at_transpose && merge_flag[position_before_merge] == position_after_merge) {
                  merge_group_symmetries.push_back(symmetries_after_transpose[position_before_merge]);
                  position_before_merge++;
               }
               if (merge_group_symmetries.size() != 1) {
                  if (auto found = merge_offset[position_after_merge].find(merge_group_symmetries);
                      found != merge_offset[position_after_merge].end()) {
                     const auto& [this_symmetry, this_offset] = found->second;
                     symmetries.push_back(this_symmetry);
                     offsets.push_back(this_offset);
                  } else {
                     success = false;
                     break;
                  }
               } else {
                  symmetries.push_back(merge_group_symmetries.front());
                  offsets.push_back(0);
               }
            }
            if (success) {
               data_after_transpose_to_destination[{symmetries_after_transpose.begin(), symmetries_after_transpose.end()}] = {
                     std::move(symmetries), std::move(offsets)};
            }
         }
      } else {
         for (const auto& [symmetries, block] : result.core->blocks) {
            data_after_transpose_to_destination[symmetries] = {symmetries, pmr::vector<Size>(rank_after_merge, 0)};
         }
      }

      // 3. 4 marks
      auto split_flag_mark = pmr::vector<bool>();
      auto reversed_before_transpose_flag_mark = pmr::vector<bool>();
      auto reversed_after_transpose_flag_mark = pmr::vector<bool>();
      auto merge_flag_mark = pmr::vector<bool>();
      if constexpr (is_fermi) {
         split_flag_mark.reserve(rank_before_split);
         reversed_before_transpose_flag_mark.reserve(rank_at_transpose);
         reversed_after_transpose_flag_mark.reserve(rank_at_transpose);
         merge_flag_mark.reserve(rank_after_merge);
         // true => ??????parity
         if (apply_parity) {
            // ????????????, ??????????????????exclude???, ???find==end
            for (auto i = 0; i < rank_before_split; i++) {
               split_flag_mark.push_back(parity_exclude_name[0].find(name_before_split[i]) == parity_exclude_name[0].end());
            }
            for (auto i = 0; i < rank_at_transpose; i++) {
               reversed_before_transpose_flag_mark.push_back(parity_exclude_name[1].find(name_after_split[i]) == parity_exclude_name[1].end());
            }
            for (auto i = 0; i < rank_at_transpose; i++) {
               reversed_after_transpose_flag_mark.push_back(parity_exclude_name[2].find(name_before_merge[i]) == parity_exclude_name[2].end());
            }
            for (auto i = 0; i < rank_after_merge; i++) {
               merge_flag_mark.push_back(parity_exclude_name[3].find(name_after_merge[i]) == parity_exclude_name[3].end());
            }
         } else {
            for (auto i = 0; i < rank_before_split; i++) {
               split_flag_mark.push_back(parity_exclude_name[0].find(name_before_split[i]) != parity_exclude_name[0].end());
            }
            for (auto i = 0; i < rank_at_transpose; i++) {
               reversed_before_transpose_flag_mark.push_back(parity_exclude_name[1].find(name_after_split[i]) != parity_exclude_name[1].end());
            }
            for (auto i = 0; i < rank_at_transpose; i++) {
               reversed_after_transpose_flag_mark.push_back(parity_exclude_name[2].find(name_before_merge[i]) != parity_exclude_name[2].end());
            }
            for (auto i = 0; i < rank_after_merge; i++) {
               merge_flag_mark.push_back(parity_exclude_name[3].find(name_after_merge[i]) != parity_exclude_name[3].end());
            }
         }
      }
      // ?????????????????????????????????
      if constexpr (!std::is_same_v<Symmetry, NoSymmetry>) {
         // TODO ?????????????????????, call zero???????????????????????????
         result.zero();
      }
      // ??????????????????????????????????????????????????????, ??????????????????ansatz????????????, ?????????????????????????????????
      // ???????????????, ????????????????????????????????????????????????????????????, ???????????????????????????????????????, ????????????????????????0
      // 5. main copy loop
      for (const auto& [symmetries_before_transpose, source_symmetries_and_offsets] : data_before_transpose_to_source) {
         // ????????????????????????
         //                   source                     destination
         // symmetries        symmetry[before_split]     symmetry[after_merge]
         // offsets           offset[before_split]       offset[after_merge]
         //
         //                   before_transpose           after_transpose
         // symmetries        symmetry[at_transpose]     symmetry[at_transpose]
         // dimensions        dimension[at_transpose]    dimension[at_transpose]
         //
         //                   source         destination
         // total_offset      O              O
         // block             O              O
         //
         // leadings
         // leadings_of_source[before_split] -> leadings_before_transpose[at_transpose]
         // leadings_of_destination[after_merge] -> leadings_after_transpose[at_transpose]
         const auto& [source_symmetries, source_offsets] = source_symmetries_and_offsets;

         auto symmetries_after_transpose = VectorSymmetryOfTensor(rank_at_transpose);
         auto dimensions_before_transpose = pmr::vector<Size>(rank_at_transpose);
         auto dimensions_after_transpose = pmr::vector<Size>(rank_at_transpose);
         Size total_size = 1;
         for (auto i = 0; i < rank_at_transpose; i++) {
            auto dimension = edge_before_transpose[i].map.at(symmetries_before_transpose[i]);
            dimensions_before_transpose[i] = dimension;
            dimensions_after_transpose[plan_source_to_destination[i]] = dimension;
            symmetries_after_transpose[plan_source_to_destination[i]] = symmetries_before_transpose[i];
            total_size *= dimension;
         }

         const auto& [destination_symmetries, destination_offsets] = data_after_transpose_to_destination.at(symmetries_after_transpose);

         // ??????????????????symmetry, ??????offset, ??????dimension
         // ????????????leadings????????????

         const auto& source_block = core->blocks.at(source_symmetries);
         auto& destination_block = result.core->blocks.at(destination_symmetries);

         Size total_source_offset = 0;
         for (auto i = 0; i < rank_before_split; i++) {
            // ?????????edge_before_split??????core->edges
            total_source_offset *= core->edges[i].map.at(source_symmetries[i]);
            total_source_offset += source_offsets[i];
         }
         Size total_destination_offset = 0;
         for (auto i = 0; i < rank_after_merge; i++) {
            total_destination_offset *= edge_after_merge[i].map.at(destination_symmetries[i]);
            total_destination_offset += destination_offsets[i];
         }

         auto leadings_of_source = pmr::vector<Size>(rank_before_split);
         for (auto i = rank_before_split; i-- > 0;) {
            if (i == rank_before_split - 1) {
               leadings_of_source[i] = 1;
            } else {
               // ?????????edge_before_split??????core->edges
               leadings_of_source[i] = leadings_of_source[i + 1] * core->edges[i + 1].map.at(source_symmetries[i + 1]);
            }
         }
         auto leadings_before_transpose = pmr::vector<Size>(rank_at_transpose);
         for (auto i = rank_at_transpose; i-- > 0;) {
            if (i != rank_at_transpose - 1 && split_flag[i] == split_flag[i + 1]) {
               leadings_before_transpose[i] = leadings_before_transpose[i + 1] * dimensions_before_transpose[i + 1];
               // dimensions_before_transpose[i + 1] == edge_before_transpose[i + 1].map.at(symmetries_before_transpose[i + 1]);
            } else {
               leadings_before_transpose[i] = leadings_of_source[split_flag[i]];
            }
         }

         auto leadings_of_destination = pmr::vector<Size>(rank_after_merge);
         for (auto i = rank_after_merge; i-- > 0;) {
            if (i == rank_after_merge - 1) {
               leadings_of_destination[i] = 1;
            } else {
               leadings_of_destination[i] = leadings_of_destination[i + 1] * edge_after_merge[i + 1].map.at(destination_symmetries[i + 1]);
            }
         }
         auto leadings_after_transpose = pmr::vector<Size>(rank_at_transpose);
         for (auto i = rank_at_transpose; i-- > 0;) {
            if (i != rank_at_transpose - 1 && merge_flag[i] == merge_flag[i + 1]) {
               leadings_after_transpose[i] = leadings_after_transpose[i + 1] * dimensions_after_transpose[i + 1];
               // dimensions_after_transpose[i + 1] == edge_after_transpose[i + 1].map.at(symmetries_after_transpose[i + 1]);
            } else {
               leadings_after_transpose[i] = leadings_of_destination[merge_flag[i]];
            }
         }

         // parity
         auto parity = false;
         if constexpr (is_fermi) {
            parity = Symmetry::get_transpose_parity(symmetries_before_transpose, plan_source_to_destination);

            parity ^= Symmetry::get_reverse_parity(symmetries_before_transpose, reversed_before_transpose_flag, reversed_before_transpose_flag_mark);
            parity ^= Symmetry::get_split_merge_parity(symmetries_before_transpose, split_flag, split_flag_mark);
            parity ^= Symmetry::get_reverse_parity(symmetries_after_transpose, reversed_after_transpose_flag, reversed_after_transpose_flag_mark);
            parity ^= Symmetry::get_split_merge_parity(symmetries_after_transpose, merge_flag, merge_flag_mark);
         }

         do_transpose(
               source_block.data() + total_source_offset,
               destination_block.data() + total_destination_offset,
               plan_source_to_destination,
               plan_destination_to_source,
               dimensions_before_transpose,
               dimensions_after_transpose,
               leadings_before_transpose,
               leadings_after_transpose,
               rank_at_transpose,
               total_size,
               parity);
      }

      return result;
   }
} // namespace TAT
#endif
