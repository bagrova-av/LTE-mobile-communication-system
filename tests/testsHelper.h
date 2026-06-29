#pragma once
#ifndef TESTS_HELPER_H
#define TESTS_HELPER_H

#include <vector>

#include "../APP/app/App.h"

App createTestApp();

std::vector<char*> createArgv(const std::vector<std::string>& args);

std::map<size_t, AddressBookEntry> createTestAddressBook();

#endif