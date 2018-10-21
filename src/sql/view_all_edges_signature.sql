DROP VIEW IF EXISTS view_all_edges_signature;
CREATE VIEW view_all_edges_signature AS
WITH
all_edges_edges_pp AS (
  SELECT
    s.src_id AS src_id,
    s.dst_id AS dst_id,
    (src_p.is_push OR src_p.is_pop) AS src_is_pp,
    (dst_p.is_push OR dst_p.is_pop) AS dst_is_pp
  FROM
    all_edges_edges s
      INNER JOIN all_edges_index src_node
        ON (s.src_id = src_node.node_id) 
      INNER JOIN all_edges_index dst_node
        ON (s.dst_id = dst_node.node_id) 
      INNER JOIN vertex_property src_p
        ON (src_p.vertex = src_node.vertex)
      INNER JOIN vertex_property dst_p
        ON (dst_p.vertex = dst_node.vertex)
),
step AS (
  SELECT
    s.src_id AS src_id,
    s.dst_id AS dst_id,
    s.src_is_pp AS src_is_pp,
    s.dst_is_pp AS dst_is_pp
  FROM
    all_edges_edges_pp s
  WHERE
    s.src_is_pp
  UNION
  SELECT
    s.src_id AS src_id,
    e.dst_id AS dst_id,
    s.src_is_pp AS src_is_pp,
    e.dst_is_pp AS dst_is_pp
  FROM 
    step s
      INNER JOIN all_edges_edges_pp e
        ON (s.dst_id = e.src_id)
  WHERE
    NOT(s.dst_is_pp)
)
SELECT
  src_node.pos AS src_pos,
  src_p.type AS src_type,
  src_p.name AS src_name,
  dst_node.pos AS dst_pos,
  dst_p.type AS dst_type,
  dst_p.name AS dst_name
FROM
  step s
    INNER JOIN all_edges_index src_node
      ON (s.src_id = src_node.node_id) 
    INNER JOIN all_edges_index dst_node
      ON (s.dst_id = dst_node.node_id) 
    INNER JOIN vertex_property src_p
      ON (src_p.vertex = src_node.vertex)
    INNER JOIN vertex_property dst_p
      ON (dst_p.vertex = dst_node.vertex)
WHERE
  s.src_is_pp
  AND
  s.dst_is_pp
  AND
  s.src_id IN (SELECT node_id FROM view_all_edges_reachable_node)
  AND
  s.dst_id IN (SELECT node_id FROM view_all_edges_reachable_node)
ORDER BY
  src_pos,
  src_type,
  src_name,
  dst_pos,
  dst_type,
  dst_name
;
