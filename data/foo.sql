
args AS (
  COALESCE(
    :min_pos,
    1
  ) AS min_pos,
  COALESCE(
    :max_pos,
    (SELECT final_index FROM parser_state)
  ) AS max_pos
),

WITH
edge AS (
  SELECT
    src.pos AS src_pos,
    src.vertex AS src_vertex,
    dst.pos AS dst_pos,
    dst.vertex AS dst_vertex
  FROM
    all_edges_edges e 
      INNER JOIN all_edges_index src
        ON (src.node_id = e.src_id)
      INNER JOIN all_edges_index dst
        ON (dst.node_id = e.dst_id)
),
node AS (
  SELECT 
    src_pos AS pos,
    src_vertex AS vertex
  FROM
    edge
  UNION
  SELECT
    dst_pos AS pos,
    dst_vertex AS vertex
  FROM
    edge
),
node_dotlines AS (
  SELECT 
    printf(
      '"%u,%u"[label="%u,%u,%s,%s%s"%s];',
      node.pos,
      node.vertex,
      node.pos,
      node.vertex,
      vertex_p.type,
      vertex_p.name,
      CASE
      WHEN vertex_p.partner
      THEN ''
--      THEN printf(',(pairs with %u)', vertex_p.partner)
      ELSE ''
      END,
      CASE
      WHEN vertex_p.is_push
      THEN printf(',shape=rarrow,style=filled,fillcolor="#0392ce"')
      WHEN vertex_p.is_pop
      THEN printf(',shape=larrow,style=filled,fillcolor="#fd5308"')
      ELSE ',shape=none'
      END
    ) AS line
  FROM
    node
      INNER JOIN vertex_property vertex_p
        ON (node.vertex = vertex_p.vertex)
),
edge_dotline AS (
  SELECT 
    printf(
      '"%u,%u" -> "%u,%u";',
      src_pos,
      src_vertex,
      dst_pos,
      dst_vertex
    ) AS line
  FROM
    edge
),
dotlines AS (
  SELECT 1 as sort_key, 'digraph {' AS line 
  UNION ALL
  SELECT 2 as sort_key, line FROM node_dotlines
  UNION ALL
  SELECT 3 as sort_key, line FROM edge_dotline
  UNION ALL
  SELECT 4 as sort_key, '}' AS line 
)
SELECT
  line
FROM
  dotlines
ORDER BY
  sort_key
