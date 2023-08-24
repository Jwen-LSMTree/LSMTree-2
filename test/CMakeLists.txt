include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(sequence_number_tests
        ../src/SequenceNumber.cpp
        sequence_number.cpp
)

target_link_libraries(sequence_number_tests gtest gtest_main)
