#include <iostream>
#include <map>

#include <windows.h>

#include "teststatusprint.hpp"

// CLIENT
int main()
{
    // п. 1
    std::string pipe_name;
    std::cout << "Enter pipe name: ";
    std::cin >> pipe_name;
    std::string pipe = "\\\\.\\pipe\\" + pipe_name;
    HANDLE h_pipe = CreateFile(pipe.c_str(),
                               GENERIC_READ | GENERIC_WRITE,
                               0,
                               nullptr,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               nullptr);
    if (h_pipe == INVALID_HANDLE_VALUE)
        test_status_print(false, "occurred while connecting pipe", pipe_name);

    // п. 2
    DWORD numberOfBytes;
    std::string command {64, '\0'};
    while (true)  // п. 5
    {
        std::cout << "> ";
        std::cin >> command;
        test_status_print(WriteFile(h_pipe, command.c_str(), command.size(), &numberOfBytes, nullptr),
                          "occurred while writing to pipe",
                          pipe_name);

        // п. 3
        if (command == "quit")
        {
            test_status_print(CloseHandle(h_pipe),
                              "occurred while closing pipe",
                              pipe_name);
            break;
        }
        else
        {
            std::string response {};
            test_status_print(ReadFile(h_pipe, &response[0], response.size(), &numberOfBytes, nullptr),
                              "occurred while reading response",
                              response, response, '\n');
        }
        command.clear();
    }
}
