#pragma once

#include <cstddef>
#include <functional>
#include <utility>

namespace graaf {

using vertex_id_t = std::size_t;
using edge_id_t = std::pair<vertex_id_t, vertex_id_t>;

struct edge_id_hash {
    [[nodiscard]] auto operator()(const edge_id_t &key) const noexcept -> std::size_t
    {
        const auto h1{std::hash<vertex_id_t>{}(key.first)};
        const auto h2{std::hash<vertex_id_t>{}(key.second)};

        // TODO: use something like boost::hash_combine
        return h1 ^ h2;
    }
};

}  // namespace graaf
