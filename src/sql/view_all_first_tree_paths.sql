---------------------------------------------------------------------
-- The `graph_quads` table describes the nesting structure of irregular
-- parts of the match using four vertices (two for leaves) in the sense
-- that there is a properly balanced path going over the four vertices
-- in order. There are two different cases, nesting like `<a><b/></a>`,
-- and siblings like `<a/>...<b/>`.
-- 
-- This VIEW gives for each `quad_id` taken as `root_id` the first tree
-- contained within the forest as a sequence of `quad_id`s in pre-order,
-- in other words, the first path through the forest.
--
-- Each `quad_id` actually appears twice under different `mode`s. Once
-- for the start vertex and once for the final vertex of a balanced
-- nesting. This is meant to aid writing the tree out linearly.
--
-- This can be used to enumerate all trees in the forest. To find the
-- next tree, find the last quad_id in the path that has an alternative
-- and replace the part of the path that corresponds to quad_ids sub-
-- graph with the result of this VIEW where `root_id` is  alternative
-- quad_id.
---------------------------------------------------------------------
DROP VIEW IF EXISTS view_all_first_tree_paths;
CREATE VIEW view_all_first_tree_paths AS
WITH RECURSIVE
/*
first_quad_ids AS (
  SELECT
    MIN(quad_id) AS quad_id
  FROM
    graph_quads
  GROUP BY
    src_id,
    dst_id
),
first_quads AS (
  SELECT
    *
  FROM
    graph_quads
      INNER JOIN first_quad_ids
        ON graph_quads.quad_id = first_quad_ids.quad_id 
),
*/
modes(mode) AS (
  VALUES('src')
  UNION ALL
  VALUES('dst')
),
step AS (
  SELECT
    quad_id AS root_id,
    quad_id,
    src_id,
    mid_src_id,
    mid_dst_id,
    dst_id,
    _siblings,
    0 as depth,
    0 AS true_depth,
    1 as nth,
    modes.mode AS mode
  FROM
    graph_quads INNER JOIN modes WHERE quad_id = 119767
  UNION
  SELECT
    s.root_id,
    q.quad_id,
    q.src_id,
    q.mid_src_id,
    q.mid_dst_id,
    q.dst_id,
    q._siblings,
    depth + 1 AS depth,
    (
      CASE
      WHEN s._siblings = 0 THEN s.true_depth + 1
      ELSE s.true_depth
      END
    ) AS true_depth,
    (
      CASE
      -- siblings lhs
      WHEN (s._siblings AND s.src_id = q.src_id AND s.mid_src_id = q.dst_id) THEN 1
      -- siblings rhs
      WHEN (s._siblings AND s.mid_dst_id = q.src_id AND s.dst_id = q.dst_id) THEN 2
      -- child
      ELSE 1
      END
    ) AS nth,
    modes.mode AS mode
  FROM
    step s INNER JOIN first_quads q ON (
      -- siblings lhs
      (s._siblings AND s.src_id = q.src_id AND s.mid_src_id = q.dst_id)
      OR
      -- siblings rhs
      (s._siblings AND s.mid_dst_id = q.src_id AND s.dst_id = q.dst_id)
      OR
      -- child
      (NOT(s._siblings) AND s.mid_src_id = q.src_id AND s.mid_dst_id = q.dst_id)
    ) INNER JOIN modes
  ORDER BY
    depth DESC,
    nth ASC,
    mode DESC
)
SELECT
  /*
    WARNING: SQLite does not explicitly guarantee that the ordering
    will match the order in which the rows were generated in the CTE.
    But there does not seem to be an alternative, short of creating a
    temporary table -- and in practise this seems to work fine.
  */
  ROW_NUMBER() OVER () AS id,
  * 
FROM
  step
