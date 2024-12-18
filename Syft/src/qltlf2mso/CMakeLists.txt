
project(qltlf2mso)
set (CMAKE_CXX_STANDARD 14)
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

BISON_TARGET(parser ltlparser.y ${CMAKE_CURRENT_BINARY_DIR}/ltlparser.c DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/ltlparser.h)
FLEX_TARGET(lexer ltllexer.l ${CMAKE_CURRENT_BINARY_DIR}/ltllexer.c DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/ltllexer.h)
ADD_FLEX_BISON_DEPENDENCY(lexer parser)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_library(qltlf_translation SHARED ltlf2fol.cpp ltl_formula.c trans.c utility.cpp partfile_util.cpp  ${BISON_parser_OUTPUTS} ${FLEX_lexer_OUTPUTS})

#add_executable(readFormula main.cpp)
add_executable(qltlf2mso translate.cpp)

#target_link_libraries(readFormula qltlf_translation)
target_link_libraries(qltlf2mso qltlf_translation)

install(TARGETS qltlf2mso
  RUNTIME DESTINATION bin
  )
