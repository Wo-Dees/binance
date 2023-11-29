find_package(OpenSSL REQUIRED)

add_library(hmac_banana STATIC)

target_sources(hmac_banana PRIVATE hmac.cpp)
target_link_libraries(hmac_banana PRIVATE OpenSSL::SSL)
