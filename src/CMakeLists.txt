cmake_minimum_required (VERSION 3.0)

project (alx)

SET_SOURCE_FILES_PROPERTIES(
  alx.c
  grammar.c
  ext/sqlite3.c
    PROPERTIES LANGUAGE C
)

SET_SOURCE_FILES_PROPERTIES(
  quadindex.cxx
    PROPERTIES LANGUAGE CXX
)

add_executable(alx 

  alx.c 
  grammar.c
  ext/sqlite3.c 
  quadindex.cxx
)

# "relocation R_X86_64_32S against symbol `global_grammar' can
# not be used when making a PIE object; recompile with -fPIC".
# TODO(bh): unclear why this happens only for Ubuntu on Windows?
# Unsure if that would not actually slow things down elsehwere.
set_property(TARGET alx PROPERTY POSITION_INDEPENDENT_CODE ON)

add_definitions(
  -DSQLITE_THREADSAFE=0
  -DSQLITE_ENABLE_JSON1=1
)

# FIXME: find a better way to do this
if (NOT WIN32)
  # set_property(TARGET alx PROPERTY C_STANDARD 11)
  # set_property(TARGET alx PROPERTY CXX_STANDARD 11)
  # The options above use -std=gnu11 which is not intended.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")

  # TODO: Check SQLite compile time options to avoid these?
  target_link_libraries(alx
    dl pthread
  )

  add_compile_options(
    -Wall -Wextra -pedantic
  )

  target_compile_options(alx
    PUBLIC "$<$<CONFIG:RELWITHDEBINFO>:-O3>"
    PUBLIC "$<$<CONFIG:RELEASE>:-O3>"
    )
endif (NOT WIN32)

if (EMSCRIPTEN)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
    -s NODERAWFS=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s SINGLE_FILE=1 \
    -s ASSERTIONS=1 \
    -s NO_EXIT_RUNTIME=0 \
  ")
endif()
