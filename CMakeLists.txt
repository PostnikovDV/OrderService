cmake_minimum_required(VERSION 3.15)
project(OrderService)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0A00)
    add_compile_definitions(WINVER=0x0A00)
endif()

# Проверка существования файлов
set(SOURCE_FILES
    src/main.cpp
    src/Orders/OrderService.cpp
    src/Orders/OrderService.h
    src/Utils/ServiceUtils.h
    src/Utils/ServiceUtils.cpp
    src/Handlers/CreateOrderHandler.cpp
    src/Handlers/CreateOrderHandler.h
    src/Handlers/GetOrderHandler.cpp
    src/Handlers/GetOrderHandler.h
    src/Handlers/DeleteOrderHandler.cpp
    src/Handlers/DeleteOrderHandler.h
    src/ServerUtils/HttpListener.cpp
    src/ServerUtils/HttpListener.h
    src/ServerUtils/HttpSession.cpp
    src/ServerUtils/HttpSession.h
    src/Clients/PaymentClient.cpp
    src/Clients/PaymentClient.h
)

# Проверяем существование файлов
foreach(FILE ${SOURCE_FILES})
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}")
        message(WARNING "File not found: ${FILE}")
    endif()
endforeach()

set(PCH_HEADER src/stdafx.h)

add_executable(OrderService ${SOURCE_FILES})

source_group("Main" FILES src/main.cpp)
source_group("Orders" FILES 
    src/Orders/OrderService.cpp
    src/Orders/OrderService.h
)
source_group("Utils" FILES
    src/Utils/ServiceUtils.h
    src/Utils/ServiceUtils.cpp
)
source_group("Handlers" FILES
    src/Handlers/CreateOrderHandler.cpp
    src/Handlers/CreateOrderHandler.h
    src/Handlers/GetOrderHandler.cpp
    src/Handlers/GetOrderHandler.h
    src/Handlers/DeleteOrderHandler.cpp
    src/Handlers/DeleteOrderHandler.h
)
source_group("ServerUtils" FILES
    src/ServerUtils/HttpListener.cpp
    src/ServerUtils/HttpListener.h
    src/ServerUtils/HttpSession.cpp
    src/ServerUtils/HttpSession.h
)

source_group("Clients" FILES 
    src/Clients/PaymentClient.cpp
    src/Clients/PaymentClient.h
)

source_group("PCH" FILES ${PCH_HEADER})

target_precompile_headers(OrderService PRIVATE ${PCH_HEADER})

# ===== BOOST =====
# Для vcpkg используем find_package с CONFIG
find_package(Boost CONFIG REQUIRED COMPONENTS system asio beast uuid)
if(Boost_FOUND)
    message(STATUS "Boost found: ${Boost_VERSION}")
else()
    message(FATAL_ERROR "Boost not found!")
endif()

# ===== libpqxx через apt (системная установка) =====
# Используем find_library вместо find_package
find_library(PQXX_LIBRARY pqxx REQUIRED)
if(PQXX_LIBRARY)
    message(STATUS "PQXX library found: ${PQXX_LIBRARY}")
else()
    message(FATAL_ERROR "PQXX library not found!")
endif()

# ===== nlohmann_json =====
find_package(nlohmann_json 3.2.0 CONFIG REQUIRED)
if(TARGET nlohmann_json::nlohmann_json)
    message(STATUS "nlohmann_json found")
else()
    message(FATAL_ERROR "nlohmann_json not found!")
endif()

# ===== Kafka (опционально) =====
# Используем find_package вместо pkg-config на Windows
find_package(cppkafka CONFIG QUIET)
find_package(rdkafka CONFIG QUIET)

if(TARGET cppkafka AND TARGET rdkafka)
    message(STATUS "Kafka libraries found, enabling Kafka support")
    target_link_libraries(OrderService PRIVATE cppkafka rdkafka)
    
    # Копирование DLL на Windows
    if(WIN32)
        # Находим DLL через vcpkg
        find_file(CPPKAFKA_DLL
            NAMES cppkafka.dll
            PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../vcpkg/vcpkg/installed/x64-windows/bin
            NO_DEFAULT_PATH
        )
        find_file(RDKAFKA_DLL
            NAMES rdkafka.dll
            PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../../vcpkg/vcpkg/installed/x64-windows/bin
            NO_DEFAULT_PATH
        )
        
        if(CPPKAFKA_DLL AND RDKAFKA_DLL)
            message(STATUS "Found Kafka DLLs, will copy to output directory")
            add_custom_command(TARGET OrderService POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CPPKAFKA_DLL}" "$<TARGET_FILE_DIR:OrderService>"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${RDKAFKA_DLL}" "$<TARGET_FILE_DIR:OrderService>"
                COMMENT "Copying Kafka DLLs"
            )
        endif()
    endif()
else()
    message(WARNING "Kafka not found, building without Kafka support")
    target_compile_definitions(OrderService PRIVATE NO_KAFKA_SUPPORT)
endif()

# ===== Линковка =====
target_link_libraries(OrderService PRIVATE
    Boost::system
    Boost::asio
    Boost::beast
    ${PQXX_LIBRARY}
    nlohmann_json::nlohmann_json
)

message(STATUS "Build configuration complete!")