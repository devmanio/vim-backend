cmake_minimum_required(VERSION 3.2)

file(GLOB HEADERS "include/eosio/hackathon_plugin/*.hpp")
add_library( hackathon_plugin
             hackathon_plugin.cpp
             ${HEADERS} )

target_link_libraries( hackathon_plugin appbase fc http_plugin chain_plugin wallet_plugin)
target_include_directories( hackathon_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
target_include_directories( hackathon_plugin PUBLIC ${CMAKE_BINARY_DIR}/contracts )

add_dependencies( hackathon_plugin hackathon )

install( TARGETS
   hackathon_plugin

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
install( FILES ${HEADERS} DESTINATION "include/eosio/hackathon_plugin" )
