DROP VIEW IF EXISTS view_random_full_path_signature;
CREATE VIEW view_random_full_path_signature AS
WITH
raw AS (
  SELECT
    rfp.pos AS src_pos,
    vertex_p.type AS src_type,
    vertex_p.name AS src_name,
    LEAD(rfp.pos) OVER w AS dst_pos,
    LEAD(vertex_p.type) OVER w AS dst_type,
    LEAD(vertex_p.name) OVER w AS dst_name
  FROM
    random_full_path rfp
      INNER JOIN vertex_property vertex_p
        ON (rfp.vertex = vertex_p.vertex)
  WHERE
    (vertex_p.is_push OR vertex_p.is_pop)
  WINDOW w AS (ORDER BY rfp.path_id)
)
SELECT
  *
FROM
  raw
WHERE
  dst_pos IS NOT NULL
ORDER BY
  src_pos,
  src_type,
  src_name,
  dst_pos,
  dst_type,
  dst_name
;
