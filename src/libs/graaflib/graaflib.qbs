VLib {
    Depends { name: "cpp" }

    name: "GraafLib"
    files: [
        "algorithm/clique_detection/bron_kerbosch.h",
        "algorithm/clique_detection/bron_kerbosch.tpp",
        "algorithm/coloring/greedy_graph_coloring.h",
        "algorithm/coloring/greedy_graph_coloring.tpp",
        "algorithm/coloring/welsh_powell.h",
        "algorithm/coloring/welsh_powell.tpp",
        "algorithm/cycle_detection/dfs_cycle_detection.h",
        "algorithm/cycle_detection/dfs_cycle_detection.tpp",
        "algorithm/graph_traversal/breadth_first_search.h",
        "algorithm/graph_traversal/breadth_first_search.tpp",
        "algorithm/graph_traversal/common.h",
        "algorithm/graph_traversal/depth_first_search.h",
        "algorithm/graph_traversal/depth_first_search.tpp",
        "algorithm/minimum_spanning_tree/kruskal.h",
        "algorithm/minimum_spanning_tree/kruskal.tpp",
        "algorithm/minimum_spanning_tree/prim.h",
        "algorithm/minimum_spanning_tree/prim.tpp",
        "algorithm/shortest_path/a_star.h",
        "algorithm/shortest_path/a_star.tpp",
        "algorithm/shortest_path/bellman_ford.h",
        "algorithm/shortest_path/bellman_ford.tpp",
        "algorithm/shortest_path/bfs_shortest_path.h",
        "algorithm/shortest_path/bfs_shortest_path.tpp",
        "algorithm/shortest_path/common.h",
        "algorithm/shortest_path/common.tpp",
        "algorithm/shortest_path/dijkstra_shortest_path.h",
        "algorithm/shortest_path/dijkstra_shortest_path.tpp",
        "algorithm/shortest_path/dijkstra_shortest_paths.h",
        "algorithm/shortest_path/dijkstra_shortest_paths.tpp",
        "algorithm/shortest_path/floyd_warshall.h",
        "algorithm/shortest_path/floyd_warshall.tpp",
        "algorithm/strongly_connected_components/common.h",
        "algorithm/strongly_connected_components/kosaraju.h",
        "algorithm/strongly_connected_components/kosaraju.tpp",
        "algorithm/strongly_connected_components/tarjan.h",
        "algorithm/strongly_connected_components/tarjan.tpp",
        "algorithm/topological_sorting/dfs_topological_sorting.h",
        "algorithm/topological_sorting/dfs_topological_sorting.tpp",
        "algorithm/utils.h",
        "algorithm/utils.tpp",
        "edge.h",
        "edge.tpp",
        "graph.h",
        "graph.tpp",
        "io/dot.h",
        "io/dot.tpp",
        "properties/vertex_properties.h",
        "properties/vertex_properties.tpp",
        "types.h",
    ]

    Export {
        Depends { name: "cpp" }
        cpp.includePaths: [exportingProduct.sourceDirectory]
    }
}
