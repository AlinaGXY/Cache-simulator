# Cache-simulator
Project 4 of Duke course ECE 550

# Arguments List
test file name
number of cache: 1 or 2
write allocate: 1 or 0
RR or LRU: 1 or 0
L1 I unified or split: 1 or 0
L1 I capacity
L1 I linesize
L1 I lines in one set: 0 means fully associative
L1 D capacity
L1 D linesize
L1 D lines in one set: 0 means fully associative
L2 unified or split: 1 or 0
L2 I capacity
L2 I linesize
L2 I lines in one set: 0 means fully associative
L2 D capacity
L2 D linesize
L2 D lines in one set: 0 means fully associative

# Input sample of demo cases
1. Unified L1 and No L2:
L1 is direct mapped, block size of 32 bytes, capacity of 8192 bytes, DO NOT allocate on write miss.

Dinero10000.din 1 0 1 1 8192 32 1 0 0 0 0 0 0 0 0 0 0

2. Split L1 and Unified L2:
L1 I$ direct mapped, block size of 32, capacity of 8192, allocate on write miss.
L1 D$ 2-way set associative, block size of 32, capacity of 8192, allocate on write miss, RND replacement policy.
L2  8-way set associative, block size of 32, capacity of 32768, allocate on write miss, RND replacement policy.

Dinero10000.din 2 1 1 0 8192 32 1 8192 32 2 1 32768 32 8 0 0 0

3. Split L1 and Split L2:
L1 I$ direct mapped, block size of 64, capacity of 8192, allocate on write miss.
L1 D$ 4-way set associative, block size of 64, capacity of 16384, allocate on write miss, LRU replacement policy.
L2 I$ 8-way set associative, block size of 64, capacity of 32768, allocate on write miss, LRU replacement policy.
L2 D$ 16-way set associative, block size of 64, capacity of 65536, allocate on write miss, LRU replacement policy.

DineroFull.din 2 1 0 0 8192 64 1 16384 64 4 0 32768 64 8 65536 64 16

# Code instruction
1.	main.cpp
There are four functions. In “RandomReplace” function, we randomly kick a block and push the new block information to the lines map.
Then in the “leastRecent” function, We use the variable “age” to record whether the block is used recently. So when the set is full, and we want to add new data to the set, we could kick the least recent use instruction/data out of the set.
The “initialize” function is to initialize the information of the cache by using argument that we input.
Finally, main function reads in .din file and executes each operation line by line. 

2.	block.h
There are three classes. The first class, “request” contains the type of the operation, tag, index, and the real address. In the class “line”, it contains the check of whether the block is dirty, or whether it is instruction and the information about its tag and age. In the class “set”, it contains the number of line in the set, and store the information of lines in an unordered map.

3.	cache.h
I declared a “Cache class”. It includes the contents like capacity, block_size, blockNumber, etc. And I transform the address into the tag and index according to cache configuration in “generalAddress” function by using the shift operator and “&” operator.  The two dimensional array “statistic” represents the count of the “read”, ”write”, ”fetch” instructions.
Find function is the core of our code. According to the request (tag and index), we find the corresponding set and search through the set to find whether the tag is in the cache. If it is in the cache, it means “hit”; If we cannot find the tag in the cache, it means “miss”.
When read or fetch miss happens, we have to search the block in the lower level and then place it in the set. If empty space is available, then we can just put it inside the set. Otherwise, we have to perform replacement strategy to decide an old block to be deleted.
When write miss happens, replace old block or not depends on the user setting. 
Here we have to notice that if the deleted old block is dirty, If it is dirty, find this data in the next cache, and update its new data. So a write request should be sent to the lower level cache (if there is). 
