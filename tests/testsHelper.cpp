#include "testsHelper.h"

App createTestApp()
{
    AppConfig appConfig{
        NetworkAddress("127.0.0.1", "8080"),
        "404685505601234",
        "490154203237518",
        "89991112200"
    };

    AppState appState{
        LocationPoint3D{0,0.6,-45.84},
        false,
        Protocol::JSON,
        std::nullopt
    };
    
    return App(appConfig, appState, createTestAddressBook());
}

std::vector<char*> createArgv(const std::vector<std::string>& args)
{
    std::vector<char*> argv;
    argv.reserve(args.size() + 1);
    for (const auto& arg : args)
    {
        argv.push_back(const_cast<char*>(arg.c_str())); 
    }
    argv.push_back(nullptr);
    return argv;
}

std::map<size_t, AddressBookEntry> createTestAddressBook()
{
    std::map<size_t, AddressBookEntry> addressBook;

    addressBook.insert({1, AddressBookEntry{"89991112233"}});
    addressBook.insert({2, AddressBookEntry{"89993334455"}});
    addressBook.insert({3, AddressBookEntry{"89995556677"}});
    addressBook.insert({4, AddressBookEntry{"89997778899"}});

    return addressBook;
}