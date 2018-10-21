---------------------------------------------------------------------
-- Pragmas
---------------------------------------------------------------------

PRAGMA foreign_keys = OFF;
PRAGMA locking_mode = EXCLUSIVE;
PRAGMA synchronous  = OFF;
PRAGMA journal_mode = OFF;

/*
PRAGMA legacy_file_format = OFF;
PRAGMA page_size = 8192;
*/

---------------------------------------------------------------------
-- Tables
---------------------------------------------------------------------

-- TODO: NOT NULL missing in many places

DROP TABLE IF EXISTS parser_state;
CREATE TABLE parser_state(
  input_path TEXT NOT NULL,
  start_index INT NOT NULL,
  final_index INT NOT NULL
);

DROP TABLE IF EXISTS grammar;
CREATE TABLE grammar(
  start_vertex INT NOT NULL
    REFERENCES vertex_property(vertex),
  final_vertex INT NOT NULL
    REFERENCES vertex_property(vertex)
);

DROP TABLE IF EXISTS vertex_property;
CREATE TABLE vertex_property(
  vertex INTEGER PRIMARY KEY,
  type DEFAULT 'empty',
  name,
  p1 INT REFERENCES vertex_property(vertex),
  p2 INT REFERENCES vertex_property(vertex),
  partner INT REFERENCES vertex_property(vertex),
  self_loop DEFAULT 'no',
  contents_self_loop DEFAULT 'no',
  topo INT,
  skippable NOT NULL,
  epsilon_group,
  shadow_group,
  stack_group INT REFERENCES vertex_property(vertex),
  representative INT REFERENCES vertex_property(vertex),
  is_push INT NOT NULL,
  is_pop INT NOT NULL
);

DROP TABLE IF EXISTS vertex_shadows;
CREATE TABLE vertex_shadows(
  vertex INT NOT NULL
    REFERENCES vertex_property(vertex),
  shadows INT NOT NULL
    REFERENCES vertex_property(vertex)
);


-- TODO: better name for this
DROP TABLE IF EXISTS old_edge;
CREATE TABLE old_edge(
  src_vertex INT NOT NULL
    REFERENCES vertex_property(vertex),
  dst_vertex INT NOT NULL
    REFERENCES vertex_property(vertex)
);


DROP TABLE IF EXISTS input_unicode;
CREATE TABLE input_unicode(
  pos INTEGER PRIMARY KEY,
  unicode INT NOT NULL
);

-- TODO: call this nfa_transitions?
DROP TABLE IF EXISTS shadowed_inputs;
CREATE TABLE shadowed_inputs(
  shadowed_inputs_id INTEGER, -- PRIMARY KEY,
  offset INT,
  vertex INT NOT NULL
    REFERENCES vertex_property(vertex)
);

DROP TABLE IF EXISTS vertex_index;
CREATE TABLE vertex_index(
  node_id INTEGER PRIMARY KEY,
  pos INT,
  vertex INT
    REFERENCES vertex_property(vertex)
);

DROP TABLE IF EXISTS graph_links;
CREATE TABLE graph_links(
  link_id INTEGER PRIMARY KEY,
  node_id INT NOT NULL
    REFERENCES graph_quads(quad_id),
  next_id INT NOT NULL
    REFERENCES graph_links(link_id)
);

DROP TABLE IF EXISTS graph_quads;
CREATE TABLE graph_quads(
  quad_id INTEGER PRIMARY KEY,
  src_id INT NOT NULL
    REFERENCES graph_nodes(node_id),
  mid_src_id INT
    REFERENCES graph_nodes(node_id),
  mid_dst_id INT
    REFERENCES graph_nodes(node_id),
  dst_id INT NOT NULL
    REFERENCES graph_nodes(node_id)
);

DROP TABLE IF EXISTS graph_nodes;
CREATE TABLE graph_nodes(
  node_id INTEGER PRIMARY KEY,
  pos INT,
  vertex INT
    REFERENCES vertex_property(vertex)
);

DROP TABLE IF EXISTS graph_old2new;
CREATE TABLE graph_old2new(
  old_id INTEGER PRIMARY KEY
    REFERENCES vertex_index(node_id),
  new_id INT
    REFERENCES graph_nodes(node_id)
);

DROP TABLE IF EXISTS graph_successors;
CREATE TABLE graph_successors(
  node_id INTEGER PRIMARY KEY
    REFERENCES graph_nodes(node_id),
  predecessors INT NOT NULL
    REFERENCES graph_links(link_id)
);

DROP TABLE IF EXISTS graph_predecessors;
CREATE TABLE graph_predecessors(
  node_id INTEGER PRIMARY KEY
    REFERENCES graph_nodes(node_id),
  predecessors INT NOT NULL
    REFERENCES graph_links(link_id)
);

DROP TABLE IF EXISTS graph_lastmodified;
CREATE TABLE graph_lastmodified(
  node_id INTEGER PRIMARY KEY
    REFERENCES graph_nodes(node_id),
  lastmodified INT NOT NULL
);

DROP TABLE IF EXISTS graph_pending;
CREATE TABLE graph_pending(
  node_id INTEGER PRIMARY KEY
    REFERENCES graph_nodes(node_id),
  pending INT NOT NULL
);

-- uncollapsed_edges? backtrace_edges?
DROP TABLE IF EXISTS cedges;
CREATE TABLE cedges(
  edge_id INTEGER PRIMARY KEY,
  src_id INT
    REFERENCES vertex_index(node_id),
  dst_id INT
    REFERENCES vertex_index(node_id),
  UNIQUE(src_id, dst_id)
);

-- collapsed_edges?
DROP TABLE IF EXISTS cfinals;
CREATE TABLE cfinals(
  edge_id INTEGER PRIMARY KEY,
  src_id INT
    REFERENCES vertex_index(node_id),
  dst_id INT
    REFERENCES vertex_index(node_id),
  UNIQUE(src_id, dst_id)
);

DROP TABLE IF EXISTS all_edges_index;
CREATE TABLE all_edges_index(
  node_id INTEGER PRIMARY KEY,
  pos INT,
  vertex INT
    REFERENCES vertex_property(vertex)
);

DROP TABLE IF EXISTS all_edges_edges;
CREATE TABLE all_edges_edges(
  edge_id INTEGER PRIMARY KEY,
  src_id INT
    REFERENCES all_edges_index(node_id),
  dst_id INT
    REFERENCES all_edges_index(node_id)
);

DROP TABLE IF EXISTS random_sorted_quads;
CREATE TABLE random_sorted_quads(
  quad_id INTEGER PRIMARY KEY,
  src_id INT NOT NULL
    REFERENCES graph_nodes(node_id),
  mid_src_id INT
    REFERENCES graph_nodes(node_id),
  mid_dst_id INT
    REFERENCES graph_nodes(node_id),
  dst_id INT NOT NULL
    REFERENCES graph_nodes(node_id)
);

DROP TABLE IF EXISTS random_quad_path;
CREATE TABLE random_quad_path(
  path_id INTEGER PRIMARY KEY,
  node_id INT NOT NULL
    REFERENCES graph_nodes(node_id)
);

DROP TABLE IF EXISTS random_quad_stack;
CREATE TABLE random_quad_stack(
  path_id INTEGER PRIMARY KEY,
  src_id INT
    REFERENCES graph_nodes(node_id),
  dst_id INT
    REFERENCES graph_nodes(node_id)
);

DROP TABLE IF EXISTS random_gap_index;
CREATE TABLE random_gap_index(
  node_id INTEGER PRIMARY KEY,
  pos INT,
  vertex INT
    REFERENCES vertex_property(vertex)
);

DROP TABLE IF EXISTS random_gap_edges;
CREATE TABLE random_gap_edges(
  edge_id INTEGER PRIMARY KEY,
  src_id INT NOT NULL
    REFERENCES graph_nodes(node_id),
  dst_id INT NOT NULL
    REFERENCES graph_nodes(node_id)
);

DROP TABLE IF EXISTS random_full_path;
CREATE TABLE random_full_path(
  path_id INTEGER PRIMARY KEY,
  pos INT NOT NULL,
  vertex INT NOT NULL
    REFERENCES vertex_property(vertex)
);

DROP TABLE IF EXISTS random_stack;
CREATE TABLE random_stack(
  path_id INTEGER PRIMARY KEY,
  pos INT NOT NULL,
  vertex INT NOT NULL
    REFERENCES vertex_property(vertex),
  nth INT NOT NULL
);

DROP TABLE IF EXISTS random_endpoints;
CREATE TABLE random_endpoints(
  endpoint_id INTEGER PRIMARY KEY,
  pos INT NOT NULL,
  vertex INT NOT NULL
    REFERENCES vertex_property(vertex)
);

DROP TABLE IF EXISTS random_json_stack;
CREATE TABLE random_json_stack(
  json_stack_id INTEGER PRIMARY KEY,
  item INT NOT NULL
);

DROP TABLE IF EXISTS csiblings;
CREATE TABLE csiblings(
  csiblings_id INTEGER PRIMARY KEY,
  src_id INT NOT NULL
    REFERENCES graph_nodes(node_id),
  dst_id INT NOT NULL
    REFERENCES graph_nodes(node_id),
  sortkey INT NOT NULL,
  unknown INT NOT NULL
);

DROP TABLE IF EXISTS cexclusions;
CREATE TABLE cexclusions(
  cexclusions_id INTEGER PRIMARY KEY,
  src_pos INT NOT NULL,
  src_vertex INT NOT NULL
    REFERENCES vertex_property(vertex),
  dst_pos INT NOT NULL,
  dst_vertex INT NOT NULL
    REFERENCES vertex_property(vertex)
);

DROP TABLE IF EXISTS collapse_vertices;
CREATE TABLE collapse_vertices(
  node_id INTEGER PRIMARY KEY,
  pos INT NOT NULL,
  vertex INT NOT NULL
    REFERENCES vertex_property(vertex)
);

DROP TABLE IF EXISTS collapse_lastround;
CREATE TABLE collapse_lastround(
  node_id INTEGER PRIMARY KEY
    REFERENCES collapse_vertices(node_id),
  lastround INT NOT NULL
);

DROP TABLE IF EXISTS collapse_position;
CREATE TABLE collapse_position(
  node_id INTEGER PRIMARY KEY
    REFERENCES collapse_vertices(node_id),
  position INT NOT NULL
);

DROP TABLE IF EXISTS collapse_edges;
CREATE TABLE collapse_edges(
  edge_id INTEGER PRIMARY KEY,
  src_id INT NOT NULL
    REFERENCES collapse_vertices(node_id),
  dst_id INT NOT NULL
    REFERENCES collapse_vertices(node_id)
);

DROP TABLE IF EXISTS collapse_lhs_edges;
CREATE TABLE collapse_lhs_edges(
  edge_id INTEGER PRIMARY KEY,
  src_id INT NOT NULL
    REFERENCES collapse_vertices(node_id),
  dst_id INT NOT NULL
    REFERENCES collapse_vertices(node_id)
);

DROP TABLE IF EXISTS collapse_rhs_edges;
CREATE TABLE collapse_rhs_edges(
  edge_id INTEGER PRIMARY KEY,
  src_id INT NOT NULL
    REFERENCES collapse_vertices(node_id),
  dst_id INT NOT NULL
    REFERENCES collapse_vertices(node_id)
);

DROP TABLE IF EXISTS collapse_node_index;
CREATE TABLE collapse_node_index(
  node_id INTEGER PRIMARY KEY
    REFERENCES collapse_vertices(node_id),
  seen INT NOT NULL
);
