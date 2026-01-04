cmake_minimum_required(VERSION 3.21)

project(mazer VERSION 1.0)

# Restrict to debug and release builds not things like MinSizeRel and RelWithDebInfo
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

include(GenerateExportHeader)

# Include functionality that will help us create the package config for this project
include(CMakePackageConfigHelpers)

enable_testing()

# Use the latest C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create the library using the library source files
add_library(mazer STATIC 
CharacterBuilder.cpp
ElapsedGameTimeProvider.cpp
Enemy.cpp
GameData.cpp
GameDataManager.cpp
GameObjectMoveStrategy.cpp
Level.cpp
pch.cpp
pickup.cpp
Player.cpp
PlayerComponent.cpp
Room.cpp
RoomGenerator.cpp
RoomInfo.cpp
Rooms.cpp)

add_library(mazer::mazer ALIAS mazer)

target_sources(mazer
 PUBLIC
  FILE_SET api
  TYPE HEADERS
  FILES
CharacterBuilder.h
ElapsedGameTimeProvider.h
Enemy.h
EnemyMovedEvent.h
EventNumber.h
GameData.h
GameDataManager.h
GameObjectEventFactory.h
GameObjectMoveStrategy.h
Level.h
pch.h
pickup.h
Player.h
PlayerCollidedWithEnemyEvent.h
PlayerCollidedWithPickupEvent.h
PlayerComponent.h
Room.h
RoomGenerator.h
RoomInfo.h
Rooms.h
SDLCollisionDetection.h)

# Generate a header file containing preprocessor macro definitions to control C/C++ symbol visibility.
generate_export_header(mazer)

# Find packages for library dependencies (these will come via vcpkg)
find_package(tinyxml2 CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(cppgamelib CONFIG REQUIRED)
find_package(unofficial-sodium CONFIG REQUIRED)
find_package(Lua REQUIRED)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)

# --- SDL2 core ---
set(SDL2_CORE_TARGET SDL2::SDL2)

# --- SDL2_ttf ---
if(TARGET SDL2_ttf::SDL2_ttf)
    set(SDL2_TTF_TARGET SDL2_ttf::SDL2_ttf)
elseif(TARGET SDL2_ttf::SDL2_ttf-static)
    set(SDL2_TTF_TARGET SDL2_ttf::SDL2_ttf-static)
else()
    message(FATAL_ERROR "SDL2_ttf target not found")
endif()

# --- SDL2_image ---
if(TARGET SDL2_image::SDL2_image)
    set(SDL2_IMAGE_TARGET SDL2_image::SDL2_image)
elseif(TARGET SDL2_image::SDL2_image-static)
    set(SDL2_IMAGE_TARGET SDL2_image::SDL2_image-static)
else()
    message(FATAL_ERROR "SDL2_image target not found")
endif()

# --- SDL2_mixer ---
if(TARGET SDL2_mixer::SDL2_mixer)
    set(SDL2_MIXER_TARGET SDL2_mixer::SDL2_mixer)
elseif(TARGET SDL2_mixer::SDL2_mixer-static)
    set(SDL2_MIXER_TARGET SDL2_mixer::SDL2_mixer-static)
else()
    message(FATAL_ERROR "SDL2_mixer target not found")
endif()

# Check if SDL2 targets are static
macro(add_sdl2_static_dependencies target)
    if (${target} MATCHES "-static$")
        if(UNIX)
            target_link_libraries(cppgamelib PRIVATE freetype z png jpeg pthread m dl)
        endif()
    endif()
endmacro()

# Set a few properties for the library 
set_target_properties(mazer PROPERTIES LINKER_LANGUAGE CXX)

# Link to external dependencies
target_link_libraries(mazer 
PRIVATE
SDL2::SDL2 
${SDL2_TTF_TARGET}
${SDL2_IMAGE_TARGET}
${SDL2_MIXER_TARGET}
unofficial-sodium::sodium
cppgamelib::cppgamelib
${LUA_LIBRARIES}
tinyxml2::tinyxml2
)

message("Include dirs:")
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()



# Make an executable that runs all the tests the provided files
enable_testing()

# Add an executable for running all tests. This excludes networking tests
add_executable(AllTests
tests/2DGameDevLibTests.cpp
tests/CharacterBuilderTests.cpp
tests/GameDataManagerTests.cpp
tests/GameDataTests.cpp
tests/GameObjectMoveStrategyTests.cpp
tests/LevelGeneratorTests.cpp
tests/LevelTests.cpp
tests/PickupTests.cpp
tests/PlayerTests.cpp
tests/RoomTests.cpp
)

# Set the properties for the test executables
set_target_properties(AllTests PROPERTIES LINKER_LANGUAGE CXX)

# Set the include directories for the test executables
target_include_directories(AllTests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# Link libraries that the test AllTests exeutable uses
target_link_libraries(AllTests PRIVATE 
mazer
SDL2::SDL2
${SDL2_TTF_TARGET}
${SDL2_IMAGE_TARGET}
${SDL2_MIXER_TARGET}
unofficial-sodium::sodium
cppgamelib::cppgamelib
${LUA_LIBRARIES}
PRIVATE
GTest::gmock_main
tinyxml2::tinyxml2
)

# Set include directory for allTests executable
target_include_directories(AllTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})


# Copy test files in the output folder of the AllTests target
add_custom_command(
  TARGET AllTests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_CURRENT_SOURCE_DIR}/testdata/"
          "$<TARGET_FILE_DIR:AllTests>"
)

# Note: We set the working dir of the test to the target output folder otherwise is runs tests from builddir and can't find the configuration files as they are not in the build dir, but the target dir (eg. Release/ etc)
add_test(NAME MainTests COMMAND AllTests WORKING_DIRECTORY $<TARGET_FILE_DIR:AllTests>)


install(TARGETS mazer 
EXPORT mazer # produce target exports for this target
FILE_SET api 
DESTINATION include/mazer
INCLUDES DESTINATION include)

# Create/write the targets exports to the specified file name in the lib directory (as this is where downstream packages look for it via find_package())

install(EXPORT mazer
DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mazer
NAMESPACE mazer::
)

# Write a basic package config version file for us please
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/mazer/mazerConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

# set a variable for with location of where we are going to install the config package to

set(ConfigPackageLocation lib/cmake/mazer)

# install the package config file (this imports the exported target file previously created) and the version file
install(
  FILES
    cmake/mazerConfig.cmake # This file must be created by us
    "${CMAKE_CURRENT_BINARY_DIR}/mazer/mazerConfigVersion.cmake"
  DESTINATION
    ${ConfigPackageLocation}
  COMPONENT
    Devel
)
