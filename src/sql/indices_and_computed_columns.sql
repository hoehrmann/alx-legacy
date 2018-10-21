---------------------------------------------------------------------
-- view_graph_quads_reachable performance supplements.
---------------------------------------------------------------------

CREATE INDEX IF NOT EXISTS idx_graph_quads_src_id
  ON graph_quads(src_id);

CREATE INDEX IF NOT EXISTS idx_graph_quads_dst_id
  ON graph_quads(dst_id);

ALTER TABLE graph_quads ADD COLUMN _siblings INT;

UPDATE
  graph_quads
SET
  _siblings = COALESCE(
    (
      SELECT 
        -- TODO: is it okay to ignore (Final, NULL, NULL, Start)?
        mid_src_p.type IN ('Final', 'Fi', 'Fi1', 'Fi2')
      FROM
        graph_nodes mid_src_node
          INNER JOIN vertex_property mid_src_p
            ON (mid_src_p.vertex = mid_src_node.vertex)
      WHERE
        mid_src_node.node_id = graph_quads.mid_src_id
    )
    ,
    0
  )
;
