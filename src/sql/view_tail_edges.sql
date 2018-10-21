DROP VIEW IF EXISTS view_tail_edges;
CREATE VIEW view_tail_edges AS
WITH
max_dst_pos AS (
  SELECT MAX(dst_pos) AS max_dst_pos FROM view_tail_edges_data
),
args AS (
  SELECT
    max_dst_pos - COALESCE(d.tail_length, 100) AS min_pos,
    max_dst_pos AS max_pos
  FROM
    max_dst_pos
      INNER JOIN command_line_defines d
)
SELECT
  e.src_pos AS src_pos,
  e.src_vertex AS src_vertex,
  e.dst_pos AS dst_pos,
  e.dst_vertex AS dst_vertex
FROM
  args 
    INNER JOIN command_line_defines d 
    INNER JOIN view_tail_edges_data e 
WHERE
  CASE
  WHEN d.max_edges IS NULL THEN
    src_pos BETWEEN args.min_pos AND args.max_pos
    AND
    dst_pos BETWEEN args.min_pos AND args.max_pos
  ELSE
    1
  END 
ORDER BY 
  e.src_pos DESC,
  e.src_vertex DESC,
  e.dst_pos DESC,
  e.dst_vertex DESC
LIMIT 
  COALESCE( (SELECT max_edges FROM command_line_defines), -1 )
