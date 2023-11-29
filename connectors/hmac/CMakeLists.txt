find_package(OpenSSL REQUIRED)

add_library(hmac)

target_sources(hmac PRIVATE hmac.cpp)
target_include_directories(hmac PUBLIC ../)
target_link_libraries(hmac PRIVATE OpenSSL::SSL)