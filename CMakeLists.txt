cmake_minimum_required(VERSION 3.12)
project(enrollment_template CXX)

include(third_party/userver/cmake/SetupEnvironment.cmake)
include(GNUInstallDirs)

add_subdirectory(third_party/userver)


# Common sources
add_library(${PROJECT_NAME}_objs OBJECT
    src/hello.hpp
    src/hello.cpp
    src/models/courier.hpp
    src/models/courier.cpp
    src/handlers/v1/get-courier/view.hpp
    src/handlers/v1/get-courier/view.cpp
    src/handlers/v1/get-all-couriers/view.hpp
    src/handlers/v1/get-all-couriers/view.cpp
    src/handlers/v1/add-couriers/view.hpp
    src/handlers/v1/add-couriers/view.cpp
    src/models/order.hpp
    src/models/order.cpp
    src/handlers/v1/get-all-orders/view.hpp
    src/handlers/v1/get-all-orders/view.cpp
    src/handlers/v1/get-order/view.hpp
    src/handlers/v1/get-order/view.cpp
    src/handlers/v1/add-orders/view.hpp
    src/handlers/v1/add-orders/view.cpp
    src/handlers/v1/complete-orders/view.hpp
    src/handlers/v1/complete-orders/view.cpp
    src/handlers/v1/assign-orders/view.hpp
    src/handlers/v1/assign-orders/view.cpp
    src/handlers/v1/get-couriers-assignments/view.hpp
    src/handlers/v1/get-couriers-assignments/view.cpp
)
target_link_libraries(${PROJECT_NAME}_objs PUBLIC userver-core userver-postgresql)


# The Service
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)


# Unit Tests
add_executable(${PROJECT_NAME}_unittest
    src/hello_test.cpp
)
target_link_libraries(${PROJECT_NAME}_unittest PRIVATE ${PROJECT_NAME}_objs userver-utest)
add_google_tests(${PROJECT_NAME}_unittest)


# Functional Tests
add_subdirectory(tests)

if(DEFINED ENV{PREFIX})
	message(STATUS "Set install prefix: $ENV{PREFIX}")
	file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
	set(CMAKE_INSTALL_PREFIX ${PREFIX_PATH})
endif()

set(CONFIG_VAR_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}/config_vars.yaml)
set(CONFIG_FALLBACK_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}/dynamic_config_fallback.json)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configs/static_config.yaml.in ${CMAKE_CURRENT_SOURCE_DIR}/configs/static_config.yaml)

FILE(GLOB CONFIGS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${PROJECT_NAME})
install(FILES ${CONFIGS_FILES} DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME} COMPONENT ${PROJECT_NAME})
