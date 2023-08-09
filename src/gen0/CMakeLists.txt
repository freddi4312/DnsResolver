set (TARGET_NAME "gen0")
set (SOURCES
  "DnsResolver.cpp"
  "Resolver.cpp"
  "DnsResolver.h"
  "Resolver.h"
  "GlobalCache.h"
  "LocalCache.h"
  "PduDnsSender.h"
)

add_executable (${TARGET_NAME})
target_sources (${TARGET_NAME} PRIVATE ${SOURCES})
target_link_libraries (${TARGET_NAME} PRIVATE
  sockpp-static
  tins
  magic_enum::magic_enum
)
