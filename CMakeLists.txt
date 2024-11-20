cmake_minimum_required(VERSION 3.13)
project(DnsResolver)
cmake_policy(SET CMP0167 NEW)

set(CMAKE_CXX_STANDARD 17)

# Находим зависимости
find_package(Boost REQUIRED COMPONENTS system filesystem json)
find_library(PQXX_LIBRARY pqxx REQUIRED)
# find_library(PQ_LIBRARY pq REQUIRED)

# Таргет для библиотеки
add_library(DnsResolverLib DnsResolver.cpp)
target_include_directories(DnsResolverLib PUBLIC .)
target_link_libraries(DnsResolverLib PUBLIC Boost::system Boost::filesystem Boost::json ${PQXX_LIBRARY} ${PQ_LIBRARY})

# Таргет для исполняемого файла
add_executable(TestResolver TestResolver.cpp)
target_link_libraries(TestResolver PRIVATE DnsResolverLib)
