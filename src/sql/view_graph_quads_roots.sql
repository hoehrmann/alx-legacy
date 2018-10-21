DROP VIEW IF EXISTS view_graph_quads_roots;
CREATE VIEW view_graph_quads_roots AS 
WITH RECURSIVE
args AS (
  SELECT
    (SELECT start_index FROM parser_state) AS start_index,
    (SELECT start_vertex FROM grammar) AS start_vertex,
    (SELECT final_index FROM parser_state) AS final_index,
    (SELECT final_vertex FROM grammar) AS final_vertex
),
roots AS (
  SELECT 
    view_graph_quads.*
  FROM
    view_graph_quads
      LEFT JOIN args
  WHERE
    src_pos = args.start_index
    AND
    dst_pos = args.final_index
    AND
    src_vertex = args.start_vertex
    AND
    dst_vertex = args.final_vertex
)
SELECT
  *
FROM
  roots
;
