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

这个部分代表的是哈希表的数据结构，其中kv_arr[TABLE_SIZE]为存储键值对的struct数组，而fill_num代表此哈希表内有多少个元素，next_offset相当于溢出桶的位置，pm_flag是**溢出桶空间回收**的相关变量，用来分辨此桶是否为空闲的，空闲时为0，占用时为1

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
接下来介绍**私有函数**的部分

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
`void split()`

这个函数用来进行桶的分裂，是私有函数当中最为重要的函数。
```
void PMLHash::split()
{
    // fill the split table
    vector<entry> temp_arr;
    int hash_num = (1 << meta->level) * HASH_SIZE * 2;
    pm_table *split_table = &table_arr[meta->next];
    pm_table *previous = NULL;
    while (true)
    {
        for (uint64_t i = 0; i < split_table->fill_num; i++)
        {
            uint64_t hash_value = hashFunc(split_table->kv_arr[i].key, hash_num);
            // move to the new table
            if (hash_value != meta->next)
            {
                pm_table *new_table = &table_arr[hash_value];
                entry en = {
                    key : split_table->kv_arr[i].key,
                    value : split_table->kv_arr[i].value
                };
                insert_bucket(new_table, en);
            }
            // stay in the old table, move to temp_arr first
            else
            {
                entry en = {
                    key : split_table->kv_arr[i].key,
                    value : split_table->kv_arr[i].value
                };
                temp_arr.push_back(en);
            }
        }
        if (split_table->next_offset == 0) {
            split_table->pm_flag = 0;
            if(previous != NULL)
               meta->overflow_num--;
            break;
        }
        if(previous != NULL)
            meta->overflow_num--;
        previous = split_table;
        split_table = (pm_table *)(split_table->next_offset);
        previous->pm_flag = 0;
        previous->next_offset = 0;
        
    }
    pm_table* p = &table_arr[meta->next];
    for (size_t i = 0; i < temp_arr.size(); i++)
    {
        insert_bucket(p, temp_arr[i]);
    }
    //split_table->next_offset = 0;
    meta->next++;
    meta->size++;
    if (meta->next == (uint64_t)((1 << meta->level) * HASH_SIZE))
    {
        meta->next = 0;
        meta->level++;
    }
    // pmem_persist(start_addr, FILE_SIZE);
}
```
该函数用来在负载因子`((double)(meta->total) / (double)(TABLE_SIZE * meta->size)`大于0.9的情况下触发分裂操作，使用**轮询**的方式来进行分裂，meta的next成员代表的是下一个要被分裂的桶，在进行分裂时，计算出这个桶中元素将要被分裂到的两个桶的下标（包括原来所在的该桶以及一个新的桶），将分到新桶的元素插入到新桶当中，而将要保留在原来这个桶的元素先压入到一个temp_arr的vector当中，同时将原来这个桶的溢出桶全部释放，即将pm_flag置0，next_offset置0，再使用insert_bucket函数来插入到哈希桶中。

接着是public部分的函数，主要介绍增删查改的函数：
```
int PMLHash::insert(const uint64_t &key, const uint64_t &value)
{
    uint64_t hash_value = hashFunc(key, (1 << meta->level) * HASH_SIZE);
    if (hash_value < meta->next)
        hash_value = hashFunc(key, (1 << meta->level) * HASH_SIZE * 2);
    pm_table *table = table_arr + hash_value;
    entry en{
        key : key,
        value : value
    };
    meta->total++;
    int flag = insert_bucket(table, en);
    if ((double)(meta->total) / (double)(TABLE_SIZE * meta->size) > 0.9)
        split();
    // if (flag == 0)
        // pmem_persist(start_addr, FILE_SIZE);
    return flag;
}
```

插入的操作是首先找到插入的桶的位置，这里由于可能已经分裂过一次了，所以需要比较hash值和下一个要分裂的桶的下标，**如果小于下标则是已经分裂过了，要用更新后的hash_size来重新确定这一个元素应该插入的桶**，否则仍然插入原来计算出的hash值对应的桶，在找到了要插入的桶之后调用insert_bucket函数，再判断负载因子`((double)(meta->total) / (double)(TABLE_SIZE * meta->size)`是否大于0.9，大于则触发分裂操作。

---

```
int PMLHash::remove(const uint64_t &key)
{
    uint64_t hash_value = hashFunc(key, (1 << meta->level) * HASH_SIZE);
    if (hash_value < meta->next)
        hash_value = hashFunc(key, (1 << meta->level) * HASH_SIZE * 2);
    pm_table *p = &table_arr[hash_value];
    pm_table *temp, *previous_table;
    int tag;
    while (true)
    {
        for (uint64_t i = 0; i < p->fill_num; i++)
        {
            if (p->kv_arr[i].key == key)
            {
                //move the last element to the tag place, and the total of elements substract 1;
                temp = p;
                //move to the last pm_table
                while (p->next_offset)
                {
                    previous_table = p;
                    p = (pm_table *)p->next_offset;
                }
                //move the last element to the tagged place, and the last pm_table delete the last one element
                temp->kv_arr[i].key = p->kv_arr[p->fill_num - 1].key;
                temp->kv_arr[i].value = p->kv_arr[p->fill_num - 1].value;
                p->fill_num--;
                meta->total--;
                //the last pm_table is empty and need to be removed
                if (p->fill_num == 0){
                    if((uint64_t)p >= (uint64_t)overflow_addr)
                       meta->overflow_num--;
                    meta->total--;
                    previous_table->next_offset = 0;
                    p->pm_flag = 0;
                }
                // pmem_persist(start_addr, FILE_SIZE);
                return 0;
            }
        }
        if (p->next_offset == 0)
            break;
        previous_table = p;
        p = (pm_table *)p->next_offset;
    }
    return -1;
}
```

删除的操作也与插入的思路有类似之处，首先用前述办法确定要被删除的元素会存放在哪一个桶内，然后沿着溢出桶链进行查找，如果找到了要被删除的元素，则取出最后一个桶的最后一个元素来填充到这个被比中的元素的位置，同时检验最后一个桶是否为空，如果为空，则将上一个桶的next_offset设为0，最后一个桶的pm_flag设为0，即将该溢出桶释放，返回0代表已经删除了目标元素，如果没有找到要被删除的元素则返回-1。

---

```
int PMLHash::search(const uint64_t &key, uint64_t &value)
{
    uint64_t hash_value = hashFunc(key, (1 << meta->level) * HASH_SIZE);
    if (hash_value < meta->next)
        hash_value = hashFunc(key, (1 << meta->level) * HASH_SIZE * 2);
    pm_table *p = &table_arr[hash_value];
    while (true)
    {
        //search the temp pm_table;
        for (uint64_t i = 0; i < p->fill_num; i++)
        {
            if (p->kv_arr[i].key == key)
            {
                value = p->kv_arr[i].value;
                return 0;
            }
        }
        //if this table is full-filled, switch to the next_offset
        if (p->next_offset)
            p = (pm_table *)p->next_offset;
        //else break the loop
        else
            break;
    }
    return -1;
}
```
查询也是类似思路，找到对应哈希桶下标之后遍历目标哈希桶链，找到要查找的元素则返回0，没有找到返回-1

```
int PMLHash::update(const uint64_t &key, const uint64_t &value)
{
    uint64_t hash_value = hashFunc(key, (1 << meta->level) * HASH_SIZE);
    if (hash_value < meta->next)
        hash_value = hashFunc(key, (1 << meta->level) * HASH_SIZE * 2);
    pm_table *p = &table_arr[hash_value];
    while (true)
    {
        //search the temp pm_table;
        for (uint64_t i = 0; i < p->fill_num; i++)
        {
            if (p->kv_arr[i].key == key)
            {
                p->kv_arr[i].value = value;
                // pmem_persist(start_addr, FILE_SIZE);
                return 0;
            }
        }
        //if this table is full-filled, switch to the next_offset
        if (p->next_offset)
            p = (pm_table *)p->next_offset;
        //else break the loop
        else
            break;
    }
    return -1;
}
```

## 功能运行结果

-

## YCSB测试
>YCSB是雅虎开源的一款通用的性能测试工具。主要用在云端或服务器端的性能评测分析，可以对各类非关系型数据库产品进行相关的性能测试和评估。
>>给定的数据集每行操作由操作类型和数据组成，由于项目使用8字节键值，所以在读取数据的时候直接将前8字节的数据复制进去即可，键和值内容相同。
>>>本项目中测试为benchmark，数据文件在benchmark文件夹下

1.10w-rw-0-100-load代表数据量10万，读写比0比100，是load阶段的数据集
2.运行流程分load和run，load用于初始化数据库，run为真正运行阶段。
   load：初始化数据库，只有插入操作。
   run：运行性能测试，有增删查改操作。

实现了OpenDir，Load，Run三个函数:

OpenDir用来打开benchmark文件夹并且列出需要执行的数据文件的名称

Load函数读取文件数据并进行插入操作，统计插入的个数并且计算所需时间

Run函数通过读入行的内容来判断要执行什么操作，同时对执行四种操作的次数计数，并计算总体的执行时间和OPS

main函数在执行了一组Load和Run操作之后需要删除NVM文件来保证结果的正确以及程序的正常运行
### 测试流程

- 首先将文件夹benchmark中的文件名读入，然后将属于同个读写比的load和run文件放在一起执行。在load阶段全为插入操作。在run阶段每读入一条语句，通过操作符来进行对应操作，记录是否操作失败、操作数、消耗时间。

### 测试结果

- ![Aaron Swartz](https://raw.githubusercontent.com/MxEmerson/2020-Fall-DBMS-Project/pengw/src/1.png?token=AMBSXEIKZBMG2ZEKXZVG4P27447IA)