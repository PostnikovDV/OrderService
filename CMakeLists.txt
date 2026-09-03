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
    src/Kafka/KafkaProducer.cpp
    src/Kafka/KafkaProducer.h
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

source_group("Kafka" FILES 
    src/Kafka/KafkaProducer.cpp
    src/Kafka/KafkaProducer.h
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
find_library(RDKAFKA_LIB rdkafka)
find_library(RDKAFKA_CPP_LIB rdkafka++)

if(RDKAFKA_LIB AND RDKAFKA_CPP_LIB)
    message(STATUS "✅ librdkafka found!")
    set(HAVE_KAFKA ON)
else()
    message(WARNING "⚠️ librdkafka not found! Kafka support disabled.")
    set(HAVE_KAFKA OFF)
endif()

# ===== Линковка =====
target_link_libraries(OrderService PRIVATE
    Boost::system
    Boost::asio
    Boost::beast
    ${PQXX_LIBRARY}
    nlohmann_json::nlohmann_json
)

if(HAVE_KAFKA)
    target_link_libraries(OrderService PRIVATE
        ${RDKAFKA_CPP_LIB}
        ${RDKAFKA_LIB}
    )
    target_compile_definitions(OrderService PRIVATE HAVE_KAFKA)
endif()

message(STATUS "Build configuration complete!")