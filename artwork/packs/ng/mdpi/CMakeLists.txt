set(icon_dpi "mdpi")

file(GLOB action_icons actions/*.png)
file(GLOB toolbar_icons toolbar/*.png)
file(GLOB navigation_icons navigation/*.png)
file(GLOB panel_icons panel/*.png)
file(GLOB desktop_icons desktop/*.png)
file(GLOB branding_icons branding/*.png)


install(FILES ${action_icons} DESTINATION ${RESOURCES_DEST_DIR}/ng/${icon_dpi}/actions/)
install(FILES ${toolbar_icons} DESTINATION ${RESOURCES_DEST_DIR}/ng/${icon_dpi}/toolbar/)
install(FILES ${navigation_icons} DESTINATION ${RESOURCES_DEST_DIR}/ng/${icon_dpi}/navigation/)
install(FILES ${panel_icons} DESTINATION ${RESOURCES_DEST_DIR}/ng/${icon_dpi}/panel/)
install(FILES ${desktop_icons} DESTINATION ${RESOURCES_DEST_DIR}/ng/${icon_dpi}/desktop/)
install(FILES ${branding_icons} DESTINATION ${RESOURCES_DEST_DIR}/ng/${icon_dpi}/branding/)
