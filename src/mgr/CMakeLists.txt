PROJECT(mgr-server C)

SET(DEFAULT_FONT ${CMAKE_SOURCE_DIR}/font/is8859-1-8x14)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}) # For default_font.h, icon_server.h, version.h
ADD_DEFINITIONS(-DFONTDIR="${CMAKE_SOURCE_DIR}/font")
ADD_DEFINITIONS(-DICONDIR="${CMAKE_SOURCE_DIR}/icon")
ADD_DEFINITIONS(-DMOUSE_DEV="fix-me")
ADD_DEFINITIONS(-DSCREEN_DEV="fix-me")
ADD_DEFINITIONS(-DTERMNAME="mgr")

OPTION(MGR_ENABLE_BUCKEY "Keyboard operated 'mouse' commands." OFF)
IF(MGR_ENABLE_BUCKEY)
  ADD_DEFINITIONS(-DBUCKEY)
ENDIF()

OPTION(MGR_ENABLE_CUT "Enable cut/paste between screen and snarfbuffer." OFF)
IF(MGR_ENABLE_CUT)
  ADD_DEFINITIONS(-DCUT)
ENDIF()

OPTION(MGR_ENABLE_DEBUG "Enable debugging code (for -d option)." OFF)
IF(MGR_ENABLE_DEBUG)
  ADD_DEFINITIONS(-DDEBUG)
ENDIF()

OPTION(MGR_ENABLE_EMUMIDMSBUT "Fake a middle mouse button with left and right button." OFF)
IF(MGR_ENABLE_EMUMIDMSBUT)
  ADD_DEFINITIONS(-DEMUMIDMSBUT)
ENDIF()

OPTION(MGR_ENABLE_EXTENDED_MENU "Extended menu operations." OFF)
IF(MGR_ENABLE_EXTENDED_MENU)
  ADD_DEFINITIONS(-DXMENU)
ENDIF()

OPTION(MGR_ENABLE_STRETCH "Mouse reshaping windows by stretching a corner." OFF)
IF(MGR_ENABLE_STRETCH)
  ADD_DEFINITIONS(-DSTRETCH)
ENDIF()

OPTION(MGR_ENABLE_THIN_BORDER "Enable thin three pixel wide window borders for low-res." OFF)
IF(MGR_ENABLE_THIN_BORDER)
  ADD_DEFINITIONS(-DSUM_BDR=3)
ENDIF()

ADD_CUSTOM_COMMAND(
    OUTPUT default_font.h
    COMMAND fonttoc < ${DEFAULT_FONT} default_font > default_font.h
    DEPENDS ${DEFAULT_FONT})

SET(mgr_server_ICONS
    ${CMAKE_SOURCE_DIR}/icon/server/ball_1
    ${CMAKE_SOURCE_DIR}/icon/server/ball_2
    ${CMAKE_SOURCE_DIR}/icon/server/ball_3
    ${CMAKE_SOURCE_DIR}/icon/server/ball_4
    ${CMAKE_SOURCE_DIR}/icon/server/ball_5
    ${CMAKE_SOURCE_DIR}/icon/server/ball_6
    ${CMAKE_SOURCE_DIR}/icon/server/ball_7
    ${CMAKE_SOURCE_DIR}/icon/server/ball_8
    ${CMAKE_SOURCE_DIR}/icon/server/cr
    ${CMAKE_SOURCE_DIR}/icon/server/def_pattern
    ${CMAKE_SOURCE_DIR}/icon/server/mouse_arrow
    ${CMAKE_SOURCE_DIR}/icon/server/mouse_box
    ${CMAKE_SOURCE_DIR}/icon/server/mouse_bull
    ${CMAKE_SOURCE_DIR}/icon/server/mouse_bull2
    ${CMAKE_SOURCE_DIR}/icon/server/mouse_cross
    ${CMAKE_SOURCE_DIR}/icon/server/mouse_cup
    ${CMAKE_SOURCE_DIR}/icon/server/mouse_cut
)
ADD_CUSTOM_COMMAND(
    VERBATIM
    OUTPUT icon_server.c
		COMMAND ${CMAKE_COMMAND} -E echo "/* bitmaps used by mgr server, file is auto-generated */" > icon_server.c
		COMMAND ${CMAKE_COMMAND} -E echo "#include <mgr/bitblit.h>" >> icon_server.c
		COMMAND bitmaptoc ${mgr_server_ICONS} >> icon_server.c
    DEPENDS ${mgr_server_ICONS})

SET(icon_server_CONTENT)
STRING(APPEND icon_server_CONTENT "/* bitmaps used by mgr server, file is auto-generated */\n")
STRING(APPEND icon_server_CONTENT "\n")
FOREACH(server_icon ${mgr_server_ICONS})
  GET_FILENAME_COMPONENT(server_icon_name ${server_icon} NAME)
  STRING(APPEND icon_server_CONTENT "extern BITMAP ${server_icon_name};\n")
ENDFOREACH()
FILE(GENERATE
     OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/icon_server.h
     CONTENT "${icon_server_CONTENT}")

ADD_CUSTOM_COMMAND(
    VERBATIM
    OUTPUT version.h
		COMMAND ${CMAKE_COMMAND} -E echo "/* compilation info for mgr, file is auto-generated */" > version.h
		COMMAND ${CMAKE_COMMAND} -E echo >> version.h
		COMMAND ${CMAKE_COMMAND} -E echo "char version[]=\"MGR version ${MGR_VERSION}\\n\";" >> version.h)


SET(mgr_server_SOURCES
    Write.c
    border.c
    colormap.c
    copyright.c
    cut.c
    data.c
    debug_flags.c
    destroy.c
    do_buckey.c
    do_button.c
    do_event.c
    do_menu.c
    down_load.c
    erase_win.c
    font_subs.c
    get_font.c
    get_info.c
    get_menus.c
    get_rect.c
    get_text.c
    getshell.c
    graph_subs.c
    icon_server.c
    intersect.c
    kbd.c
    mgr.c
    mouse.c
    mouse_get.c
    move.c
    move_box.c
    new_window.c
    popen.c
    print.c
    put_window.c
    scroll.c
    set_mode.c
    shape.c
    sigdata.c
    startup.c
    subs.c
    update.c
    utmp.c
    win_make.c
    win_stack.c
    win_subs.c
    write_ok.c
)

SET(mgr_server_HEADERS
    Write.h
    border.h
    clip.h
    colormap.h
    copyright.h
    cut.h
    default_font.h
    defs.h
    destroy.h
    do_buckey.h
    do_button.h
    do_event.h
    do_menu.h
    down_load.h
    erase_win.h
    event.h
    font_subs.h
    get_font.h
    get_info.h
    get_menus.h
    get_rect.h
    get_text.h
    getshell.h
    graph_subs.h
    icon_server.h
    intersect.h
    kbd.h
    menu.h
    mgr.h
    mouse.h
    mouse_get.h
    move.h
    move_box.h
    new_window.h
    print.h
    proto.h
    put_window.h
    scroll.h
    set_mode.h
    shape.h
    sigdata.h
    startup.h
    subs.h
    update.h
    utmp.h
    version.h
    win_make.h
    win_stack.h
    win_subs.h
    write_ok.h
)

ADD_EXECUTABLE(mgr-server
               ${mgr_server_SOURCES}
               ${mgr_server_HEADERS})
TARGET_LINK_LIBRARIES(mgr-server bitblit mgr)
IF(NOT CMAKE_SYSTEM_NAME MATCHES "Emscripten")
  SET_TARGET_PROPERTIES(mgr-server PROPERTIES VERSION ${MGR_VERSION})
ENDIF()
INSTALL(TARGETS mgr-server DESTINATION bin)

SET(mgrlogin_SOURCES
    mgrlogin.c
    font_subs.c
    set_mode.c
)

SET(mgrlogin_HEADERS
    font_subs.h
    set_mode.h
)

ADD_EXECUTABLE(mgrlogin
               ${mgrlogin_SOURCES}
               ${mgrlogin_HEADERS})
TARGET_LINK_LIBRARIES(mgrlogin bitblit)
TARGET_COMPILE_DEFINITIONS(mgrlogin PRIVATE
                           -DMGRLOGIN
                           -DMGR_BINARY="fix-me")
IF(NOT CMAKE_SYSTEM_NAME MATCHES "Emscripten")
  SET_TARGET_PROPERTIES(mgrlogin PROPERTIES VERSION ${MGR_VERSION})
ENDIF()
INSTALL(TARGETS mgrlogin DESTINATION bin)
