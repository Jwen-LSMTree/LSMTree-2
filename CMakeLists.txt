cmake_minimum_required(VERSION 3.15)
project(LSMTree)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-m64)
add_compile_options(-O3)

set(SRC_FILES
        ./src/bloom_filter/Bloomfilter.cpp
        src/sequence_number_filter/SequenceNumberFilter.cpp
        ./src/disk/sstable/SSTable.cpp
        ./src/disk/sstable/SSTableDataLocation.cpp
        ./src/disk/sstable/DataBlockLocation.cpp
        ./src/disk/sstable/BlockLocation.cpp
        ./src/disk/sstable/SSTableId.cpp
        ./src/disk/DiskStorage.cpp
        ./src/disk/LevelNonZero.cpp
        ./src/disk/LevelZero.cpp
        ./src/disk/SearchResult.cpp
        ./src/exception/NoEntryFound.cpp
        ./src/memory/SkipList.cpp
        ./src/Entry.cpp
        ./src/KVStore.cpp
        ./src/Option.cpp
        ./src/SequenceNumber.cpp
        ./src/Util.cpp
)

add_executable(latency_put test/latency/put.cpp ${SRC_FILES})
add_executable(latency_get test/latency/get.cpp ${SRC_FILES}
        src/disk/sstable/SSTable2.cpp)
add_executable(latency_snapshot test/latency/snapshot.cpp ${SRC_FILES})

add_subdirectory(test)
add_subdirectory(test/lib/googletest-1.14.0)
