cmake_minimum_required(VERSION 3.14)
project(TelecomApp)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git 
    GIT_TAG v1.12.x
)
FetchContent_MakeAvailable(googletest)

set(CORE_SOURCE_LIST
    CORE/types.cpp
    CORE/NetworkAddress.cpp
    CORE/ClientSocket.cpp
    CORE/ServerSocket.cpp
    CORE/PacketTransporter.cpp
    CORE/network/BinarySerializer.cpp
    CORE/network/JsonSerializer.cpp
    CORE/entitiesConfigs/SubscriberRecord.cpp
    COMMON/utils.cpp
    COMMON/networkUtils.cpp
    COMMON/parserUtils.cpp
    COMMON/validationUtils.cpp
    COMMON/logging/FileLogger.cpp
    COMMON/logging/Logger.cpp
    APP/app/AppParams.cpp
    APP/app/StartupParams.cpp
)

add_library(${PROJECT_NAME}Core STATIC ${CORE_SOURCE_LIST})
target_include_directories(${PROJECT_NAME}Core PUBLIC
    CORE/
    CORE/network
    COMMON/
)
target_link_libraries(${PROJECT_NAME}Core PUBLIC 
    nlohmann_json::nlohmann_json
)


add_library(${PROJECT_NAME}ClientLib STATIC
    APP/app/App.cpp
    APP/CommandHandler/CommandHandler.cpp
    APP/CommandHandler/SingleArgCommand.cpp
    APP/CommandHandler/CommandActive.cpp
    APP/CommandHandler/CommandExit.cpp
    APP/CommandHandler/CommandMove.cpp
    APP/CommandHandler/CommandProtocol.cpp    
)
target_include_directories(${PROJECT_NAME}ClientLib PUBLIC 
    APP/
    APP/app
    APP/CommandHandler
)
target_link_libraries(${PROJECT_NAME}ClientLib PUBLIC ${PROJECT_NAME}Core)

add_executable(${PROJECT_NAME}Client
    APP/mainApp.cpp
)
target_link_libraries(${PROJECT_NAME}Client PRIVATE ${PROJECT_NAME}ClientLib)


add_executable(${PROJECT_NAME}Server
    SERVER/mainServer.cpp
    SERVER/ServerApp.cpp
    #SERVER/entities/hlrAndEir/HlrEirInterface.cpp
    SERVER/entities/hlrAndEir/JsonHlrEir.cpp
    SERVER/entities/BaseStation.cpp
    SERVER/entities/MobilityManagmentEntity.cpp
)
target_link_libraries(${PROJECT_NAME}Server PRIVATE ${PROJECT_NAME}Core)

enable_testing()

add_executable(${PROJECT_NAME}Tests
    tests/testMain.cpp
    tests/testsHelper.cpp
    tests/UtilsTests.cpp
    tests/NetworkAddressTests.cpp
    tests/ValidatorsTests.cpp
    tests/ParserTests.cpp
    tests/AppTests.cpp
    tests/CommandsTests.cpp
)

target_link_libraries(${PROJECT_NAME}Tests PRIVATE 
    ${PROJECT_NAME}Core
    ${PROJECT_NAME}ClientLib
    GTest::gtest 
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}Tests)