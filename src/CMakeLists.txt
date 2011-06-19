
include_directories(
    ${json2id3_SOURCE_DIR}/src
    ${TAGLIB_INCLUDE_DIR}
    ${LIBJSON_INCLUDE_DIR}
    )
add_executable(json2id3 main.cpp)
target_link_libraries(json2id3 ${TAGLIB_LIBRARIES} ${LIBJSON_LIBRARIES})
