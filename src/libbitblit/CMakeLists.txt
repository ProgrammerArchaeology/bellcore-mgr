PROJECT(bitblit C)

SET(MGR_BITBLIT_BACKEND "stub-backend" CACHE STRING "libbitblit backend renderer")
SET_PROPERTY(CACHE MGR_BITBLIT_BACKEND PROPERTY STRINGS stub-backend sdl-backend)
MESSAGE(STATUS "bitblit-backend=${MGR_BITBLIT_BACKEND}")

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
)

SET(bitblit_HEADERS
)

IF(MGR_BITBLIT_BACKEND MATCHES "stub-backend")
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/stub-backend)
  LIST(APPEND bitblit_SOURCES
       stub-backend/bit_blit.c
       stub-backend/bit_on.c
       stub-backend/bit_size.c
       stub-backend/bitmap.c
       stub-backend/blit.c
       stub-backend/line.c
       stub-backend/palette.c
       stub-backend/pixel.c
       stub-backend/screen.c)
  LIST(APPEND bitblit_HEADERS
       stub-backend/bitmap.h
       stub-backend/screen.h)
ENDIF()

ADD_LIBRARY(bitblit
            STATIC
            ${bitblit_SOURCES}
            ${bitblit_HEADERS})
IF(NOT CMAKE_SYSTEM_NAME MATCHES "Emscripten")
  SET_TARGET_PROPERTIES(bitblit PROPERTIES VERSION ${MGR_VERSION})
ENDIF()

INSTALL(TARGETS bitblit DESTINATION lib)
