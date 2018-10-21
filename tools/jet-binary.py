#!/usr/bin/env python3
import xml.sax
import json
from pprint import pprint
import os
import logging
import sqlite3 as db
import re

filedir = os.path.abspath(os.path.dirname(__file__))
'''
  pylov/Pattern.py
  pylov/Graph.py
  pylov/Converter.py
  pylov/Simplify.py
  pylov/XmlHandler.py

'''


class Simplify(object):

  def _ranges(self, p):
    pass

  def _ascii_insensitive_string(self, p):
    pass

  def _case_sensitive_string(self, p):
    pass

  def _optional(self, p):
    return ['choice', {}, [['empty', {}, []], Pattern.p(p)]]

  def _lazy_optional(self, p):
    return ['orderedChoice', {}, [['empty', {}, []], Pattern.p(p)]]

  def _greedy_optional(self, p):
    return ['orderedChoice', {}, [Pattern.p(p), ['empty', {}, []]]]

  def _zero_or_more(self, p):
    return [
      'oneOrMore',
      {},
      self._optional(['optional',
                      {},
                      Pattern.p(p)])
    ]

  def _lazy_zero_or_more(self, p):
    return [
      'oneOrMore',
      {},
      self._lazy_optional(['lazyOptional',
                           {},
                           Pattern.p(p)])
    ]

  def _greedy_zero_or_more(self, p):
    return [
      'oneOrMore',
      {},
      self._greedy_optional(['greedyOptional',
                           {},
                           Pattern.p(p)])
    ]

class Pattern(object):

  """
  ...
  """

  _arity = {
    'grammar': {
      'indy': False,
      'arity': None,
      'wrapper': None
    },
    'ranges': {
      'indy': False,
      'arity': None,
      'wrapper': None
    },
    'repetition': {
      'indy': False,
      'arity': 1,
      'wrapper': 'group'
    },
    'optional': {
      'indy': False,
      'arity': 1,
      'wrapper': 'group'
    },
    'option': {
      'indy': False,
      'arity': 1,
      'wrapper': 'group'
    },
    'greedyOptional': {
      'indy': False,
      'arity': 1,
      'wrapper': 'group'
    },
    'lazyOptional': {
      'indy': False,
      'arity': 1,
      'wrapper': 'group'
    },
    'zeroOrMore': {
      'indy': False,
      'arity': 1,
      'wrapper': 'group'
    },
    'greedyZeroOrMore': {
      'indy': False,
      'arity': 1,
      'wrapper': 'group'
    },
    'lazyZeroOrMore': {
      'indy': False,
      'arity': 1,
      'wrapper': 'group'
    },
    'oneOrMore': {
      'indy': False,
      'arity': 1,
      'wrapper': 'group'
    },
    'greedyOneOrMore': {
      'indy': False,
      'arity': 1,
      'wrapper': 'group'
    },
    'lazyOneOrMore': {
      'indy': False,
      'arity': 1,
      'wrapper': 'group'
    },
    'group': {
      'indy': False,
      'arity': 2,
      'wrapper': 'group'
    },
    'rule': {
      'indy': True,
      'arity': 1,
      'wrapper': 'group'
    },
    'define': {
      'indy': True,
      'arity': 1,
      'wrapper': 'group'
    },
    'exclusion': {
      'indy': True,
      'arity': 2,
      'wrapper': 'group'
    },
    'choice': {
      'indy': True,
      'arity': 2,
      'wrapper': 'choice'
    },
    'conjunction': {
      'indy': True,
      'arity': 2,
      'wrapper': 'conjunction'
    },
    'orderedChoice': {
      'indy': True,
      'arity': 2,
      'wrapper': 'orderedChoice'
    },
    'orderedConjunction': {
      'indy': True,
      'arity': 2,
      'wrapper': 'orderedConjunction'
    },
    'ref': {
      'indy': False,
      'arity': 0,
      'wrapper': None
    },
    'range': {
      'indy': False,
      'arity': 0,
      'wrapper': None
    },
    'asciiInsensitiveString': {
      'indy': False,
      'arity': 0,
      'wrapper': None
    },
    'string': {
      'indy': False,
      'arity': 0,
      'wrapper': None
    },
    'empty': {
      'indy': False,
      'arity': 0,
      'wrapper': None
    },
    'notAllowed': {
      'indy': False,
      'arity': 0,
      'wrapper': None
    }
  }

  @staticmethod
  def p(p):
    return p[2][0]

  @staticmethod
  def p1(p):
    return p[2][0]

  @staticmethod
  def p2(p):
    return p[2][1]

  @staticmethod
  def min(p):
    return p[1]['min']

  @staticmethod
  def max(p):
    return p[1]['max']

  @staticmethod
  def first(p):
    return p[1]['first']

  @staticmethod
  def last(p):
    return p[1]['last']

  @staticmethod
  def name(p):
    return p[1]['name']

  @staticmethod
  def type(p):
    return p[0]

  @staticmethod
  def wrapper(p):
    return Pattern._arity[Pattern.type(p)]['wrapper']

  @staticmethod
  def arity(p):
    return Pattern._arity[Pattern.type(p)]['arity']


class Converter(object):

  @staticmethod
  def correct_arity(elem):

    log = logging.getLogger()

    name, attr, kids = elem

    # if name not in self._arity:
    #   self._log.error('Unknown element type %s', name)
    #   raise NotImplementedError('Unknown element type %s' % name)

    limit = Pattern.arity(elem)
    wrapper = Pattern.wrapper(elem)

    if limit is None:
      log.debug('No arity correction for %s', name)
      return

    if len(kids) <= limit:
      log.debug('Arity for %s within limits', name)
      return

    if wrapper is None:
      log.error('Element %s cannot have children', name)
      raise NotImplementedError()

    # assert wrapper in self._arity
    assert Pattern.arity([wrapper, {}, []]) >= 2

    while len(kids) > limit:

      log.debug('Wrapping %s children in %s', name, wrapper)

      snd = kids.pop()
      fst = kids.pop()

      kids.append([wrapper, {}, [fst, snd]])

  def convert(self, g, p, after):
    pass

  def _create(self, g, s):

    expr = r'(?P<src>\S+)\s*->\s*(?P<dst>\S+)'

    vm = {}

    for m in re.finditer(expr, s):
      vm.setdefault(m.group('src'))
      vm.setdefault(m.group('dst'))

    # TODO: create vertex for each

    for m in re.finditer(expr, s):
      g.add_edge(vm[m.group('src')], vm[m.group('dst')])

    return vm

  def _convert_ref(self, g, p, after):

    d = self._create(g, '''
      s -> r
      r -> f
    ''')

    g.vp_type(d['r'], 'Reference')
    g.vp_name(d['r'], Pattern.name(p))

    return d['s'], d['f']

  def _convert_binary_operation(self, g, p, after, op):

    p1s, p1f = self.convert(g, Pattern.p1(p), after)
    p2s, p2f = self.convert(g, Pattern.p2(p), after)

    d = self._create(
      g,
      '''
      s -> if
      if -> if1
      if -> if2
      if1 -> %(p1s)s
      if2 -> %(p2s)s
      %(p1f)s -> fi1
      %(p2f)s -> fi2
      fi1 -> fi
      fi2 -> fi
      fi -> f
      ''' % dict(
        p1s=p1s,
        p1f=p1f,
        p2s=p2s,
        p2f=p2f,
      )
    )

    g.vp_type(d['if'], 'If')
    g.vp_type(d['if1'], 'If1')
    g.vp_type(d['if2'], 'If2')
    g.vp_type(d['fi1'], 'Fi1')
    g.vp_type(d['fi2'], 'Fi2')
    g.vp_type(d['fi'], 'Fi')
    g.vp_type(d['s'], 'Empty')
    g.vp_type(d['f'], 'Empty')

    g.vp_name(d['if'], op)
    g.vp_name(d['fi'], op)

    g.link(d['if'], d['fi'])
    g.link(d['if1'], d['fi1'])
    g.link(d['if2'], d['fi2'])

    return d['s'], d['f']

  def _convert_choosy_one_or_more(self, g, p, after, op):

    pds, pdf = self.convert(g, p, after)

    d = self._create(
      g,
      '''
      s -> %(startX)s
      %(finalX)s -> if
      if -> if1
      if -> if2
      if1 -> %(startX)s
      if2 -> startAfter
      fi1 -> fi
      fi2 -> fi
      finalAfter -> fi1
      finalAfter -> fi2
      fi -> fi1
      fi -> fi2
      fi -> f
      ''' % dict(
        startX=pds,
        finalX=pdf,
      )
    )

    if op.startswith('lazy') or op.startswith('greedy'):

      g.vp_name(d['if'], '#ordered_choice')
      g.vp_name(d['fi'], '#ordered_choice')

      g.vp_type(d['if'], 'If')
      g.vp_type(d['if1'], 'If1')
      g.vp_type(d['if2'], 'If2')
      g.vp_type(d['fi1'], 'Fi1')
      g.vp_type(d['fi2'], 'Fi2')
      g.vp_type(d['fi'], 'Fi')
      g.vp_type(d['s'], 'Empty')
      g.vp_type(d['f'], 'Empty')

      g.link(d['if'], d['fi'])
      g.link(d['if1'], d['fi1'])
      g.link(d['if2'], d['fi2'])

    after.append([d['finalAfter'], d['final']])

    # return is a problem


class Graph(object):

  def _deploy_app_id(self):
    # TODO: set pragmas application_id and user_version
    pass

  def _deploy_schema(self):

    self._dbh.executescript(
      '''

      ---------------------------------------------------------------
      -- Pragmata
      ---------------------------------------------------------------

      PRAGMA foreign_keys = ON; -- TODO: initially off, turn on later?
                                -- problem is fk errors when dropping.
      PRAGMA synchronous  = OFF;
      PRAGMA journal_mode = OFF;
      PRAGMA locking_mode = EXCLUSIVE;

      ---------------------------------------------------------------
      -- Graph
      ---------------------------------------------------------------

      -- TODO: limit to 1 row?
      DROP TABLE IF EXISTS graph_property;
      CREATE TABLE graph_property(
        start_vertex
          REFERENCES Vertex(vertex)
            ON DELETE CASCADE
            ON UPDATE CASCADE,
        final_vertex
          REFERENCES Vertex(vertex)
            ON DELETE CASCADE
            ON UPDATE CASCADE
      );
      
      ---------------------------------------------------------------
      -- Vertices
      ---------------------------------------------------------------

      DROP TABLE IF EXISTS Vertex;
      CREATE TABLE Vertex(
        vertex INT UNIQUE NOT NULL
      );

      ---------------------------------------------------------------
      -- Edges
      ---------------------------------------------------------------

      DROP TABLE IF EXISTS Edge;
      CREATE TABLE Edge(
        src INT NOT NULL,
        dst INT NOT NULL,
        UNIQUE(src, dst),
        FOREIGN KEY (src)
          REFERENCES Vertex(vertex)
          ON DELETE CASCADE
          ON UPDATE CASCADE
        FOREIGN KEY (dst)
          REFERENCES Vertex(vertex)
          ON DELETE CASCADE 
          ON UPDATE CASCADE
      );

      CREATE INDEX idx_Edge_dst
        ON Edge (dst);

      -----------------------------------------------------------------
      -- Triggers that add vertices and edges when needed elsewhere
      -----------------------------------------------------------------

      DROP TRIGGER IF EXISTS trigger_Edge_insert;
      CREATE TRIGGER trigger_Edge_insert
        BEFORE INSERT ON Edge
        BEGIN
          INSERT OR IGNORE
          INTO Vertex(vertex)
          VALUES(NEW.src);

          INSERT OR IGNORE
          INTO Vertex(vertex)
          VALUES(NEW.dst);
        END;

      -----------------------------------------------------------------
      -- vertex_property
      -----------------------------------------------------------------

      DROP TABLE IF EXISTS vertex_property;
      CREATE TABLE vertex_property (
        vertex INT PRIMARY KEY UNIQUE NOT NULL
          REFERENCES Vertex(vertex)
            ON UPDATE CASCADE
            ON DELETE CASCADE,
        type NOT NULL DEFAULT 'empty',
        name,
        p1 INT
          REFERENCES Vertex(vertex)
            ON UPDATE CASCADE
            ON DELETE CASCADE,
        p2 INT
          REFERENCES Vertex(vertex)
            ON UPDATE CASCADE
            ON DELETE CASCADE,
        partner
          REFERENCES Vertex(vertex)
            ON UPDATE CASCADE
            ON DELETE CASCADE,
        run_list,
        self_loop
          DEFAULT 'no',
        contents_self_loop
          DEFAULT 'no',
        topo INT,
        skippable,
        epsilon_group,
        shadow_group,
        stack_group
          REFERENCES Vertex(vertex)
            ON UPDATE CASCADE
            ON DELETE CASCADE,
        representative
          REFERENCES Vertex(vertex)
            ON UPDATE CASCADE
            ON DELETE CASCADE
      );

      DROP VIEW IF EXISTS view_vp_plus;
      CREATE VIEW view_vp_plus AS 
      SELECT
        vertex,
        type,
        name,
        p1,
        p2,
        partner,
        run_list,
        self_loop,
        contents_self_loop,
        topo,
        skippable,
        epsilon_group,
        shadow_group,
        stack_group,
        representative,
        CAST( type IN (
          'Start', 'If', 'If1', 'If2',
          'Final', 'Fi', 'Fi1', 'Fi2'
        ) AS INT) AS is_stack,
        CAST(type IN ('Start', 'If', 'If1', 'If2') AS INT) AS is_push,
        CAST(type IN ('Final', 'Fi', 'Fi1', 'Fi2') AS INT) AS is_pop,
        CAST( type IN (
          'If', 'If1', 'If2',
          'Fi', 'Fi1', 'Fi2'
        ) AS INT) AS is_conditional
      FROM
        vertex_property
      ;

      DROP TABLE IF EXISTS vertex_shadows;
      CREATE TABLE vertex_shadows(
        vertex INT
          REFERENCES Vertex(vertex)
            ON UPDATE CASCADE
            ON DELETE CASCADE,
        shadows INT
          REFERENCES Vertex(vertex)
            ON UPDATE CASCADE
            ON DELETE CASCADE,
        UNIQUE(vertex, shadows)
      );

      CREATE INDEX idx_vertex_shadows_shadows
        ON vertex_shadows(shadows);

    '''
    )

  def __init__(self):
    self._dbh = db.connect('DEBUG.sqlite')
    self._deploy_app_id()
    self._deploy_schema()


class JetHandler(xml.sax.ContentHandler):

  def __init__(self):

    self._stack = [["", {}, []]]
    self._log = logging.getLogger()

  def startElement(self, name, attrs):

    elem = [name, dict(attrs), []]

    self._stack[-1][2].append(elem)
    self._stack.append(elem)

  def endElement(self, name):

    popped = self._stack.pop()

    assert popped[0] == name
    assert self._stack[-1][2][-1] == popped

    Converter.correct_arity(popped)
    # Converter.simplify_shallow( popped )


p = xml.sax.make_parser()
h = JetHandler()

p.setContentHandler(h)
p.parse(open("xxx", "r"))

# pprint(h._stack[0])

g = Graph()
