# If not stated otherwise in this file or this component's license file the
# following copyright and licenses apply:
#
# Copyright 2020 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(PLUGIN_NAME AVInput)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})
set(PLUGIN_IMPLEMENTATION ${MODULE_NAME}Implementation)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(${NAMESPACE}Helpers REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)

if (USE_THUNDER_R4)
    find_package(${NAMESPACE}COM REQUIRED)
else ()
    find_package(${NAMESPACE}Protocols REQUIRED)
endif (USE_THUNDER_R4)

set(PLUGIN_AVINPUT_STARTUPORDER "" CACHE STRING "To configure startup order of AVInput plugin")

# -----------------------------------------------------------------------
# DS_COMRPC: build against the DeviceSettings Thunder plugin via COM-RPC
#            (sources in DS_COMRPC/, no libds / IARM dependency)
# DS_IARM:   build against libds / IARM directly
#            (sources in DS_IARM/)
#
# Select at configure time:
#   cmake -DDS_COMRPC=ON    (default: OFF -> DS_IARM is used)
# In Yocto:
#   EXTRA_OECMAKE += "-DDS_COMRPC=ON"
# -----------------------------------------------------------------------
option(DS_COMRPC
    "Build AVInput using the DeviceSettings COM-RPC plugin (DS_COMRPC/). OFF = build using libds/IARM (DS_IARM/)."
    OFF)

# ---------------------------------------------------------------------------
# Source selection: no compile-time macro guards needed.
# Each subdirectory contains a self-contained, guard-free implementation.
# ---------------------------------------------------------------------------
if (DS_COMRPC)
    message(STATUS "AVInput: DS_COMRPC=ON -- building from DS_COMRPC/")
    set(DS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/DS_COMRPC)
else()
    message(STATUS "AVInput: DS_COMRPC=OFF -- building from DS_IARM/ (libds / IARM)")
    set(DS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/DS_IARM)
endif()

# Shared sources (no libds dependency — used by both DS_IARM and DS_COMRPC)
set(AVINPUT_SHARED_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/AVInputUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Module.cpp)

add_library(${MODULE_NAME} SHARED
    ${DS_SOURCE_DIR}/AVInput.cpp
    ${AVINPUT_SHARED_SOURCES})

target_include_directories(${MODULE_NAME} PRIVATE ${DS_SOURCE_DIR})
target_include_directories(${MODULE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_compile_definitions(${MODULE_NAME} PRIVATE MODULE_NAME=Plugin_${PLUGIN_NAME})

set_source_files_properties(${DS_SOURCE_DIR}/AVInput.cpp PROPERTIES COMPILE_FLAGS "-fexceptions")

add_library(${PLUGIN_IMPLEMENTATION} SHARED
    ${DS_SOURCE_DIR}/AVInputImplementation.cpp
    ${AVINPUT_SHARED_SOURCES})

target_include_directories(${PLUGIN_IMPLEMENTATION} PRIVATE ${DS_SOURCE_DIR})
target_include_directories(${PLUGIN_IMPLEMENTATION} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(${PLUGIN_IMPLEMENTATION} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

if (USE_THUNDER_R4)
    target_link_libraries(${PLUGIN_IMPLEMENTATION} PRIVATE ${NAMESPACE}COM::${NAMESPACE}COM)
else ()
    target_link_libraries(${PLUGIN_IMPLEMENTATION} PRIVATE ${NAMESPACE}Protocols::${NAMESPACE}Protocols)
endif (USE_THUNDER_R4)

if (RDK_SERVICE_L2_TEST)
   find_library(TESTMOCKLIB_LIBRARIES NAMES TestMocklib)
   if (TESTMOCKLIB_LIBRARIES)
       message ("linking mock libraries ${TESTMOCKLIB_LIBRARIES} library")
       target_link_libraries(${PLUGIN_IMPLEMENTATION} PRIVATE ${TESTMOCKLIB_LIBRARIES})
   else (TESTMOCKLIB_LIBRARIES)
       message ("Require ${TESTMOCKLIB_LIBRARIES} library")
   endif (TESTMOCKLIB_LIBRARIES)
endif (RDK_SERVICE_L2_TEST)

# ---------------------------------------------------------------------------
# Link dependencies depend on which implementation is selected
# ---------------------------------------------------------------------------
if (DS_COMRPC)
    # COM-RPC path: entservices-helpers provides DeviceSettingsClientHelper.h
    target_link_libraries(${PLUGIN_IMPLEMENTATION}
        PUBLIC
            ${NAMESPACE}Plugins::${NAMESPACE}Plugins
            ${NAMESPACE}::${NAMESPACE}Helpers)
    target_link_libraries(${MODULE_NAME}
        PUBLIC
            ${NAMESPACE}Plugins::${NAMESPACE}Plugins
            CompileSettingsDebug::CompileSettingsDebug
            ${NAMESPACE}Definitions::${NAMESPACE}Definitions
            ${NAMESPACE}::${NAMESPACE}Helpers)
else()
    # libds / IARM path
    find_package(DS)
    find_package(IARMBus)

    target_include_directories(${MODULE_NAME}          PRIVATE ${DS_INCLUDE_DIRS})
    target_include_directories(${MODULE_NAME}          PRIVATE ${IARMBUS_INCLUDE_DIRS})
    target_include_directories(${PLUGIN_IMPLEMENTATION} PRIVATE ${DS_INCLUDE_DIRS})
    target_include_directories(${PLUGIN_IMPLEMENTATION} PRIVATE ${IARMBUS_INCLUDE_DIRS})

    target_link_libraries(${PLUGIN_IMPLEMENTATION}
        PUBLIC
            ${NAMESPACE}Plugins::${NAMESPACE}Plugins
            ${IARMBUS_LIBRARIES}
            ${DS_LIBRARIES}
            ${NAMESPACE}::${NAMESPACE}Helpers)
    target_link_libraries(${MODULE_NAME}
        PUBLIC
            ${NAMESPACE}Plugins::${NAMESPACE}Plugins
            CompileSettingsDebug::CompileSettingsDebug
            ${NAMESPACE}Definitions::${NAMESPACE}Definitions
            ${IARMBUS_LIBRARIES}
            ${DS_LIBRARIES}
            ${NAMESPACE}::${NAMESPACE}Helpers)
endif()

install(TARGETS ${PLUGIN_IMPLEMENTATION} DESTINATION lib/${STORAGE_DIRECTORY}/plugins)
install(TARGETS ${MODULE_NAME} DESTINATION lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})
