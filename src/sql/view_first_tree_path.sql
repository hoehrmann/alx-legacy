DROP VIEW IF EXISTS view_first_tree_path;
CREATE VIEW view_first_tree_path AS
SELECT
  *
FROM 
  view_all_first_tree_paths
WHERE
  root_id = (SELECT MIN(quad_id) FROM view_graph_quads_roots)
ORDER BY
  id
