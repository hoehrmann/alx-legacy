WITH RECURSIVE
base AS (
  SELECT
    offset AS pos,
    vertex
  FROM
    shadowed_inputs
),
unshadowed AS (
  SELECT
    base.pos,
    COALESCE(vertex_shadows.shadows, base.vertex) AS vertex
  FROM
    base 
      LEFT JOIN vertex_shadows
        ON (base.vertex = vertex_shadows.vertex)
/*
  WHERE
    base.pos = (SELECT MAX(offset) FROM shadowed_inputs)
*/
),
step AS (
  SELECT
    u.pos - (src_p.type = 'Input') AS src_pos,
    e.src_vertex,
    u.pos AS dst_pos,
    e.dst_vertex AS dst_vertex
  FROM
    unshadowed u
      INNER JOIN old_edge e
        ON (e.dst_vertex = u.vertex)
      INNER JOIN vertex_property src_p
        ON (e.src_vertex = src_p.vertex)
  UNION
  SELECT
    step.src_pos - (src_p.type = 'Input') AS src_pos,
    e.src_vertex AS src_vertex,
    step.src_pos AS dst_pos,
    e.dst_vertex AS dst_vertex
  FROM
    step
      INNER JOIN old_edge e
        ON (step.src_vertex = e.dst_vertex)
      INNER JOIN vertex_property src_p
        ON (e.src_vertex = src_p.vertex)
)
SELECT
  *
FROM
  step
LIMIT
  40
