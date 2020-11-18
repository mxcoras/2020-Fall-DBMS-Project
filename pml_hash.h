#include <libpmem.h>
#include <stdint.h>
#include <iostream>
#include <unistd.h>
#include <memory.h>
#include <vector>

#define TABLE_SIZE 16 // adjustable
#define HASH_SIZE  16 // adjustable
#define FILE_SIZE 1024 * 1024 * 16 // 16 MB adjustable

using namespace std;

typedef struct metadata {
    size_t size;            // the size of whole hash table array 
    size_t level;           // level of hash
    uint64_t next;          // the index of the next split hash table
    uint64_t overflow_num;  // amount of overflow hash tables 
} metadata;

// data entry of hash table
typedef struct entry {
    uint64_t key;
    uint64_t value;
} entry;

// hash table
typedef struct pm_table {
    entry kv_arr[TABLE_SIZE];   // data entry array of hash table
    uint64_t fill_num;          // amount of occupied slots in kv_arr
    uint64_t next_offset;       // the file address of overflow hash table 
} pm_table;

// persistent memory linear hash
class PMLHash {
private:
    void* start_addr;      // the start address of mapped file
    void* overflow_addr;   // the start address of overflow table array
    metadata* meta;        // virtual address of metadata
    pm_table* table_arr;   // virtual address of hash table array

    void split();
    uint64_t hashFunc(const uint64_t &key, const size_t &hash_size);
    pm_table* newOverflowTable(uint64_t &offset);

public:
    PMLHash() = delete;
    PMLHash(const char* file_path);
    ~PMLHash();

    int insert(const uint64_t &key, const uint64_t &value);
    int search(const uint64_t &key, uint64_t &value);
    int remove(const uint64_t &key);
    int update(const uint64_t &key, const uint64_t &value);
};