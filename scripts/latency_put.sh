#!/bin/bash

N=5
ENTRY_COUNT=(10000 50000 100000 500000 1000000)
TEST_COUNT=5

echo "[ latency_put ]"
echo -e "TEST_COUNT = ${TEST_COUNT}"

for ((i = 0; i < N; i++)); do
  echo -e "\n"
  echo -e "ENTRY_COUNT = ${ENTRY_COUNT[i]}"
  ../build/latency_put "${ENTRY_COUNT[i]}" $TEST_COUNT
done
