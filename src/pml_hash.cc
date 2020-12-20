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
    size_t mapped_len;
    if ((start_addr = pmem_map_file(file_path, FILE_SIZE, PMEM_FILE_CREATE, 0666, &mapped_len, &is_pmem)) == NULL)
    {
        perror("pmem_map_file");
        exit(1);
    }
    else
    {
        overflow_addr = (void *)((uint64_t)start_addr + FILE_SIZE / 2);
        table_arr = (pm_table *)((uint64_t)start_addr + sizeof(metadata));
        meta = (metadata *)start_addr;
        overflow_arr = (pm_table *)overflow_addr;
        if (meta->size == 0)
            meta->size = 16;
    }
}

/**
 * PMLHash::~PMLHash 
 * 
 * unmap and close the data file
 */
PMLHash::~PMLHash()
{
    pmem_persist(start_addr, FILE_SIZE);
    pmem_unmap(start_addr, FILE_SIZE);
}

/**
 * PMLHash 
 * 
 * @param  {pm_table *} addr : address of hash table to be inserted
 * @param  {entry} en        : the entry to insert
 * @return {int}             : success: 0. fail: -1
 * 
 * insert a key-value pair into a specified address
 */
int PMLHash::insert_bucket(pm_table *addr, entry en)
{
    //cout << "insert_bucket"<< endl;
    //cout << en.value <<endl; 
    pm_table *table = addr;
    
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
    //table->next_offset = 0;
    // pmem_persist(start_addr, FILE_SIZE);
    return 0;
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
    vector<entry> temp_arr;
    int hash_num = (1 << meta->level) * HASH_SIZE * 2;
    pm_table *split_table = &table_arr[meta->next];
    pm_table *previous = NULL;
   // pm_table *next;
    while (true)
    {
        //cout << "spliting" << endl;
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

    // fill the old table
    // split_table = &table_arr[meta->next];
    // split_table->fill_num = 0;
    // split_table->pm_flag = 1;
    pm_table* p = &table_arr[meta->next];
    for (size_t i = 0; i < temp_arr.size(); i++)
    {
        // if (split_table->fill_num >= 16)
        // {
        //     next = find_first_free_table();
        //     split_table->next_offset=(uint64_t)next;
        //     split_table = next;
        //     split_table->fill_num = 0;
        //     split_table->pm_flag = 1;
        // }
        // split_table->kv_arr[split_table->fill_num++] = temp_arr[i];
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
// pm_table *PMLHash::newOverflowTable(uint64_t &offset)
// {
//     if (offset > (FILE_SIZE - sizeof(pm_table)))
//         return NULL;
//     pm_table *new_overflow_table = (pm_table *)((uint64_t)start_addr + offset);
//     meta->overflow_num++;
//     return new_overflow_table;
// }

pm_table * PMLHash::find_first_free_table(){
    for(uint64_t i = 0; i <= meta->overflow_num; i++) {
        if(overflow_arr[i].pm_flag == 0) 
           return &overflow_arr[i];
    }
    if(meta->overflow_num + 1 >= (FILE_SIZE / 2) / sizeof(pm_table)) {
        perror("No available overflow table");
        exit(1);
    }
    return &overflow_arr[meta->overflow_num + 1];
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