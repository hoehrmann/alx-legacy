DROP VIEW IF EXISTS view_all_edges_reachable_node;
CREATE VIEW view_all_edges_reachable_node AS
WITH RECURSIVE
base AS (
  SELECT
    i.node_id AS node_id
  FROM
    view_graph_quads_reachable_node q
      INNER JOIN graph_nodes n
        ON (n.node_id = q.node_id)
      INNER JOIN all_edges_index i
        ON (i.pos = n.pos AND i.vertex = n.vertex)
),
all_edges_edges_skippable AS (
  SELECT
    e.src_id AS src_id,
    e.dst_id AS dst_id,
    src_p.skippable AS src_skippable,
    dst_p.skippable AS dst_skippable
  FROM
    all_edges_edges e 
      INNER JOIN all_edges_index d
        ON (d.node_id = e.dst_id)
      INNER JOIN all_edges_index s
        ON (s.node_id = e.src_id)
      INNER JOIN vertex_property dst_p
        ON (dst_p.vertex = d.vertex)
      INNER JOIN vertex_property src_p
        ON (src_p.vertex = s.vertex)
),
step_ltr AS (
  SELECT node_id FROM base
  UNION
  SELECT
    e.dst_id AS node_id
  FROM
    step_ltr step
      INNER JOIN all_edges_edges_skippable e 
        ON (step.node_id = e.src_id)
  WHERE
    e.dst_skippable
),
step_rtl AS (
  SELECT node_id FROM base
  UNION
  SELECT
    e.src_id AS node_id
  FROM
    step_rtl step
      INNER JOIN all_edges_edges_skippable e 
        ON (step.node_id = e.dst_id)
  WHERE
    e.src_skippable
)
SELECT
  node_id
FROM
  all_edges_index
WHERE
  node_id IN (
    SELECT node_id FROM step_rtl
    INTERSECT
    SELECT node_id FROM step_ltr
  )
;
