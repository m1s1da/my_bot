cmake_minimum_required(VERSION 3.15)
set(BOT_NAME "tavernbot")
project(${BOT_NAME})

aux_source_directory(src mainsrc)
add_executable(
        ${BOT_NAME}
        ${mainsrc})

string(ASCII 27 Esc)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set_target_properties(${BOT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        )

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
find_package(DPP)
find_package(OpenSSL REQUIRED)


target_include_directories(${BOT_NAME} PUBLIC
        ${OPENSSL_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

target_include_directories(${BOT_NAME} PRIVATE
        third_party/spdlog/include
        third_party/cpp-dotenv/include
        )

target_link_libraries(${BOT_NAME}
        spdlog
        cpp_dotenv
        SQLiteCpp
        dl
        ${CMAKE_THREAD_LIBS_INIT}
        ${OPENSSL_CRYPTO_LIBRARY}
        ${OPENSSL_SSL_LIBRARY}
        )

add_subdirectory(third_party/spdlog)
add_subdirectory(third_party/cpp-dotenv)
add_subdirectory(third_party/SQLiteCpp)

if (DPP_FOUND)
    target_link_libraries(${BOT_NAME} ${DPP_LIBRARIES})
    target_include_directories(${BOT_NAME} PUBLIC ${DPP_INCLUDE_DIR})
else ()
    message(WARNING "Could not find DPP install. Building from source instead.")
    option(DPP_BUILD_TEST "" OFF)
    include(FetchContent)

    FetchContent_Declare(
            libdpp
            GIT_REPOSITORY https://github.com/brainboxdotcc/DPP.git
            GIT_TAG master)

    FetchContent_GetProperties(libdpp)
    if (NOT libdpp_POPULATED)
        FetchContent_Populate(libdpp)
        target_include_directories(${BOT_NAME} PUBLIC
                ${libdpp_SOURCE_DIR}/include
                )
        add_subdirectory(
                ${libdpp_SOURCE_DIR}
                ${libdpp_BINARY_DIR}
                EXCLUDE_FROM_ALL)
    endif ()

    target_link_libraries(${BOT_NAME} dpp)
endif ()