PROJECT(bitmaptoc C)

SET(bitmaptoc_SOURCES
    bitmaptoc.c
)

ADD_EXECUTABLE(bitmaptoc
               ${bitmaptoc_SOURCES})
TARGET_LINK_LIBRARIES(bitmaptoc bitblit)
IF(NOT CMAKE_SYSTEM_NAME MATCHES "Emscripten")
  SET_TARGET_PROPERTIES(bitmaptoc PROPERTIES VERSION ${MGR_VERSION})
ENDIF()
INSTALL(TARGETS bitmaptoc DESTINATION bin)

SET(fonttoc_SOURCES
    fonttoc.c
)

ADD_EXECUTABLE(fonttoc
               ${fonttoc_SOURCES})
TARGET_LINK_LIBRARIES(fonttoc bitblit)
IF(NOT CMAKE_SYSTEM_NAME MATCHES "Emscripten")
  SET_TARGET_PROPERTIES(fonttoc PROPERTIES VERSION ${MGR_VERSION})
ENDIF()
INSTALL(TARGETS fonttoc DESTINATION bin)
