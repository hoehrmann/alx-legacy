#include "alx.h"

void
alx_process_one_input_class(
  struct alx_parser_state * const p,
  alx_input_class_id const input_class
) {

  size_t const next_block_begin = p->shadowed_inputs.size;

  uint32_t mask = UINT32_C( 0 );

  for (
    size_t m_ix = p->prev_block_begin;
    m_ix < next_block_begin;
    /**/
  ) {

    // skip duplicates to avoid blowup
    if (
      mask 
      &&
      (alx_v32_at(&p->shadowed_inputs, m_ix - 1) | mask)
      ==
      (alx_v32_at(&p->shadowed_inputs, m_ix))
    ) {

      m_ix += 1;
      continue;

    }

    alx_vertex const* input_vlptr = 
      alx_grammar_vlix_to_vlptr(
        alx_g(p),
        alx_g(p)->nfa_states[
          alx_uint32_lsb31(
            alx_v32_at(
              &p->shadowed_inputs, m_ix) ) ].vertices
      );

    for (; *input_vlptr; ++input_vlptr) {

// alx_debugf(stderr, "p %u\n", *input_vlptr);

      alx_vertex const* successors_vlptr =
        alx_gv_successors_vlptr(
          alx_g(p),
          *input_vlptr
        );

      for (; *successors_vlptr; ++successors_vlptr) {
          
        alx_nfa_state_id const dst_id =
          alx_grammar_nfa_next_state(
            alx_g(p),
            *successors_vlptr,
            input_class
          );
        
        if (dst_id > 0) {
          alx_v32_push_back(
            &p->shadowed_inputs,
            dst_id | mask
          );
          mask = UINT32_C( 1 ) << UINT32_C( 31 );
        }
      }
    }

    m_ix += 1;

  }

  size_t const new_inputs =
    p->shadowed_inputs.size - next_block_begin;

  if (new_inputs == 0) {

    alx_v32_push_back(
      &p->shadowed_inputs,
      0
    );

  } else if (new_inputs > 1) {

    // TODO: Why asc?

    // TODO: this needs a better sorting function
    // optimised for very many but very small arrays

    // NOTE: This is used only for complicated grammars.

    alx_v32_qsort(
      &p->shadowed_inputs,
      next_block_begin,
      new_inputs,
      sizeof(*p->shadowed_inputs.d),
      alx_cmp_uint32_ts_0a
    );

  }

#if 0
  // TODO: make sure the following is not needed. 
  // NOTE: has been commented out for a looong time.
  alx_v32_push_back(&p->shadowed_inputs, 0);
  p->shadowed_inputs.size -= 1;
#endif

  p->prev_block_begin = next_block_begin;

}

