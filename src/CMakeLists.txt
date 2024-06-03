include_directories(common)

set(SERVER_NAME test_server)
set(CLIENT_NAME test_client)

add_executable(
  ${SERVER_NAME}
  server/main.cxx
  server/server.cxx
  common/packet.cxx
  common/checksum.cxx
)

add_executable(
  ${CLIENT_NAME}
  client/main.cxx
  common/packet.cxx
  common/checksum.cxx
)
