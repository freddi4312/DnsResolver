set (TARGET_NAME "tester")
set (SOURCES "main.cpp")

add_executable (${TARGET_NAME} ${SOURCES})
target_link_libraries (${TARGET_NAME} PRIVATE
  sockpp-static
  tins
)

add_dependencies (${TARGET_NAME} gen1)
