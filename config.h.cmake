//DO NOT MODIFIY autogen from config.h.cmake

#define BUILD_MODE "${CMAKE_BUILD_TYPE}"

#define PLEXYNAME  "${APPLICATION_NAME}"
#define PLEXYVERSION  "${APPLICATION_MAIN_VERSION}"
#define PLEXYDATE  "${APPLICATION_DATE}"

#define PLEXYPREFIX  "${CMAKE_INSTALL_PREFIX}"
#define PLEXYLIBDIR "${CMAKE_INSTALL_LIBDIR}"

#ifndef PLEXYQTIMPORTSDIR
#define PLEXYQTIMPORTSDIR "${QT_IMPORTS_INSTALL_DIR}"
#endif

#define PLEXYRESOURCESDIR "${RESOURCES_DEST_DIR}"

#define OPENCV_ROOT "${OpenCV_ROOT_DIR}"

#define K_SOCIAL_KIT_GOOGLE_API_KEY "${GOOGLE_API_KEY}"
#define K_SOCIAL_KIT_FACEBOOK_API_KEY "${FACEBOOK_API_KEY}"
#define K_SOCIAL_KIT_DROPBOX_API_KEY "${SOCIAL_KIT_DROPBOX_API_KEY}"
#define K_SOCIAL_KIT_FLICKR_API_KEY "${SOCIAL_KIT_FLICKR_API_KEY}"
#define K_SOCIAL_KIT_OPEN_WEATHER_MAP_API_KEY "${OPEN_WEATHER_MAP_API_KEY}"

#define K_PLEXYDESK_OSX_PLUGIN_PREFIX "${CMAKE_PLEXYDESK_OSX_PLUGIN_PREFIX}"
#define K_PLEXYDESK_OSX_PLUGIN_DATA_DIR "${CMAKE_PLEXYDESK_OSX_PLUGIN_DATA_DIR}"

#define USE_QT 1
