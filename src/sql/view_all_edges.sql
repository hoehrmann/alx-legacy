DROP VIEW IF EXISTS view_all_edges;
CREATE VIEW view_all_edges AS
SELECT
  src.pos AS src_pos,
  src.vertex AS src_vertex,
  dst.pos AS dst_pos,
  dst.vertex AS dst_vertex
FROM
  all_edges_edges e 
    INNER JOIN all_edges_index src
      ON (src.node_id = e.src_id)
    INNER JOIN all_edges_index dst
      ON (dst.node_id = e.dst_id)
