#include "pml_hash.h"

int main()
{
    clock_t start_time = clock();
    PMLHash hash("/mnt/pmemdir/file");
    int insert_failed = 0;
    int search_failed = 0;
    int update_failed = 0;
    int remove_failed = 0;
    cout << "start insert" << endl;
    for (uint64_t i = 1; i <= 200000; i++)
    {
        if (hash.insert(i, i) == -1)
            ++insert_failed;
    }
    //cout << "finished" << endl;
    cout << "insert failed: " << insert_failed << endl;
    cout << "start search" << endl;
    for (uint64_t i = 1; i <= 200000; i++)
    {
        uint64_t val;
        if (hash.search(i, val) == -1)
            ++search_failed;
    }
    cout << "search failed: " << search_failed << endl;
    cout << "start update" << endl;
    for (uint64_t i = 1; i <= 200000; i++)
    {
        if (hash.update(i, i + 1) == -1)
            ++update_failed;
    }
    cout << "update failed: " << update_failed << endl;
    cout << "start remove" << endl;
    for (uint64_t i = 1; i <= 200000; i++)
    {
        if (hash.remove(i) == -1)
            ++remove_failed;
    }
    cout << "remove failed: " << remove_failed << endl;
    cout << "finished" << endl;
    clock_t end_time = clock();
    cout << "serial time: " << (double)(end_time - start_time) / CLOCKS_PER_SEC << endl;
    return 0;
}