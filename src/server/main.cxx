#include <cstdlib>
#include "server.hxx"

constexpr size_t g_port = 1234;
constexpr std::string_view g_ip = "127.0.0.1";

int main() {
  server server{g_ip, g_port};
  server.listen();
  return EXIT_SUCCESS;
}
