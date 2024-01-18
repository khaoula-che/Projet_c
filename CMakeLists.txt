cmake_minimum_required(VERSION 3.10)
project(projet_c LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

add_executable(projet_c main.c medecin_page.c medecin_page.c bdd.h bdd.c patient_page.c)

set(PQ_INCLUDE_DIR "C:/Program Files/PostgreSQL/16/include")
set(PQ_LIBRARY "C:/Program Files/PostgreSQL/16/lib")

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

include_directories(${PQ_INCLUDE_DIR})
target_link_libraries(projet_c ${GTK3_LIBRARIES} ${PQ_LIB} ${OPENSSL_LIBRARIES})