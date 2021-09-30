#ifndef TESTSTATUSPRINT_HPP
#define TESTSTATUSPRINT_HPP

#include <iostream>

void variadic_print(std::ostream& os) {}

template <typename T, typename ...Args>
void variadic_print(std::ostream& os, T&& x, Args&&... args)
{
    os << x;
    variadic_print(std::cout, args...);
}

// функция для удобной печати сообщений
template <typename Status, typename T1, typename T2, typename ...Args>
void test_status_print(Status status, T1 action, T2 object, Args&&... args)
{
    if (!status)
        std::cerr << "Error "
                  << GetLastError()
                  << " "
                  << action
                  << " \""
                  << object
                  << "\"!\n";
    else
        variadic_print(std::cout, args...);
}

#endif  // TESTSTATUSPRINT_HPP
