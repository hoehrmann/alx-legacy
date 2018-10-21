DROP VIEW IF EXISTS view_cfinals;
CREATE VIEW view_cfinals AS
SELECT
  src.pos AS src_pos,
  src.vertex AS src_vertex,
  dst.pos AS dst_pos,
  dst.vertex AS dst_vertex
FROM
  cfinals e 
    INNER JOIN vertex_index src
      ON (src.node_id = e.src_id)
    INNER JOIN vertex_index dst
      ON (dst.node_id = e.dst_id)
