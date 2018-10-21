DROP VIEW IF EXISTS view_random_matched_text_for;
CREATE VIEW view_random_matched_text_for AS
WITH 
step AS (
  SELECT
    rfp.pos AS start_pos,
    rfp.vertex AS start_vertex,
    vertex_p.type AS type,
    vertex_p.name AS name,
    rep.pos AS final_pos,
    rep.vertex AS final_vertex,
    SUBSTR(
      input_data.utf8,
      rfp.pos,
      rep.pos - rfp.pos
    ) AS matched_text
  FROM
    random_full_path rfp
      INNER JOIN vertex_property vertex_p ON (vertex_p.vertex = rfp.vertex)
      INNER JOIN random_endpoints rep ON (rfp.path_id = rep.endpoint_id)
      INNER JOIN input_data
  WHERE
    vertex_p.is_push
    AND
    vertex_p.name = (SELECT name FROM command_line_defines)
  ORDER BY
    rfp.path_id
)
SELECT
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
FROM
  step
GROUP BY
  NULL
;
