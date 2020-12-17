#include "pml_hash.h"

int main()
{
    PMLHash hash("/mnt/pmemdir/file");
    for (uint64_t i = 1; i <= HASH_SIZE * TABLE_SIZE * 2; i++)
    {
        hash.insert(i, i);
        cout << "  key: " << i << endl;
        // hash.insert(i, i);
    }
    // for (uint64_t i = 1; i <= HASH_SIZE; i++)
    // {
    //     uint64_t val;
    //     hash.search(i, val);
    //     cout << "key: " << i << "\nvalue: " << val << endl;
    // }
    // for (uint64_t i = HASH_SIZE * TABLE_SIZE + 1;
    //      i <= (HASH_SIZE + 1) * TABLE_SIZE; i++)
    // {
    //     hash.insert(i, i);
    // }
    for (uint64_t i = HASH_SIZE * TABLE_SIZE; i <= HASH_SIZE * TABLE_SIZE * 2; i++)
    {
        uint64_t val;
        if (hash.search(i, val) == -1)
            cout << "key: " << i << endl;
    }
    // for(uint64_t i = 1; i <= HASH_SIZE; i++)
    // {
    //     hash.remove(i);
    //     if(hash.search(i,i) == -1) cout << "remove " << i << "success" << endl;
    // }
    // for(uint64_t i = HASH_SIZE + 1; i <= 2 * HASH_SIZE; i++)
    // {
    //     if(hash.update(i, i + 1) == 0) cout << "update success" << endl;
    // }
    return 0;
}