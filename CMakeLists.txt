# minimum CMake version required
cmake_minimum_required(VERSION 3.15)
# Project name, version and description
project(discord-bot VERSION 1.0 DESCRIPTION "A discord bot")

# Add DPP as dependency
add_subdirectory(libs/DPP)
add_subdirectory(libs/spdlog)
add_subdirectory(libs/cpp-dotenv)

find_package(SqliteOrm REQUIRED)

# Create an executable
add_executable(${PROJECT_NAME}
        src/main.cpp
        # your others files...
        src/DBAdapter.cpp
        src/DBAdapter.h
        src/MessageChecker.cpp src/MessageChecker.h)

# Linking libraries

target_link_libraries(${PROJECT_NAME} PRIVATE
        dpp
        spdlog
        cpp_dotenv
        sqlite_orm::sqlite_orm
        )
#target_link_libraries(${PROJECT_NAME} PRIVATE sqlite_orm::sqlite_orm)

# Specify includes
target_include_directories(${PROJECT_NAME} PRIVATE
        libs/DPP/include
        libs/spdlog/include
        )

# Set C++ version
set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        )