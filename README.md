# 2020-Fall-DBMS-Project

This is the project of DBMS course in 2020 Fall.

release 分支共有两个，master 分支为实现了基础功能的代码，reclaim 分支为实现了空闲溢出桶回收的分支。thread 分支没有开发完成。

## 开发环境

- Ubuntu 20.04.1 LTS
- GNU C++ 9.3.0
- GNU Make 4.2.1
- GNU CMake 3.18.5
- pmdk 1.10

## 文件与目录

- `benchmark/`: YCSB workload
- `docs/`: 文档目录，包含项目要求与报告（报告仅在 master 分支）
- `src/`: 项目源代码
  - `main.cc`: 程序入口，运行 YCSB 测试
  - `pml_hash.cc`: 线性哈希类的实现
  - `pml_hash.h`: 线性哈希类与一些常量的声明

## 编译与运行

```sh
mkdir build && cd build
cmake ../
make
./pmlhash
```
