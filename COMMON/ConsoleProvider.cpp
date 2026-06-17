#include "ConsoleProvider.h"

void ConsoleProvider::setInvitation(const std::string& invitation)
{
    std::lock_guard<std::mutex> lock(consoleMutex);
    currentInvitation = invitation;
}

void ConsoleProvider::printNotification(std::string_view msg, std::string_view prefix)
{
    executeSafePrint([&]()
                    {
                        std::cout << prefix << msg << '\n';
                    }
    );
}