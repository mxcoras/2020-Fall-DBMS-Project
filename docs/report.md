# 2020-Fall-DBMS-Project
授课老师：冯剑琳<br>

# 一、实验目的
&emsp;本次课程设计要求实现一个课本上所讲解的线性哈希的持久化实现，底层持久化是在模拟的 NVM硬件上面进行。<br>
&emsp;本次实验目的是将基于线性哈希的数据库持久化在NVM上，使用NVM的封装函数进行文件操作，然后文件数据管理通过自己设计的定长页表进行空间管理，为线性哈希的数据库分配和回收空间。 
# 二、NVM环境模拟安装

-

# 三、NVM环境测试

-

# 四、安装PMDK库

-

# 五、代码展示
## 5.1 pmlhash.h
### 定义的常数
---
#define TABLE_SIZE 16              // adjustable
#define HASH_SIZE 16               // adjustable
#define FILE_SIZE 1024 * 1024 * 16 // 16 MB adjustable
---



# 五、功能运行结果

-

# 六、YCSB测试

## 6.1测试流程

- 首先将文件夹benchmark中的文件名读入，然后将属于同个读写比的load和run文件放在一起执行。在load阶段全为插入操作。在run阶段每读入一条语句，通过操作符来进行对应操作，记录是否操作失败、操作数、消耗时间。

## 6.2测试结果

- ![Aaron Swartz](https://raw.githubusercontent.com/MxEmerson/2020-Fall-DBMS-Project/pengw/src/1.png?token=AMBSXEIKZBMG2ZEKXZVG4P27447IA)