# 2020-Fall-DBMS-Project

## NVM环境模拟安装

-

## NVM环境测试

-

## 安装PMDK库

-

## 代码展示
### 底层设计  

本次实验的主体部分包含在pml_hash.cc和pml_hash.h两个文件当中。构建了整个底层部分，负责对数据的操作。  

`pml_hash.h`当中是本次实验的数据结构：

---
```
typedef struct metadata
{
    size_t size;           // the size of whole hash table array
    size_t level;          // level of hash
    uint64_t next;         // the index of the next split hash table
    uint64_t overflow_num; // amount of overflow hash tables
    uint64_t total;        // the number of total elements
    uint64_t index;        // the index of the first free overflow bucket;
} metadata;
```

这个结构的功能主要是用来存储整个线性哈希的元数据，total代表表中总共有多少个元素，index代表下一个空溢出桶的下标。

---

`entry:uint64_t key,uint64_t value `

这个结构为哈希表中的键值

---
```
typedef struct pm_table
{
    entry kv_arr[TABLE_SIZE]; // data entry array of hash table
    uint64_t fill_num;        // amount of occupied slots in kv_arr
    uint64_t next_offset;     // the file address of overflow hash table
    uint64_t pm_flag;         // 1 -- occupied, 0 -- available
} pm_table;
```

这个部分代表的是哈希表的数据结构，其中kv_arr[TABLE_SIZE]为存储键值对的struct数组，而fill_num代表此哈希表内有多少个元素，next_offset相当于溢出桶的位置，pm_flag是溢出桶空间回收的相关变量，用来分辨此桶是否为空闲的，空闲时为0，占用时为1
---
接着是主要的实现部分PMLHash

```
class PMLHash
{
private:
    void *start_addr;    // the start address of mapped file
    void *overflow_addr; // the start address of overflow table array
    metadata *meta;      // virtual address of metadata
    pm_table *table_arr; // virtual address of hash table array
    pm_table *overflow_arr; // virtual address of overflow table array

    int insert_bucket(pm_table *addr, entry en);
    void split();
    uint64_t hashFunc(const uint64_t &key, const size_t &hash_size);
    pm_table *newOverflowTable(uint64_t &offset);
    pm_table *find_first_free_table();

public:
    PMLHash() = delete;
    PMLHash(const char *file_path);
    ~PMLHash();

    int insert(const uint64_t &key, const uint64_t &value);
    int search(const uint64_t &key, uint64_t &value);
    int remove(const uint64_t &key);
    int update(const uint64_t &key, const uint64_t &value);
};
```
首先解释一下**私有成员**的部分：
```
    void *start_addr;   
    //start_addr指向的是映射文件的虚拟地址开始位置
    void *overflow_addr;
    //overflow_addr指向溢出桶空间的虚拟地址起始位置
    pm_table *table_arr; 
    //table_arr指向的是哈希桶的起始地址
    pm_table *overflow_arr; 
    //overflow_arr指向的是溢出哈希表的起始位置
```
---
接下来介绍私有函数的部分
---
`int insert_bucket(pm_table *addr, entry en)`
该函数用于在找到对应的哈希桶之后将元素插入到这个哈希桶或者其溢出桶。
```
    while (table->next_offset != 0){
        table = (pm_table *)table->next_offset;
    }
    if (table->fill_num >= 16)
    {
        //uint64_t offset = (FILE_SIZE / 2) + (meta->overflow_num * sizeof(pm_table));
        table->next_offset = (uint64_t)(find_first_free_table());
        meta->overflow_num++;
        // if (table->next_offset == 0)
        //     return -1;
        table = (pm_table *)table->next_offset;
        table->pm_flag = 1;
        table->fill_num = 0;
        table->next_offset = 0;
    }
    table->kv_arr[table->fill_num] = en;
    table->fill_num++;
```
在到达该哈希桶链上的最后一个桶后，需要判断哈希桶是否满了，如果桶满了，就使用find_first_free_table函数生成一个溢出桶并将元素插入新桶，否则插入最后这一个桶。
---
```
    uint64_t hashFunc(const uint64_t &key, const size_t &hash_size)
    //哈希函数，通过key和当前的hash大小来找到对应的桶，hash_size随着桶分裂的轮数增加而变大
```
`pm_table *find_first_free_table()`
最开始是不考虑桶空间回收的找下一个溢出桶位置的newOverflowTable函数，后面实现拓展功能之后通过遍历溢出桶来找到下一个空闲溢出桶
```
    if(overflow_arr[i].pm_flag == 0) 
    //如果pm_flag为0即该桶空闲，就返回这个溢出桶
        return &overflow_arr[i];
```
'void split()'
    
    

## 功能运行结果

-

## YCSB测试

### 测试流程

- 首先将文件夹benchmark中的文件名读入，然后将属于同个读写比的load和run文件放在一起执行。在load阶段全为插入操作。在run阶段每读入一条语句，通过操作符来进行对应操作，记录是否操作失败、操作数、消耗时间。

### 测试结果

- ![Aaron Swartz](https://raw.githubusercontent.com/MxEmerson/2020-Fall-DBMS-Project/pengw/src/1.png?token=AMBSXEIKZBMG2ZEKXZVG4P27447IA)