project(cmocka-headers C)

set(cmocka_HDRS
  cmocka.h
)

install(
  FILES
    ${cmocka_HDRS}
  DESTINATION
    ${INCLUDE_INSTALL_DIR}
  COMPONENT
    headers
)
