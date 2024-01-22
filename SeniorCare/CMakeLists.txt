cmake_minimum_required(VERSION 3.10)
project(projet_c LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(JSONGLIB REQUIRED json-glib-1.0)

find_package(CURL REQUIRED)

find_package(OpenSSL REQUIRED)

include_directories(
        ${GTK3_INCLUDE_DIRS}
        ${JSONGLIB_INCLUDE_DIRS}
        ${CURL_INCLUDE_DIRS}
        ${OPENSSL_INCLUDE_DIR}
        "C:/Projet_c/postgresql/include"
)

add_executable(projet_c main.c medecin_page.c bdd.h bdd.c patient_page.c)

target_link_libraries(projet_c
        ${GTK3_LIBRARIES}
        ${JSONGLIB_LIBRARIES}
        ${CURL_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        "C:/Projet_c/postgresql/lib/libpq.lib"
        )
