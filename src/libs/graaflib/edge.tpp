#pragma once

#include "edge.h"

namespace graaf {

template<typename WEIGHTED_EDGE_T>
#if __cplusplus >= 202002L && defined(__cpp_concepts)
GRAAF_REQUIRES(derived_from_weighted_edge<WEIGHTED_EDGE_T>)
#endif
auto get_weight(const WEIGHTED_EDGE_T &edge)
    -> GRAAF_RETTYPE(typename WEIGHTED_EDGE_T::weight_t, is_weighted_edge_v<WEIGHTED_EDGE_T>)
{
    return edge.get_weight();
}

template<typename EDGE_T>
GRAAF_REQUIRES(std::is_arithmetic_v<EDGE_T>)
auto get_weight(const EDGE_T &edge) -> GRAAF_RETTYPE(EDGE_T, std::is_arithmetic_v<EDGE_T>)
{
    return edge;
}

template<typename EDGE_T>
auto get_weight(const EDGE_T & /*edge*/)
    -> GRAAF_RETTYPE(int, !is_weighted_edge_v<EDGE_T> && !std::is_arithmetic_v<EDGE_T>)
{
    // By default, an edge has unit weight
    return 1;
}

}  // namespace graaf
