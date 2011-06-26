
include_directories(
    ${json2id3_SOURCE_DIR}/src
    ${TAGLIB_INCLUDE_DIR}
    ${LIBJSON_INCLUDE_DIR}
    )
link_directories(${json2id3_BINARY_DIR}/src)
add_executable(json2id3 main.cpp)
target_link_libraries(json2id3 ${TAGLIB_LIBRARIES} ${LIBJSON_LIBRARIES})

install(TARGETS json2id3 RUNTIME DESTINATION bin)
