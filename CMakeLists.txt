cmake_minimum_required(VERSION 3.15)
project(discord-bot VERSION 1.0 DESCRIPTION "A discord bot")

aux_source_directory(src mainsrc)
add_executable(
        ${PROJECT_NAME}
        ${mainsrc})


target_link_libraries(${PROJECT_NAME}
        dpp
        spdlog
        cpp_dotenv
        SQLiteCpp
        )

target_include_directories(${PROJECT_NAME} PRIVATE
        libs/DPP/include
        libs/spdlog/include
        )

add_subdirectory(libs/DPP)
add_subdirectory(libs/spdlog)
add_subdirectory(libs/cpp-dotenv)
add_subdirectory(libs/SQLiteCpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        )