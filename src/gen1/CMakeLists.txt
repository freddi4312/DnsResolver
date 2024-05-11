set (TARGET_NAME "gen1")
set (SOURCES
  "Resolver.cpp"
  "Resolver.h"
  "common.h"
  "main.cpp"
  "Receiver.h"
  "Receiver.cpp"
  "DnsXmitter.h"
  "DnsXmitter.cpp"
  "SocketExceptions.h"
  "DomainName.h"
  "DomainName.cpp"
  "Cache.h" 
  "Cache.cpp"
  "Record.h"
)

add_executable (${TARGET_NAME})
target_sources (${TARGET_NAME} PRIVATE ${SOURCES})
target_link_libraries (${TARGET_NAME} PRIVATE
  sockpp-static
  tins
  magic_enum::magic_enum
)
