#include "pml_hash.h"
/**
 * PMLHash::PMLHash 
 * 
 * @param  {char*} file_path : the file path of data file
 * if the data file exist, open it and recover the hash
 * if the data file does not exist, create it and initial the hash
 */
PMLHash::PMLHash(const char *file_path)
{
    int is_pmem;
    start_addr = pmem_map_file(file_path, FILE_SIZE, PMEM_FILE_CREATE, 0666, &meta->size, &is_pmem);

    if(start_addr == NULL) {
        FILE *f = fopen(file_path, "w+");
        fclose(f);
        start_addr = pmem_map_file(file_path, FILE_SIZE, PMEM_FILE_CREATE, 0666, &meta->size, &is_pmem);
        table_arr = (pm_table*)(start_addr + sizeof(metadata));
        overflow_addr = start_addr + FILE_SIZE / 2;
        meta = (metadata*)start_addr;
        memset(meta, 0, sizeof(metadata));
    }

    else {
    overflow_addr = start_addr + FILE_SIZE / 2;
    table_arr = (pm_table*)(start_addr + sizeof(metadata));
    meta = (metadata*)start_addr;
    }

}
/**
 * PMLHash::~PMLHash 
 * 
 * unmap and close the data file
 */
PMLHash::~PMLHash()
{
    pmem_unmap(start_addr, FILE_SIZE);
}
/**
 * PMLHash 
 * 
 * split the hash table indexed by the meta->next
 * update the metadata
 */
void PMLHash::split()
{
    // fill the split table
    entry temp_arr[TABLE_SIZE];
    int t1 = 0, t2 = 0;
    memset(temp_arr, 0, sizeof(temp_arr));
    int total = 2 << (meta -> level);

    for(int i = 0; i < table_arr[meta -> level].fill_num; i++) {
        int flag = hashFunc(table_arr[meta -> next].kv_arr[i].key, total * 16);

        //move to the new bucket
        if(flag != meta -> next) {
            int tag = hashFunc(table_arr[meta -> next].kv_arr[i].key, 2 * 16 * total);
            table_arr[tag].kv_arr[t1].key = table_arr[meta -> next].kv_arr[i].key;
            table_arr[tag].kv_arr[t1].value = table_arr[meta -> next].kv_arr[i].value;
            table_arr[tag].fill_num++;
            t1++;
        }
        //stay in the old bucket,move to temp_arr first
        else {
            temp_arr[t2].key = table_arr[meta -> next].kv_arr[i].key;
            temp_arr[t2].value = table_arr[meta -> next].kv_arr[i].value;
            t2++;
        }
    }
    
    // fill the new table

    // update the next of metadata
}
/**
 * PMLHash 
 * 
 * @param  {uint64_t} key     : key
 * @param  {size_t} hash_size : the N in hash func: idx = hash % N
 * @return {uint64_t}         : index of hash table array
 * 
 * need to hash the key with proper hash function first
 * then calculate the index by N module
 */
uint64_t PMLHash::hashFunc(const uint64_t &key, const size_t &hash_size)
{
    return (key * 3) % hash_size; 
}

/**
 * PMLHash 
 * 
 * @param  {uint64_t} offset : the file address offset of the overflow hash table
 *                             to the start of the whole file
 * @return {pm_table*}       : the virtual address of new overflow hash table
 */
pm_table *PMLHash::newOverflowTable(uint64_t &offset)
{
    pm_table * new_overflow_table = (pm_table*)(start_addr + offset);
    return new_overflow_table;
}

/**
 * PMLHash 
 * 
 * @param  {uint64_t} key   : inserted key
 * @param  {uint64_t} value : inserted value
 * @return {int}            : success: 0. fail: -1
 * 
 * insert the new kv pair in the hash
 * 
 * always insert the entry in the first empty slot
 * 
 * if the hash table is full then split is triggered
 */
int PMLHash::insert(const uint64_t &key, const uint64_t &value)
{
    uint64_t hashvalue=hashFunc(key,HASH_SIZE);
    pm_table *table=(pm_table*)(table_arr+hashvalue-1);
    uint64_t offset;
    if(table->fill_num<TABLE_SIZE){
        table->kv_arr[table->fill_num].key=key;
        table->kv_arr[table->fill_num].value=value;
        table->fill_num++;
    }
    else{
        offset=(uint64_t)(overflow_addr-start_addr)+meta->overflow_num*sizeof(pm_table);
        if(offset<0.5*FILE_SIZE&&(!table->next_offset)){
            pm_table *new_table=newOverflowTable(offset);
            table->next_offset=meta->overflow_num*sizeof(pm_table);
            new_table->kv_arr[0].key=key;
            new_table->kv_arr[0].value=value;
            new_table->fill_num++;
            new_table->next_offset=0;
            meta->overflow_num++;
        }
        else if(offset<0.5*FILE_SIZE&&(table->next_offset)){
            while(table->next_offset){//whether overflow table exists
                table=(pm_table*)(overflow_addr+table->next_offset);
                if(table->fill_num<TABLE_SIZE){
                    table->kv_arr[table->fill_num].key=key;
                    table->kv_arr[table->fill_num].value=value;
                    table->fill_num++;
                    break;
                }
                else{//create a new table
                    offset=(uint64_t)(overflow_addr-start_addr)+meta->overflow_num*sizeof(pm_table);
                    table+=table->next_offset;
                    offset+=table->next_offset;
                }
            }
        }
        else
        {
            return -1;
        } 
    }
    return 0; //for test
}

/**
 * PMLHash 
 * 
 * @param  {uint64_t} key   : the searched key
 * @param  {uint64_t} value : return value if found
 * @return {int}            : 0 found, -1 not found
 * 
 * search the target entry and return the value
 */
int PMLHash::search(const uint64_t &key, uint64_t &value)
{
    size_t i = 2 << (meta -> level);
    uint64_t t = hashFunc(key, i * 16);
    int len = table_arr[t].fill_num;

    //search the t-th hash table
    for(int o = 0; o < len; o++) {
        if(table_arr[t].kv_arr[o].value == value && table_arr[t].kv_arr[o].key == key) return 0;
    }
    //search the t-th overflow table
    //if the hash table dose not exist the value and the overflow table exists
    if(table_arr[t].fill_num == 16 && table_arr[t].next_offset != 0) {
        pm_table *temp = (pm_table*) table_arr[t].next_offset;
        int overflow_len = temp -> fill_num;
        for(int o = 0; o < overflow_len; o++) {
            if(temp -> kv_arr[o].value == value) return 0;
        }
    }
    return -1;
}

/**
 * PMLHash 
 * 
 * @param  {uint64_t} key : target key
 * @return {int}          : success: 0. fail: -1
 * 
 * remove the target entry, move entries after forward
 * if the overflow table is empty, remove it from hash
 */
int PMLHash::remove(const uint64_t &key)
{
    return 0; //for test
}

/**
 * PMLHash 
 * 
 * @param  {uint64_t} key   : target key
 * @param  {uint64_t} value : new value
 * @return {int}            : success: 0. fail: -1
 * 
 * update an existing entry
 */
int PMLHash::update(const uint64_t &key, const uint64_t &value)
{
    size_t i = 2 << (meta -> level);
    uint64_t t = hashFunc(key, i * 16);
    int len = table_arr[t].fill_num;

    //search the t-th hash table
    for(int o = 0; o < len; o++) {
        if(table_arr[t].kv_arr[o].key == key) {
            table_arr[t].kv_arr[o].value = value;
            return 0;
        }
    }
    //search the t-th overflow table
    //if the hash table dose not exist the value and the overflow table exists
    if(table_arr[t].fill_num == 16 && table_arr[t].next_offset != 0) {
        pm_table *temp = (pm_table*) table_arr[t].next_offset;
        int overflow_len = temp -> fill_num;
        for(int o = 0; o < overflow_len; o++) {
            if(temp -> kv_arr[o].key == key) {
                temp -> kv_arr[o].value = value;
                return 0;
            }
        }
    }
    return -1;
}