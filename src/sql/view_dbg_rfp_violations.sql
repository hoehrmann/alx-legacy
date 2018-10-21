DROP VIEW IF EXISTS view_dbg_rfp_violations;
CREATE VIEW view_dbg_rfp_violations AS
SELECT
  p1.path_id AS src_path_id,
  p2.path_id AS dst_path_id
FROM
  random_full_path p1
    LEFT JOIN random_full_path p2
      ON (p2.path_id - 1 = p1.path_id)
    LEFT JOIN old_edge e
      ON (
        e.src_vertex = p1.vertex
        AND
        e.dst_vertex = p2.vertex
      )
WHERE
  e.rowid IS NULL
  AND
  p2.rowid IS NOT NULL
;
