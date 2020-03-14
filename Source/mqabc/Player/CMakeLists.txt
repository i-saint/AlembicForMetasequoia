string(REGEX MATCH "([0-9]+)" MQ_VERSION "${CMAKE_CURRENT_SOURCE_DIR}")
set(MQSDK "mqsdk${MQ_VERSION}")
set(MQABC_PLAYER "mqabcPlayer${MQ_VERSION}")
find_package(Metasequoia${MQ_VERSION})

file(GLOB sources ../*.cpp ../*.h *.cpp *.h)
add_library(${MQABC_PLAYER} MODULE ${sources})
set_target_properties(${MQABC_PLAYER} PROPERTIES BUNDLE ON)
set_target_properties(${MQABC_PLAYER} PROPERTIES BUNDLE_EXTENSION "plugin")
set_target_properties(${MQABC_PLAYER} PROPERTIES OUTPUT_NAME "mqabcPlayer")

add_dependencies(${MQABC_PLAYER} MeshUtils ${MQSDK})
target_include_directories(${MQABC_PLAYER}
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/mqabc
        ${MQSDK_DIR}
        ${ALEMBIC_INCLUDE_DIRS}
)
target_link_libraries(${MQABC_PLAYER}
    PRIVATE
        MeshUtils
        ${MQSDK}
        ${ALEMBIC_LIBRARIES}
        ${COCOA_LIBRARY}
)

install(DIRECTORY $<TARGET_BUNDLE_DIR:${MQABC_PLAYER}> DESTINATION ${DIST_DIR})
