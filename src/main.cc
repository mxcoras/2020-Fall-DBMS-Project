#include "pml_hash.h"

#define DEBUG

int main()
{
#ifndef DEBUG
    clock_t start_time = clock();
    PMLHash hash("/mnt/pmemdir/file");
    int insert_failed = 0;
    int search_failed = 0;
    int update_failed = 0;
    int remove_failed = 0;
    cout << "start insert" <<endl;
    for (uint64_t i = 1; i <= 10000; i++)
    {
        if (hash.insert(i, i) == -1)
            ++insert_failed;
    }
    cout << "insert failed: " << insert_failed <<endl;
    cout << "start search" <<endl;
    for (uint64_t i = 1; i <= 10000; i++)
    {
        uint64_t val;
        if (hash.search(i, val) == -1)
            ++search_failed;
    }
    cout << "search failed: " << search_failed <<endl;
    cout << "start update" <<endl;
    for (uint64_t i = 1; i <= 10000; i++)
    {
        if (hash.update(i, i + 1) == -1)
            ++update_failed;
    }
    cout << "update failed: " << update_failed <<endl;
    cout << "start remove" <<endl;
    for (uint64_t i = 1; i <= 10000; i++)
    {
        if (hash.remove(i) == -1)
            ++remove_failed;
    }
    cout << "remove failed: " << remove_failed <<endl;
    cout << "finished" <<endl;
    clock_t end_time = clock();
    cout << "serial time:" << (double)(end_time - start_time)/CLOCKS_PER_SEC <<endl;
    return 0;
#endif

#ifdef DEBUG
    double start_time = omp_get_wtime();
    PMLHash hash("/mnt/pmemdir/file");
    int insert_failed = 0;
    int search_failed = 0;
    int update_failed = 0;
    int remove_failed = 0;
    cout << "start insert" <<endl;
    #pragma omp parallel for num_threads(4)
    for (uint64_t i = 1; i <= 10000; i++)
    {
        if (hash.insert(i, i) == -1)
            #pragma omp atomic
            ++insert_failed;
    }
    cout << "insert failed: " << insert_failed <<endl;
    cout << "start search" <<endl;
    #pragma omp parallel for num_threads(4)
    for (uint64_t i = 1; i <= 10000; i++)
    {
        uint64_t val;
        if (hash.search(i, val) == -1)
            #pragma omp atomic
            ++search_failed;
    }
    cout << "search failed: " << search_failed <<endl;
    cout << "start update" <<endl;
    #pragma omp parallel for num_threads(4)
    for (uint64_t i = 1; i <= 10000; i++)
    {
        if (hash.update(i, i + 1) == -1)
            #pragma omp atomic
            ++update_failed;
    }
    cout << "update failed: " << update_failed <<endl;
    cout << "start remove" <<endl;
    #pragma omp parallel for num_threads(4)
    for (uint64_t i = 1; i <= 10000; i++)
    {
        if (hash.remove(i) == -1)
            #pragma omp atomic
            ++remove_failed;
    }
    cout << "remove failed: " << remove_failed <<endl;
    cout << "finished" <<endl;
    double end_time = omp_get_wtime();
    cout << "parallel time:" << end_time - start_time <<endl;
    return 0;
#endif

}