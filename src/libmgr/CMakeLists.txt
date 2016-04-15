PROJECT(mgr C)

ADD_DEFINITIONS(-DHFONTDIR="${CMAKE_SOURCE_DIR}/hfont")

SET(mgr_SOURCES
    ckmgrterm.c
    hfont.c
    is_mgr_term.c
    m_bitc_send.c
    m_bitfile.c
    m_getborder.c
    m_getdepth.c
    m_getevent.c
    m_getfontid.c
    m_getfontn.c
    m_getfonts.c
    m_gethost.c
    m_getpalette.c
    m_getscreen.c
    m_getwindowp.c
    m_getwindows.c
    m_localsrv.c
    m_parse.c
    m_setup.c
    m_tty.c
    oldinfo.c
    term.c
)

SET(mgr_HEADERS
)

ADD_LIBRARY(mgr
            STATIC
            ${mgr_SOURCES}
            ${mgr_HEADERS})
IF(NOT CMAKE_SYSTEM_NAME MATCHES "Emscripten")
  SET_TARGET_PROPERTIES(mgr PROPERTIES VERSION ${MGR_VERSION})
ENDIF()

INSTALL(TARGETS mgr DESTINATION lib)
