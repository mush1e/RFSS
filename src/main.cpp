#include "server.hpp"

using namespace rfss;

auto main() -> int {
    HTTP_Server server(8080);
    server.start();
    return 0;
}