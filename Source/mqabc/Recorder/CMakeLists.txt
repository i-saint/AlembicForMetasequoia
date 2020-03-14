string(REGEX MATCH "([0-9]+)" MQ_VERSION "${CMAKE_CURRENT_SOURCE_DIR}")
set(MQSDK "mqsdk${MQ_VERSION}")
set(MQABC_RECORDER "mqabcRecorder${MQ_VERSION}")
find_package(Metasequoia${MQ_VERSION})

file(GLOB sources ../*.cpp ../*.h *.cpp *.h)
add_library(${MQABC_RECORDER} MODULE ${sources})
set_target_properties(${MQABC_RECORDER} PROPERTIES BUNDLE ON)
set_target_properties(${MQABC_RECORDER} PROPERTIES BUNDLE_EXTENSION "plugin")
set_target_properties(${MQABC_RECORDER} PROPERTIES OUTPUT_NAME "mqabcRecorder")

add_dependencies(${MQABC_RECORDER} MeshUtils ${MQSDK})
target_include_directories(${MQABC_RECORDER}
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/mqabc
        ${MQSDK_DIR}
        ${ALEMBIC_INCLUDE_DIRS}
)
target_link_libraries(${MQABC_RECORDER}
    PRIVATE
        MeshUtils
        ${MQSDK}
        ${ALEMBIC_LIBRARIES}
        ${COCOA_LIBRARY}
)

install(DIRECTORY $<TARGET_BUNDLE_DIR:${MQABC_RECORDER}> DESTINATION ${DIST_DIR})
