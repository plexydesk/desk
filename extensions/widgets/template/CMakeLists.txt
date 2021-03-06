set(CMAKE_AUTOMOC ON)

set(sourceFiles
    template.cpp
    templateinterface.cpp
    )

set(headerFiles
    template.h
    templateinterface.h
    )

set(QTMOC_SRCS
    template.h
    templateinterface.h
    )

if (ENABLE_QT5)
QT5_WRAP_CPP(QT_MOC_SRCS ${QTMOC_SRCS})
endif()

set(sourceFiles
    ${sourceFiles}
    ${headerFiles}
    )

set(libs
    ${QT_QTCORE_LIBRARY}
    )

ADD_LIBRARY(templatewidget SHARED ${sourceFiles})

if (MINGW)
    SET_TARGET_PROPERTIES(templatewidget PROPERTIES
        IMPORT_SUFFIX ".lib"
        IMPORT_PREFIX ""
        PREFIX ""
        )
endif(MINGW)

TARGET_LINK_LIBRARIES(templatewidget
    ${PLEXY_UI_CORE_LIBRARY}
    ${libs}
    )

if (ENABLE_QT5)
QT5_USE_MODULES(templatewidget Widgets Xml Core  )
endif()

if (NOT APPLE)
    INSTALL(TARGETS templatewidget DESTINATION ${CMAKE_INSTALL_LIBDIR}/plexyext)
    INSTALL(FILES template.desktop DESTINATION share/plexy/ext/groups)
endif (NOT APPLE)

if (APPLE)
   INSTALL(TARGETS templatewidget LIBRARY DESTINATION ${CMAKE_PLEXYDESK_OSX_PLUGIN_PREFIX} COMPONENT Framework)
   INSTALL(FILES template.desktop DESTINATION ${CMAKE_PLEXYDESK_OSX_PLUGIN_DATA_DIR})
endif(APPLE)
