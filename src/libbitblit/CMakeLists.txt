PROJECT(bitblit C)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/stub-backend)

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
    stub-backend/bit_blit.c
    stub-backend/bit_on.c
    stub-backend/bit_size.c
    stub-backend/bitmap.c
    stub-backend/blit.c
    stub-backend/line.c
    stub-backend/palette.c
    stub-backend/pixel.c
    stub-backend/screen.c
)

SET(bitblit_HEADERS
    stub-backend/bitmap.h
    stub-backend/screen.h
)

ADD_LIBRARY(bitblit
            STATIC
            ${bitblit_SOURCES}
            ${bitblit_HEADERS})
IF(NOT CMAKE_SYSTEM_NAME MATCHES "Emscripten")
  SET_TARGET_PROPERTIES(bitblit PROPERTIES VERSION ${MGR_VERSION})
ENDIF()

INSTALL(TARGETS bitblit DESTINATION lib)
