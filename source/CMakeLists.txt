cmake_minimum_required(VERSION 3.6)

set(Boost_USE_STATIC_LIBS)
find_package(Boost 1.40.0 COMPONENTS graph REQUIRED)
if (NOT Boost_FOUND)
  message(FATAL_ERROR "Cannot find Boost dependencies")
endif()

include_directories("${Boost_INCLUDE_DIRS}")
include_directories("${PROJECT_SOURCE_DIR}/include")

add_library(cgtl "arch_graph.cc" "bsgs.cc" "dbg.cc" "perm.cc" "perm_word.cc"
                 "perm_group.cc" "schreier_sims.cc")

set_target_properties(cgtl PROPERTIES
  LINKER_LANGUAGE CXX
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED ON
  COMPILE_FLAGS "-Wall -pedantic -Wextra"
)
