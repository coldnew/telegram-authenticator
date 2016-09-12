#ADD_SUBDIRECTORY(pam_telegram_authenticator)
#ADD_SUBDIRECTORY(telegram-authenticator)

INCLUDE_DIRECTORIES (
  ${PAM_INCLUDE_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR})

# common files
SET(common_SRCS
  config.c
  telegram.c)

# pam_telegram_authenticator

SET(pam_telegram_authenticator_SRCS
  ${common_SRCS}
  pam_telegram_authenticator.c)

ADD_LIBRARY(pam_telegram_authenticator MODULE ${pam_telegram_authenticator_SRCS})
SET_TARGET_PROPERTIES(pam_telegram_authenticator PROPERTIES PREFIX "")

TARGET_LINK_LIBRARIES (pam_telegram_authenticator
  ${PKGS_LDFLAGS}
  ${PAM_LIBRARIES})

INSTALL(TARGETS pam_telegram_authenticator DESTINATION /${CMAKE_INSTALL_LIBDIR}/security)

# telegram-authenticator

SET(telegram-authenticator_SRCS
  ${common_SRCS}
  telegram-authenticator.c)

ADD_EXECUTABLE(telegram-authenticator ${telegram-authenticator_SRCS})
SET_TARGET_PROPERTIES(telegram-authenticator PROPERTIES PREFIX "")

TARGET_LINK_LIBRARIES (telegram-authenticator ${PKGS_LDFLAGS})
