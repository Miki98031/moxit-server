#include <iostream>
#include "moxit_server.h"

using namespace std;

int main()
{
    MoxitServer server = MoxitServer("0.0.0.0", 8080);
    server.startListen();

    std::string command;
    while (std::cin >> command, command != "quit") {
          //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
