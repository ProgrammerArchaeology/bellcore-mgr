PROJECT(croff C)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE(AddCCompilerFlag)
ADD_C_COMPILER_FLAG(-Wall)

# hash table size (for keywords)
SET(BUCKETS 59)
ADD_DEFINITIONS(-DHSIZE=${BUCKETS})

SET(mktable_SOURCES
    mktable.c
    hash.c
)

SET(mktable_HEADERS
    hash.h
)

ADD_EXECUTABLE(mktable
               ${mktable_SOURCES}
               ${mktable_HEADERS})

FIND_PACKAGE(FLEX)
FLEX_TARGET(CroffScanner croff.l ${CMAKE_CURRENT_BINARY_DIR}/croff.c)

ADD_CUSTOM_COMMAND(
    VERBATIM
    OUTPUT keys.c
		COMMAND mktable -n keywords ${BUCKETS} < ${CMAKE_CURRENT_SOURCE_DIR}/keywords > ${CMAKE_CURRENT_BINARY_DIR}/keys.c
    DEPENDS keywords)

SET(croff_SOURCES
    ${FLEX_CroffScanner_OUTPUTS}
    hash.c
    keys.c
)

SET(croff_HEADERS
    hash.h
)

ADD_EXECUTABLE(croff
               ${croff_SOURCES}
               ${croff_HEADERS})
