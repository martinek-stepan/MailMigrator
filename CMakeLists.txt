cmake_minimum_required (VERSION 2.8)

set (PROJECT_NAME MailMigrator)
project (${PROJECT_NAME})   
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})


if(CMAKE_COMPILER_IS_GNUCXX)
  add_compile_options(-std=c++11) # CMake 2.8.12 or newer
  set(CMAKE_CXX_STANDARD 11) # C++11...
  set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
  set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11
endif()


# add dependencies
add_subdirectory("dep")

# add source
add_subdirectory("src")

add_executable(${PROJECT_NAME} ${SOURCE} ${HEADERS})

if(CMAKE_COMPILER_IS_GNUCXX)
  TARGET_LINK_LIBRARIES(${PROJECT_NAME} pthread)
  set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wformat=0")
endif()

include_directories(
  ${PROJECT_SOURCE_DIR}/dep/SQLiteCpp/include/
  ${PROJECT_SOURCE_DIR}/dep/SQLiteCpp/sqlite3/

)

target_link_libraries( ${PROJECT_NAME}
   SQLiteCpp
   sqlite3
)
