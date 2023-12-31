include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

set(SRC_FILES
        ../src/bloom_filter/Bloomfilter.cpp
        ../src/disk/sstable/SSTable2.cpp
        ../src/disk/sstable/BlockLocation.cpp
        ../src/disk/sstable/DataBlockLocation.cpp
        ../src/disk/sstable/SSTableDataLocation.cpp
        ../src/disk/sstable/SSTableId.cpp
        ../src/disk/DiskStorage.cpp
        ../src/disk/LevelNonZero.cpp
        ../src/disk/LevelZero.cpp
        ../src/disk/SearchResult.cpp
        ../src/exception/NoEntryFound.cpp
        ../src/memory/SkipList.cpp
        ../src/Entry.cpp
        ../src/KVStore.cpp
        ../src/SequenceNumber.cpp
        ../src/Util.cpp
        )

add_executable(sequence_number_tests
        ../src/SequenceNumber.cpp
        accuracy/SequenceNumber.cpp
        )

add_executable(skip_list_tests
        ${SRC_FILES}
        ../src/Option.cpp
        accuracy/SkipList.cpp
        )

add_executable(disk_storage_tests
        ${SRC_FILES}
        ../src/Option.cpp
        accuracy/DiskStorage.cpp
        )

add_executable(kvstore_tests
        ${SRC_FILES}
        accuracy/Option.cpp
        accuracy/KVStore.cpp
        )

target_link_libraries(sequence_number_tests gtest gtest_main)
target_link_libraries(skip_list_tests gtest gtest_main)
target_link_libraries(disk_storage_tests gtest gtest_main)
target_link_libraries(kvstore_tests gtest gtest_main)