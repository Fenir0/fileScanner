cmake_minimum_required(VERSION 3.10.0)

set(CMAKE_CXX_STANDARD 20)
set(PATCH_VERSION "1" CACHE STRING "Patch version")
set(PROJECT_VERSION "0.0.${PATCH_VERSION}")

project(lab7 VERSION ${PROJECT_VERSION} LANGUAGES C CXX)

# Find Boost with filesystem and system components
find_package(Boost REQUIRED COMPONENTS filesystem system regex)

# Find Flex/libfl
find_package(FLEX)
if(FLEX_FOUND)
    find_library(FL_LIBRARY fl)
endif()

add_executable(lab7 scanner.hpp scanner.cpp main.cpp)

target_include_directories(lab7 PRIVATE ${Boost_INCLUDE_DIRS})

# Link against the specific Boost libraries you're using
target_link_libraries(lab7 PRIVATE 
    Boost::filesystem
    Boost::system
    Boost::regex
)

# Only link FL if found
if(FL_LIBRARY)
    target_link_libraries(lab7 PRIVATE ${FL_LIBRARY})
endif()

install(TARGETS lab7 RUNTIME DESTINATION bin)

if(UNIX)
    set(CPACK_GENERATOR DEB)
endif(UNIX)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT darth@vader.com)

include(CPack)