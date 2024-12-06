# Exercise 1
* `w01-byte` bytes/second: 1630.95 byte/sec
* `w02-byte` bytes/second: 286224 byte/sec
* `w03-byte` bytes/second: 3.42613e+07 byte/sec

# Exercise 2
### Scenario 1:
* How many array elements can fit into a cache block?
<!-- Fill this in -->
Type of array elements are Integer which is 4 bytes, block size is 8 bytes => 2 array elements can fit into a cache line.
* What combination of parameters is producing the hit rate you observe? Think about the sizes of each of the parameters.
<!-- Fill this in -->
với các thuộc tính về array và cache trong scenery 1, ta có:
Number of block = 1 => cache chỉ chứa 1 line và line đó chứa 2 array elements.
Step size = 1 => 32 array elements từ arr[0] -> arr[31] được truy cập tuần tự liên tục.
Khi truy cập arr[0], cache miss -> arr[1] thuộc cùng line với arr[0] => cache hit -> arr[2] do trong cache memory chỉ có 1 line nên cache miss và line sẽ được thay thế.
* What is our hit rate if we increase Rep Count arbitrarily? Why?
<!-- Fill this in -->
Thay đổi Rep Count sẽ không làm thay đổi hit rate vì sau 1 vòng lặp như trên thì hit rate luôn là 0.5
### Scenario 2:
* What combination of parameters is producing the hit rate you observe? Think about the sizes of each of the parameters.
<!-- Fill this in -->
Trong scenario 2 cũng chỉ có 1 line trong cache và line này chứa được 2 array elements.
Trong scenario này, step size = 27 => sau khi miss ở lần truy cập arr[0]. arr[27] sẽ được truy cập, nghĩa là line chứa arr[0] sẽ bị thay thế  và đây sẽ tiếp tục là cache miss.
* What happens to our hit rate if we increase the number of blocks and why?
<!-- Fill this in -->
line (block) trong cache, sau khi bị miss ở lần truy cập đầu tiên vào arr[0], cache sẽ tiếp tục miss ở lần truy cập đầu tiên vào arr[27], nhưng lúc này cache có đủ không gian để lưu cả arr[0] và arr[27] trong các block riêng biệt. Nhờ đó, các lần truy cập tiếp theo đến arr[0] và arr[27] sẽ là cache hit. Kết quả là tỷ lệ hit sẽ tăng lên, vì nhiều phần tử của mảng có thể được lưu giữ trong cache mà không bị thay thế ngay lập tức.

### Scenario 3:
* Choose a `number of blocks` greater than `1` and determine the smallest `block size` that uses every block *and* maximizes the hit rate given the parameters above. Explain why.
Number of blocks: 4
Block Size: 8
- Với number of blocks là 4 và block size là 8 byte, cache có thể lưu trữ tổng cộng 4 blocks x 8 byte = 32 byte, tương đương với 8 phần tử của mảng.
- Lần đầu tiên truy cập arr[0], sẽ là cache miss vì cache chưa chứa dữ liệu. arr[0] và arr[1] sẽ được nạp vào block 1. Lần truy cập arr[2], tiếp tục là cache miss, nhưng lần này arr[2] và arr[3] sẽ được nạp vào block 2. Tương tự, lần truy cập arr[4] sẽ nạp thêm arr[4] và arr[5] vào block 3, và arr[6] sẽ được nạp vào block 4. Sau khi cache đã chứa đủ 4 block, các lần truy cập tiếp theo như arr[0], arr[2],.... sẽ là cache hit, vì các phần tử này đã được lưu trong cache.
=> Tỷ lệ hit sẽ tăng lên trong lần lặp thứ hai vì các phần tử mảng đã được nạp vào cache từ vòng lặp đầu tiên. Do đó, các phần tử sẽ không phải nạp lại từ bộ nhớ chính trong vòng lặp tiếp theo, dẫn đến cache hit.

# Exercise 3
* Order the functions from fastest to slowest, and explain why each function's ranking makes sense using your understanding of how the cache works. Some functions might have similar runtimes. If this is the case, explain why.
<!-- Fill this in -->
ijk:	n = 1000, 0.194 Gflop/s
ikj:	n = 1000, 0.179 Gflop/s
jik:	n = 1000, 0.221 Gflop/s
jki:	n = 1000, 0.570 Gflop/s
kij:	n = 1000, 0.382 Gflop/s
kji:	n = 1000, 0.397 Gflop/s


