#include <map>
#include <sstream>
#include <iostream>

#include <windows.h>

#include "teststatusprint.hpp"

const size_t command_size = 64;

// SERVER
int main()
{
    // п. 1
    std::string pipe_name;
    std::cout << "Enter pipe name: ";
    std::cin >> pipe_name;
    std::string pipe = "\\\\.\\pipe\\" + pipe_name;
    HANDLE h_pipe = CreateNamedPipe(pipe.c_str(),
                                    PIPE_ACCESS_DUPLEX,
                                    PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
                                    PIPE_UNLIMITED_INSTANCES,
                                    64, 64, 0, nullptr);
    if (h_pipe == INVALID_HANDLE_VALUE)
        test_status_print(false, "occurred while creating pipe", pipe_name);

    // п. 2
    DWORD numberOfBytes;
    while (true)
    {
        std::cout << "Awaiting for client-to-pipe connection...\n";
        test_status_print(ConnectNamedPipe(h_pipe, nullptr),
                          "occurred while connecting pipe",
                          pipe_name);

        // п. 3
        std::string command (command_size, '\0');
        std::string keyword,
                    key,
                    value,
                    response {};
        std::map<std::string, std::string> data {};
        while (true)  // п. 4
        {
            std::cout << "Awaiting for client command...\n";
            test_status_print(ReadFile(h_pipe, &command[0], command.size(), &numberOfBytes, nullptr),
                              "occurred while reading command",
                              command);
            command.resize(command.find('\0'));

            std::istringstream parser {command};
            parser >> std::ws >> keyword;
            if (keyword == "set")
            {
                parser >> key >> value;
                data[key] = value;
                response = "acknowledged";
            }
            else if (keyword == "get")
            {
                parser >> key;
                if (data.find(key) != data.end())
                    response = "found " + data[key];
                else
                    response = "missing";
            }
            else if (keyword == "list")
            {
                for (auto i = data.begin(); i != data.end(); ++ i)
                    response += i->first + " ";
            }
            else if (keyword == "delete")
            {
                parser >> key;
                auto del = data.find(key);
                if (del != data.end())
                {
                    data.erase(del);
                    response = "deleted";
                }
                else
                    response = "missing";
            }
            else if (keyword == "quit")
            {
                test_status_print(DisconnectNamedPipe(h_pipe),
                                  "occurred while disconnecting pipe",
                                  pipe_name);
                command.replace(0, command.size(), command.size(), '\0');
                command.resize(command_size, '\0');
                break;
            }
            else
            {
                std::cerr << "Incorrect command! (command: \"" << command << "\")\n";
                response = "incorrect command";
            }

            test_status_print(WriteFile(h_pipe, response.c_str(), response.size(), &numberOfBytes, nullptr),
                              "occurred while writing to pipe",
                              pipe_name);

            command.replace(0, command.size(), command.size(), '\0');
            command.resize(command_size, '\0');
            response.clear();
            keyword.clear();
            key.clear();
            value.clear();
        }

        // п. 5
        char kill;
        bool exit = false;
        std::cout << "Do you want to destroy pipe \"" << pipe_name << "\" (y/n)?: ";
        while (std::cin >> kill)
        {
            if (kill == 'y')
            {
                test_status_print(CloseHandle(h_pipe),
                                  "occurred while closing pipe",
                                  pipe_name);
                exit = true;
                break;
            }
            else if (kill == 'n')
                break;
            else
            {
                std::cout << "(y/n): ";
                continue;
            }
        }
        if (exit)
            break;
    }
}
