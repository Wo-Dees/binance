find_package(Boost 1.40 COMPONENTS system REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${Boost_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(binana)

target_sources(binana PRIVATE client.cpp order.cpp)
target_include_directories(binana PUBLIC ../)
target_link_libraries(binana PRIVATE Threads::Threads OpenSSL::SSL ${Boost_LIBRARIES})