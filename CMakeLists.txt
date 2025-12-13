cmake_minimum_required(VERSION 3.15)
project(OrderService)



set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0A00)  # Windows 10
    add_compile_definitions(WINVER=0x0A00)        # Windows 10
endif()

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
)

set(PCH_HEADER src/stdafx.h)

add_executable(my_OrderService ${SOURCE_FILES})

source_group("Main" FILES
    src/main.cpp
)

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


source_group("PCH" FILES ${PCH_HEADER})

target_precompile_headers(my_OrderService PRIVATE ${PCH_HEADER})

find_package(Boost CONFIG REQUIRED COMPONENTS system asio beast)


find_package(libpqxx REQUIRED CONFIG)
message(STATUS "libpqxx found: ${libpqxx_FOUND}")

find_package(nlohmann_json 3.2.0 REQUIRED)

find_package(PkgConfig REQUIRED)

# Ищем Kafka через pkg-config
pkg_check_modules(CPPKAFKA cppkafka)
pkg_check_modules(RDKAFKA rdkafka)

message(STATUS "CPPKAFKA found: ${CPPKAFKA_FOUND}")
message(STATUS "RDKAFKA found: ${RDKAFKA_FOUND}")

if(CPPKAFKA_FOUND AND RDKAFKA_FOUND)
    message(STATUS "Kafka libraries found, enabling Kafka support")
    
    # 1. Добавляем include директории (для компилятора)
    target_include_directories(my_OrderService PRIVATE 
        ${CPPKAFKA_INCLUDE_DIRS}
    )
    
    # 2. Добавляем пути к библиотекам (для линкера)
    target_link_directories(my_OrderService PRIVATE 
        ${CPPKAFKA_LIBRARY_DIRS}
    )
    
    # 3. Подключаем библиотеки по именам
    target_link_libraries(my_OrderService PRIVATE 
        cppkafka
        rdkafka
    )
    
    # 4. Ищем и копируем DLL (только на Windows)
    if(WIN32)
        find_file(CPPKAFKA_DLL
            NAMES cppkafka.dll
            PATHS ${CPPKAFKA_LIBRARY_DIRS}
            PATH_SUFFIXES bin
            NO_DEFAULT_PATH
        )
        
        find_file(RDKAFKA_DLL
            NAMES rdkafka.dll
            PATHS ${CPPKAFKA_LIBRARY_DIRS}
            PATH_SUFFIXES bin
            NO_DEFAULT_PATH
        )
        
        if(CPPKAFKA_DLL AND RDKAFKA_DLL)
            message(STATUS "Found Kafka DLLs, will copy to output directory")
            
            add_custom_command(TARGET my_OrderService POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CPPKAFKA_DLL}"
                    "$<TARGET_FILE_DIR:my_OrderService>"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${RDKAFKA_DLL}"
                    "$<TARGET_FILE_DIR:my_OrderService>"
                COMMENT "Copying Kafka DLLs"
            )
        else()
            message(WARNING "Kafka DLLs not found, program may fail at runtime")
        endif()
    endif()
else()
    message(WARNING "Kafka not found, building without Kafka support")
    target_compile_definitions(my_OrderService PRIVATE NO_KAFKA_SUPPORT)
endif()



# ДОБАВЬТЕ ОТЛАДКУ
if(TARGET libpqxx::pqxx)
    message(STATUS "Target libpqxx::pqxx exists")
    
    # Получим свойства цели
    get_target_property(PQXX_INCLUDES libpqxx::pqxx INTERFACE_INCLUDE_DIRECTORIES)
    message(STATUS "PQXX include directories: ${PQXX_INCLUDES}")
    
    get_target_property(PQXX_LIBRARY libpqxx::pqxx LOCATION)
    message(STATUS "PQXX library: ${PQXX_LIBRARY}")
else()
    message(FATAL_ERROR "Target libpqxx::pqxx not found!")
endif()



target_link_libraries(my_OrderService PRIVATE
    Boost::system
    Boost::asio
    Boost::beast
    libpqxx::pqxx
    nlohmann_json::nlohmann_json
)
