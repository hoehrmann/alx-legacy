DROP VIEW IF EXISTS view_grammar_edges;
CREATE VIEW view_grammar_edges AS
SELECT
  NULL AS src_pos,
  e.src_vertex AS src_vertex,
  NULL AS dst_pos,
  e.dst_vertex AS dst_vertex
FROM
  old_edge e
