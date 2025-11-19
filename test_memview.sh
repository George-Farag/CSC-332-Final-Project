#!/bin/bash

echo "=== TEST CASES FOR memview ==="

# Test 1: No arguments
echo -e "\n[TEST 1] No arguments"
./memview 2>&1

# Test 2: File does not exist
echo -e "\n[TEST 2] Non-existent file"
./memview nofile.txt 2>&1

# Test 3: Empty file
echo -e "\n[TEST 3] Empty file"
touch empty.txt
./memview empty.txt 2>&1

# Test 4: Small file
echo -e "\n[TEST 4] Small file"
echo "hello" > small.txt
./memview small.txt

# Test 5: Normal multi-line file
echo -e "\n[TEST 5] Normal text file"
echo -e "line1\nline2\nline3" > normal.txt
./memview normal.txt

# Test 6: Permission denied
echo -e "\n[TEST 6] Permission denied"
chmod 000 small.txt
./memview small.txt 2>&1
chmod 644 small.txt

# Test 7: Large file
echo -e "\n[TEST 7] Large file"
dd if=/dev/urandom of=big.bin bs=1024 count=4
./memview big.bin

# Test 8: Virtual memory map
echo -e "\n[TEST 8] Virtual memory map presence"
./memview normal.txt | grep "\[ Process Virtual Memory Map" && echo "✓ Found"

# Test 9: Shared memory absent
echo -e "\n[TEST 9] No shared memory"
ipcs -m | awk 'NR>3 {print}'  # display table
./memview normal.txt

# Test 10: Shared memory present
echo -e "\n[TEST 10] Shared memory present"
SEG=$(ipcmk -M 4096 | awk '{print $4}')
./memview normal.txt

# Test 11: Shared memory persists
echo -e "\n[TEST 11] Shared memory persistence"
./memview normal.txt

# Test 12: mmap failure (simulate by zeroing file)"
echo -e "\n[TEST 12] mmap failure simulation"
touch badfile
chmod 000 badfile
./memview badfile 2>&1
chmod 644 badfile

echo -e "\n=== END OF TEST CASES ==="
