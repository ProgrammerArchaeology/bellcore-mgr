PROJECT(bitblit C)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/stub_lib)

SET(bitblit_SOURCES
    bitmap/bit_alloc.c
    bitmap/bit_create.c
    bitmap/bit_open.c
    bitmap/bitmaphead.c
    movie/share.c
    movie/timestamp.c
    seqbitmap/bit_load.c
    seqbitmap/bit_save.c
    seqbitmap/bitmapread.c
    seqbitmap/bitmapwrite.c
    stub_lib/bit_blit.c
    stub_lib/bit_on.c
    stub_lib/bit_size.c
    stub_lib/bitmap.c
    stub_lib/blit.c
    stub_lib/line.c
    stub_lib/palette.c
    stub_lib/pixel.c
    stub_lib/screen.c
)

SET(bitblit_HEADERS
    stub_lib/bitmap.h
    stub_lib/screen.h
)

ADD_LIBRARY(bitblit
            STATIC
            ${bitblit_SOURCES}
            ${bitblit_HEADERS})
IF(NOT CMAKE_SYSTEM_NAME MATCHES "Emscripten")
  SET_TARGET_PROPERTIES(bitblit PROPERTIES VERSION ${MGR_VERSION})
ENDIF()

INSTALL(TARGETS bitblit DESTINATION lib)
