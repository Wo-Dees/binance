find_package(Boost 1.40 COMPONENTS system REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${Boost_INCLUDE_DIR})

add_library(banana STATIC)

target_sources(banana PRIVATE binance/client.cpp)
target_link_libraries(banana PRIVATE Threads::Threads OpenSSL::SSL ${Boost_LIBRARIES})