WITH 
base AS (
  SELECT
    rfp.pos AS start_pos,
    rfp.vertex AS start_vertex,
    vertex_p.type AS type,
    vertex_p.name AS name,
    rep.pos AS final_pos,
    rep.vertex AS final_vertex,
    vertex_p.is_push AS is_push,
    vertex_p.is_pop AS is_pop,
    rfp.path_id AS path_id
  FROM
    random_full_path rfp
      INNER JOIN vertex_property vertex_p ON (vertex_p.vertex = rfp.vertex)
      INNER JOIN random_endpoints rep ON (rfp.path_id = rep.endpoint_id)
      INNER JOIN input_data
  WHERE
    vertex_p.is_push
    OR
    vertex_p.is_pop

/*
  WHERE
    vertex_p.is_push
    AND
    vertex_p.name = (SELECT name FROM command_line_defines)
*/
  ORDER BY
    rfp.path_id
),
step AS (
  SELECT
    *,
    SUBSTR(
      input_data.utf8,
      start_pos,
      final_pos - start_pos
    ) AS matched_text,
    LEAD(start_pos) OVER (order by path_id) AS next_pos
    
  FROM
    base
      INNER JOIN input_data
)
SELECT

  case
  when is_push then printf(
    '<m name="%s">%s',
    name,
    json_quote(substr(
      input_data.utf8,
      start_pos,
      next_pos - start_pos
    ))
  )
  when is_pop then printf(
    '</m>%s',
    json_quote(substr(
      input_data.utf8,
      start_pos,
      next_pos - start_pos
    ))
  )
  end as line

/*
  JSON_GROUP_ARRAY(
    JSON_OBJECT(
      'start_pos',
      start_pos,
      'start_vertex',
      start_vertex,
      'type',
      type,
      'final_pos',
      final_pos,
      'final_vertex',
      final_vertex,
      'matched_text',
      matched_text
    )
  ) AS line
*/

FROM
  step
    INNER JOIN input_data
LIMIT 200
;
