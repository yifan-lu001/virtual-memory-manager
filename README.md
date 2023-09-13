# Virtual Memory Manager

This repository implements a virtual memory manager in C which performs paging with two replacement policies.

## Installation

Clone the repository by running the following command-line prompt:
```
git clone https://github.com/yifan-lu001/virtual-memory-manager.git
```

Run ```make``` to build the program, and then run ```./proj3 <allocation-policy> <num-frames> <input-filename>``` to output the transactions made in virtual and physical memory.

Scheduling policies:
```
0 = First In First Out (FIFO)
1 = Third Chance Replacement
```

## Authors

This project was created by Yifan Lu (yifan.lu001@gmail.com) for the CMPSC 473 course at Penn State University.

