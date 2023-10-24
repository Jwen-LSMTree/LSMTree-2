#!/bin/bash

N=5
ENTRY_COUNT=(100000 500000 1000000 5000000 10000000)
DUPLICATE_COUNT=3
TEST_COUNT=5

echo "[ latency_snapshot ]"
echo -e "TEST_COUNT=${TEST_COUNT}"

for ((i = 0; i < N; i++)); do
  echo -e "\n"
  echo -e "ENTRY_COUNT = ${ENTRY_COUNT[i]}"
  ../build/latency_snapshot "${ENTRY_COUNT[i]}" $DUPLICATE_COUNT $TEST_COUNT
done
