set(FILES_in_install_path
    ${APPLICATION_EXE_NAME}.ico
    ${CMAKE_SOURCE_DIR}/COPYING
    )

INSTALL(FILES qt.conf DESTINATION bin)

INSTALL(FILES ${FILES_in_install_path} DESTINATION ./)
