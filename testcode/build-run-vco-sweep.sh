#!/bin/bash
gcc -o3 vco-sweep-batched.c -o test-vco-batch
sudo taskset -c 2 nice -n -20 ./test-vco-batch
