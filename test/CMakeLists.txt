include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

set(SRC_FILES
        ../src/bloom_filter/Bloomfilter.cpp
        ../src/cache/BlockCache.cpp
        ../src/cache/TableCache.cpp
        ../src/disk/sstable/SSTable.cpp
        ../src/disk/sstable/SSTableDataLocation.cpp
        ../src/disk/sstable/SSTableId.cpp
        ../src/disk/DiskStorage.cpp
        ../src/disk/LevelNonZero.cpp
        ../src/disk/LevelZero.cpp
        ../src/disk/Location.cpp
        ../src/disk/SearchResult.cpp
        ../src/exception/NoEntryFound.cpp
        ../src/memory/SkipList.cpp
        ../src/Entry.cpp
        ../src/KVStore.cpp
        ../src/Option.cpp
        ../src/SequenceNumber.cpp
        ../src/Util.cpp
)

add_executable(sequence_number_tests
        ../src/SequenceNumber.cpp
        KVStore/SequenceNumber.cpp
)

add_executable(skip_list_tests
        ${SRC_FILES}
        KVStore/SkipList.cpp
)

add_executable(disk_storage_tests
        ${SRC_FILES}
        KVStore/DiskStorage.cpp
)

target_link_libraries(sequence_number_tests gtest gtest_main)
target_link_libraries(skip_list_tests gtest gtest_main)
target_link_libraries(disk_storage_tests gtest gtest_main)
