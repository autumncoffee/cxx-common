file(GLOB AC_COMMON_SOURCES *.cpp)

add_subdirectory(utils)

add_library(ac_common STATIC ${AC_COMMON_SOURCES})

target_link_libraries(
    ac_common
    ac_common_utils
    "-lpthread"
)
