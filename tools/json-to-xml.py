import ijson
from xml.sax.saxutils import XMLGenerator
import io
import sys
import json

parser = ijson.basic_parse(open('delme'))

out = io.StringIO()

g = XMLGenerator(out, 'utf-8')
g.startDocument()

g.processingInstruction('alx', json.dumps({
  'version': 0
}).replace('?>', '?\\u003E'))

stack = []

for event, value in parser:

  if event == 'start_array':
    _, name = next(parser)
    _, spos = next(parser)
    _, fpos = next(parser)

    name = name if ( name != '#' ) else '_'
    name = name if ( name != '' ) else None

    attrs = {
      'start_pos': str(spos),
      'final_pos': str(fpos)
    }

    child_type, child_data = next(parser)

    if child_type == 'start_array':
      stack.append( None )
    else:
      attrs['matched'] = child_data

    if name is not None:
      g.startElement(name, attrs)

    stack.append( name )

  if event == 'end_array':
    popped = stack.pop()

    if popped is not None:
      g.endElement(popped)

g.endDocument()

print( out.getvalue() )

