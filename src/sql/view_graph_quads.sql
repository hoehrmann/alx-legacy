DROP VIEW IF EXISTS view_graph_quads;
CREATE VIEW view_graph_quads AS 
SELECT

  q.*,

  src_node.pos AS src_pos,
  src_node.vertex AS src_vertex,
  mid_src_node.pos AS mid_src_pos,
  mid_src_node.vertex AS mid_src_vertex,
  mid_dst_node.pos AS mid_dst_pos,
  mid_dst_node.vertex AS mid_dst_vertex,
  dst_node.pos AS dst_pos,
  dst_node.vertex AS dst_vertex,

  -- TODO: marking pop pop nesting is not great
  CASE
  WHEN mid_src_p.is_pop THEN 'siblings'
  WHEN mid_src_id IS NULL AND src_p.is_pop THEN 'siblings'
  ELSE 'nesting'
  END AS rel

FROM
  graph_quads q
    LEFT JOIN graph_nodes src_node
      ON (q.src_id = src_node.node_id)
    LEFT JOIN graph_nodes mid_src_node
      ON (q.mid_src_id = mid_src_node.node_id)
    LEFT JOIN graph_nodes mid_dst_node
      ON (q.mid_dst_id = mid_dst_node.node_id)
    LEFT JOIN graph_nodes dst_node
      ON (q.dst_id = dst_node.node_id)
    LEFT JOIN vertex_property src_p
      ON (src_p.vertex = src_node.vertex)
    LEFT JOIN vertex_property mid_src_p
      ON (mid_src_p.vertex = mid_src_node.vertex)
    LEFT JOIN vertex_property mid_dst_p
      ON (mid_dst_p.vertex = mid_dst_node.vertex)
    LEFT JOIN vertex_property dst_p
      ON (dst_p.vertex = dst_node.vertex)
;
