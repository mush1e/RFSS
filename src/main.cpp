#include "server.hpp"

auto main() -> int {
    rfss::HTTP_Server server(8080);
    server.start();
    return 0;
}