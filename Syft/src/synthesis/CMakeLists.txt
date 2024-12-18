set (CMAKE_CXX_STANDARD 14)
#set (CMAKE_BUILD_TYPE Debug)

add_library(synthesis SHARED Common.cpp DFA.cpp InputFirst.cpp syn.cpp SSNFA.cpp CoRDFA_syn.cpp)
include_directories(${CUDD_INCLUDE_DIRS})
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(synthesis PRIVATE BUILD_DEBUG)
    target_compile_definitions(Syft PRIVATE BUILD_DEBUG)
    target_compile_definitions(play PRIVATE BUILD_DEBUG)
endif()

add_executable(play synStrategy.cpp)
add_executable(Syft main.cpp)

target_link_libraries(Syft synthesis ${CUDD_LIBRARIES})
target_link_libraries(play synthesis ${CUDD_LIBRARIES})
#target_link_libraries(synth synthesis ${CUDD_LIBRARIES})

#install(TARGETS bdd synth synthesis
install(TARGETS Syft synthesis
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  )

INSTALL (
  DIRECTORY ${CMAKE_SOURCE_DIR}/src/synthesis
    DESTINATION include
    FILES_MATCHING PATTERN "*.h*")
