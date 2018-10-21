DROP VIEW IF EXISTS view_graph_quads_reachable;
CREATE VIEW view_graph_quads_reachable AS 
WITH RECURSIVE
step AS (
  SELECT
    quad_id,
    src_id,
    mid_src_id,
    mid_dst_id,
    dst_id,
    _siblings
  FROM
    view_graph_quads_roots
  UNION
  SELECT
    q.quad_id,
    q.src_id,
    q.mid_src_id,
    q.mid_dst_id,
    q.dst_id,
    q._siblings
  FROM
    step s INNER JOIN graph_quads q ON (
      -- siblings lhs
      (s._siblings AND s.src_id = q.src_id AND s.mid_src_id = q.dst_id)
      OR
      -- siblings rhs
      (s._siblings AND s.mid_dst_id = q.src_id AND s.dst_id = q.dst_id)
      OR
      -- child
      (NOT(s._siblings) AND s.mid_src_id = q.src_id AND s.mid_dst_id = q.dst_id)
      /*
        NOTE: There are other quads that are actually used when
        building the graph like `push null null push` quads, but
        the primary use case here is actually just determining
        the set of reachable non-skippable vertices, and any such
        vertex would already be included through quads at higher
        levels.
      */
    )
)
SELECT
  *
FROM
  step
;

