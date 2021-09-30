#include <map>
#include <sstream>
#include <iostream>

#include <windows.h>

#include "teststatusprint.hpp"

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
                                    PIPE_TYPE_MESSAGE,
                                    1, //PIPE_UNLIMITED_INSTANCES,
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
        std::cout << "Awaiting for client command...\n";

        // п. 3
        std::string command {64, '\0'};
        while (true)  // п. 4
        {
            test_status_print(ReadFile(h_pipe, &command[0], command.size(), &numberOfBytes, nullptr),
                              "occurred while reading command",
                              command);

            std::map<std::string, std::string> data {};
            std::istringstream parser {command};
            std::string keyword,
                        key,
                        value,
                        response {};
            parser >> keyword;
            if (keyword == "set")
            {
                parser >> key >> value;
                data[key] = value;
                response = "acknowledged";
                break;
            }
            else if (keyword == "get")
            {
                parser >> key;
                if (data.find(key) != data.end())
                    response = "found" + data[key];
                else
                    response = "missing";
                break;
            }
            else if (keyword == "list")
            {
                for (auto i = data.begin(); i != data.end(); ++ i)
                    response += i->first + " ";
                break;
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
                break;
            }
            else if (keyword == "quit")
            {
                test_status_print(DisconnectNamedPipe(h_pipe),
                                  "occurred while disconnecting pipe",
                                  pipe_name);
                break;
            }
            else
            {
                std::cerr << "Incorrect command! (command: \"" << command << "\")\n";
                continue;
            }

            test_status_print(WriteFile(h_pipe, response.c_str(), response.size(), &numberOfBytes, nullptr),
                              "occurred while writing to pipe",
                              pipe_name);

            // п. 5
            char kill;
            while (std::cin >> kill)
            {
                std::cout << "Do you want to destroy pipe \"" << pipe_name << "\" (y\n)?\n";
                if (kill == 'y')
                {
                    test_status_print(CloseHandle(h_pipe),
                                      "occurred while closing pipe",
                                      pipe_name);
                }
                else if (kill == 'n')
                    break;
            }

            command.clear();
        }
    }
}
