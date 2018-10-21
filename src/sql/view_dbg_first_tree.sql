DROP VIEW IF EXISTS view_dbg_first_tree;
CREATE VIEW view_dbg_first_tree AS
WITH foo AS (
  SELECT
    id,
    quad_id,
    true_depth,
    CASE
    WHEN _siblings = 0 AND mode = 'src' THEN src_id
    WHEN _siblings = 0 AND mode = 'dst' THEN dst_id
    END AS node_id
  FROM
    view_first_tree_path
), bar AS (
  SELECT
    id,
    quad_id,
    case when graph_nodes.node_id IS NOT NULL THEN printf(
      '%*s %s,%s %s %s (%s)',
      4*true_depth,
      '',
      graph_nodes.pos,
      graph_nodes.vertex,
      vertex_property.type,
      vertex_property.name,
      vertex_property.partner
    ) ELSE NULL END AS x
  FROM
    foo
    LEFT JOIN graph_nodes ON foo.node_id = graph_nodes.node_id
    LEFT JOIN vertex_property ON graph_nodes.vertex = vertex_property.vertex
)
SELECT * FROM bar
