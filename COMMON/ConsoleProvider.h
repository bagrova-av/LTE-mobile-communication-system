#pragma once
#ifndef CONSOLE_PROVIDER_H
#define CONSOLE_PROVIDER_H

#include <mutex>
#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>

class ConsoleProvider
{
private:
    static inline std::mutex consoleMutex;
    static inline std::string currentInvitation = "Please, enter the command: ";

public:
    static void setInvitation(const std::string& invitation);

    template<typename Func>
    static void executeSafePrint(const Func& printLogic, bool showInvitation = true)
    {
        std::lock_guard<std::mutex> lock(consoleMutex);
        
        tcflush(STDIN_FILENO, TCIFLUSH); // Clearing the OS input buffer
        std::cout << "\r" << std::string(80, ' ') << "\r";

        printLogic();

        if (showInvitation)
        {
            std::cout << '\n' << currentInvitation << std::flush;
        }
    }

    static void printNotification(std::string_view message, std::string_view prefix);
};

// inline void ConsoleProvider::safePrint(std::string_view message, std::string_view prefix,
//                                        bool isError)
// {
//     std::lock_guard<std::mutex> lock(consoleMutex);
//     tcflush(STDIN_FILENO, TCIFLUSH); // Clearing the OS input buffer

//     std::cout << "\r" << std::string(80, ' ') << "\r";

//     if (isError)
//     {
//         std::cerr << message << '\n';
//     }
//     else
//     {
//         std::cout << prefix << message << '\n';
//     }

//     std::cout << currentInvitation << std::flush;
// }

#endif