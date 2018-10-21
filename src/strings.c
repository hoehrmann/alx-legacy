#include "alx.h"

/////////////////////////////////////////////////////////////////////
// Strings (mainly to produce random match JSON output).
/////////////////////////////////////////////////////////////////////

void
alx_string_free(struct alx_string * const string) {

  if (string->data) {
    free(string->data);
  }

}

struct alx_string
alx_string_new(
  struct alx_parser_state * const p
) {

  struct alx_string string;

  string.capacity = 16*1024;
  string.p = p;
  string.size = 0;
  string.data = NULL;
  string.itoa.mag_len = 0;
  string.itoa.mag_val = 0;
//  string.itoa.mag_buf = { 0 };

  alx_parser_state_memrealloc(
    p,
    (void**)&(string.data),
    sizeof(*string.data) * string.capacity
  );

  for (uint32_t value = 0; value < 1000; ++value) {
    snprintf(&string.itoa.itoa3[ 4 * value], 4, "%03u", value);
  }

  return string;

}

int
alx_string_unsafe_end(
  struct alx_string * const string
) {

  return 0;

}


void
alx_string_unsafe_begin(
  struct alx_string * const string,
  size_t const additional
) {

  if (string->capacity > string->size + additional) {
    return;
  }

  size_t const new_capacity = alx_max_size_t(
    string->capacity * 2, string->capacity + additional);

  alx_parser_state_memrealloc(
    string->p,
    (void**)&(string->data),
    sizeof(*string->data) * new_capacity
  );

  string->capacity = new_capacity;

}

void
alx_string_append_raw(
  struct alx_string * const string,
  char const* const data,
  size_t const size,
  size_t const num
) {

  size_t const total = num * size;
  alx_string_unsafe_begin(string, total);
  memcpy(string->data + string->size, data, total);
  string->size += total;
  alx_string_unsafe_end(string);

}

void
alx_string_append_uint32_unsafe(
  struct alx_string * const string,
  uint32_t const new_val
) {

  // caller must reserve 16 bytes

  char* const out_buf = &string->data[ string->size ];

  if ( new_val < 1000 ) {

    string->size += snprintf(out_buf, 16, "%u", new_val);
    return;

  }

  if (
    (new_val < string->itoa.mag_val)
    ||
    (string->itoa.mag_val + 999 < new_val)
  ) {

    string->itoa.mag_val = 1000 * (new_val / 1000);
    string->itoa.mag_len = snprintf(
      string->itoa.mag_buf,
      16,
      "%u",
      string->itoa.mag_val
    );

  }

  assert( new_val >= string->itoa.mag_val );
  assert( new_val <= string->itoa.mag_val + 999 );

  memcpy(
    out_buf,
    string->itoa.mag_buf,
    string->itoa.mag_len - 3
  );

  memcpy(
    out_buf + string->itoa.mag_len - 3,
    &string->itoa.itoa3[ 4 * (new_val - string->itoa.mag_val) ],
    4
  );

  string->size += string->itoa.mag_len;

}

void
alx_string_append_uint32_unsafe_old(
  struct alx_string * const string,
  uint32_t const tail
) {

  char tmp[12];

  char *buf = tmp + 11;
  *buf-- = 0;

  uint32_t value = tail;

  do {
    *buf-- = '0' + value % 10;
    value /= 10;
  } while (value != 0);

  size_t length = 10 - (buf - tmp);
  memcpy(string->data + string->size, buf + 1, length);
  string->size += length;

}

void
alx_string_json_finish(
  struct alx_string * const string
) {

#if 0
  alx_string_unsafe_begin(string, 1);
  string->data[ 0 ] = '\n';
  string->data[ string->size++ ] = '\n';
  alx_string_unsafe_end(string);
#endif

}

void
alx_string_append_byte(
  struct alx_string * const string,
  uint8_t const byte
) {

  alx_string_unsafe_begin(string, 1);
  string->data[ string->size++ ] = byte;
  alx_string_unsafe_end(string);

}

void
alx_string_append_json_pop(
  struct alx_string * const string,
  size_t const indent,
  char const* const name,
  bool const bracket
) {

  alx_string_unsafe_begin(string, 3);

  if (bracket) {
    string->data[ string->size++ ] = ']';
  }

  string->data[ string->size++ ] = ']';

  alx_string_unsafe_end(string);

}

void
alx_string_append_xml_pop(
  struct alx_string * const string,
  size_t const indent,
  char const* const escaped_name,
  bool const bracket
) {

  if (!*escaped_name) {
    return;
  }

  alx_string_unsafe_begin(
    string,
    3
    +
    alx_grammar_name_length_max_bytes(
      alx_g( string->p )
    )
    +
    2*indent
    +
    1
  );

  string->data[ string->size++ ] = '\n';

  for (size_t ix = 1; ix < indent; ++ix) {
    string->data[ string->size++ ] = ' ';
    string->data[ string->size++ ] = ' ';
  }

  string->data[ string->size++ ] = '<';
  string->data[ string->size++ ] = '/';

  for (size_t ix = 0; escaped_name[ix]; ++ix) {
    string->data[ string->size++ ] = escaped_name[ix];
  }

  string->data[ string->size++ ] = '>';

  alx_string_unsafe_end(string);

}

void
alx_string_append_json_push(
  struct alx_string * const string,
  size_t const indent,
  char const* const escaped_name,
  uint32_t const start_pos,
  uint32_t const final_pos,
  bool const bracket,
  bool const comma
) {

  size_t const max_uint32_length = 16;
  size_t const max_additional = (
    12
    +
    2 * indent
    +
    alx_grammar_name_length_max_bytes(
      alx_g( string->p )
    )
    +
    2 * max_uint32_length
    +
    1
  );

  // TODO: rename function
  alx_string_unsafe_begin(string, max_additional);

  if (comma) {
    string->data[ string->size++ ] = ',';
  }

  string->data[ string->size++ ] = '\n';

  for (size_t ix = 0; ix < indent; ++ix) {
    string->data[ string->size++ ] = ' ';
    string->data[ string->size++ ] = ' ';
  }

  string->data[ string->size++ ] = '[';
  string->data[ string->size++ ] = '"';

  for (size_t ix = 0; escaped_name[ix]; ++ix) {
    string->data[ string->size++ ] = escaped_name[ix];
  }

  string->data[ string->size++ ] = '"';
  string->data[ string->size++ ] = ',';
  string->data[ string->size++ ] = ' ';

  alx_string_append_uint32_unsafe( string, start_pos );

  string->data[ string->size++ ] = ',';
  string->data[ string->size++ ] = ' ';

  alx_string_append_uint32_unsafe( string, final_pos );

  string->data[ string->size++ ] = ',';
  string->data[ string->size++ ] = ' ';

  if (bracket) {
    string->data[ string->size++ ] = '[';
  }

  alx_string_unsafe_end( string );

}

void
alx_string_append_xml_push(
  struct alx_string * const string,
  size_t const indent,
  char const* const escaped_name,
  uint32_t const start_pos,
  uint32_t const final_pos,
  bool const bracket,
  bool const comma
) {

  if (!*escaped_name) {
    return;
  }

  size_t const max_uint32_length = 16;
  size_t const max_additional = (
    8
    +
    2 * indent
    +
    alx_grammar_name_length_max_bytes(
      alx_g( string->p )
    )
    +
    2 * max_uint32_length
    +
    1
  );

  // TODO: rename function
  alx_string_unsafe_begin(string, max_additional);

  string->data[ string->size++ ] = '\n';

  for (size_t ix = 0; ix < indent; ++ix) {
    string->data[ string->size++ ] = ' ';
    string->data[ string->size++ ] = ' ';
  }

  string->data[ string->size++ ] = '<';

  for (size_t ix = 0; escaped_name[ix]; ++ix) {
    string->data[ string->size++ ] = escaped_name[ix];
  }

  string->data[ string->size++ ] = ' ';

  string->data[ string->size++ ] = 'p';
  string->data[ string->size++ ] = '=';

  string->data[ string->size++ ] = '\'';

  alx_string_append_uint32_unsafe( string, start_pos );

  string->data[ string->size++ ] = ',';

  alx_string_append_uint32_unsafe( string, final_pos );

  string->data[ string->size++ ] = '\'';

  // FIXME: cannot close if adding attributes
  string->data[ string->size++ ] = '>';

  alx_string_unsafe_end( string );

}

void
alx_string_append_json_input_span(
  struct alx_string * const string,
  alx_offset const start_pos,
  alx_offset const final_pos
) {

  struct alx_v32 const* const input_unicode = 
    &string->p->input_unicode;

  alx_string_unsafe_begin(
    string,
    2 + 16 * (final_pos - start_pos)
  );

  char buf[16];

  string->data[ string->size++ ] = '"';

  for (size_t ix = start_pos; ix < final_pos; ++ix) {

    uint32_t const cp = alx_v32_at( input_unicode, ix );

    if (

      cp < 0x20
      ||
      cp == '"'
      ||
      cp == '\\'
      ||
      cp == ']'
      ||
      cp == '['
      ||
      cp > 0x7F

    ) {

      if (cp > 0xffff) {

        sprintf(
          buf,
          "\\u%04X\\u%04X",
          (0xD7C0 + (cp >> 10)),
          (0xDC00 + (cp & 0x3FF))
        );

      } else {

        sprintf(
          buf,
          "\\u%04X",
          cp
        );

      }

      alx_string_append_raw(string, buf, strlen(buf), 1);

    } else {

      string->data[ string->size++ ] = cp;

    }

  }

  string->data[ string->size++ ] = '"';

  alx_string_unsafe_end( string );

}

void
alx_string_append_fffd(
  struct alx_string * const string
) {

  // U+FFFD 
  alx_string_append_byte( string, 0xEF );
  alx_string_append_byte( string, 0xBF );
  alx_string_append_byte( string, 0xBD );

}

bool
alx_string_xml_is_char(
  struct alx_string const* const string,
  uint32_t const cp
) {

  // TODO: name not ideal given that U+FFFD _is_ xml char

  assert( cp <= 0x10FFFF );

  if (
    cp == 0xFFFE
    ||
    cp == 0xFFFF
    ||
    (cp >= 0xD800 && cp <= 0xDFFF)
    ||
    cp < 0x09
    ||
    cp == 0x0B
    ||
    cp == 0x0C
    ||
    (cp >= 0x0E && cp < 0x20)
    ||
    cp == 0xFFFD
  ) {
    return false;
  }

  return true;

}

bool
alx_string_xml_is_escapee(
  struct alx_string const* const string,
  uint32_t const cp
) {

  if (
    cp == 0x0D
    ||
    cp == 0x0A
    ||
    cp == 0x09
    ||
    cp == '<'
    ||
    cp == '>'
    ||
    cp == '&'
    ||
    cp == '\''
    ||
    cp > 0x7F
  ) {

    return true;

  }

  return false;
  
}

void
alx_string_append_xml_input_span(
  struct alx_string * const string,
  alx_offset const start_pos,
  alx_offset const final_pos
) {

  struct alx_v32 const* const input_unicode = 
    &string->p->input_unicode;

  alx_string_unsafe_begin(
    string,
    8 + 16 * (final_pos - start_pos)
  );

  char buf[16];

  string->data[ string->size++ ] = ' ';
  string->data[ string->size++ ] = 'd';
  string->data[ string->size++ ] = 'a';
  string->data[ string->size++ ] = 't';
  string->data[ string->size++ ] = 'a';
  string->data[ string->size++ ] = '=';
  string->data[ string->size++ ] = '\'';

  for (size_t ix = start_pos; ix < final_pos; ++ix) {

    uint32_t const cp = alx_v32_at( input_unicode, ix );

    if (!alx_string_xml_is_char(string, cp)) {

      alx_string_append_fffd( string );

      // TODO: store actual value in extra attribute
      // fffd='#xXXXXXX'

    } else if (alx_string_xml_is_escapee(string, cp)) {

      sprintf(
        buf,
        "&#x%X;",
        cp
      );

      alx_string_append_raw(string, buf, strlen(buf), 1);

    } else {

      string->data[ string->size++ ] = cp;

    }

  }

  string->data[ string->size++ ] = '\'';

  alx_string_unsafe_end( string );

}

void
alx_string_append_json_data(
  struct alx_string * const string,
  size_t const indent,
  char const* const escaped_name,
  uint32_t const start_pos,
  uint32_t const final_pos,
  bool const bracket,
  bool const comma
) {

  alx_string_append_json_push(
    string,
    indent,
    escaped_name,
    start_pos,
    final_pos,
    bracket,
    comma
  );

  alx_string_append_json_input_span(string, start_pos, final_pos);

  alx_string_append_json_pop(string, indent+1, escaped_name, false);

}

void
alx_string_append_xml_data(
  struct alx_string * const string,
  size_t const indent,
  char const* const escaped_name,
  uint32_t const start_pos,
  uint32_t const final_pos,
  bool const bracket,
  bool const comma
) {

  alx_string_append_xml_push(
    string,
    indent,
    escaped_name,
    start_pos,
    final_pos,
    bracket,
    comma
  );

  // remove >
  string->size--;

  alx_string_append_xml_input_span(string, start_pos, final_pos);

  alx_string_append_byte( string, '/' );
  alx_string_append_byte( string, '>' );

}
