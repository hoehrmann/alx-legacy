DROP VIEW IF EXISTS view_graph_quads_reachable_node;
CREATE VIEW view_graph_quads_reachable_node AS
SELECT src_id AS node_id FROM view_graph_quads_reachable
UNION
SELECT mid_src_id AS node_id FROM view_graph_quads_reachable
UNION
SELECT mid_dst_id AS node_id FROM view_graph_quads_reachable
UNION
SELECT dst_id AS node_id FROM view_graph_quads_reachable
;
