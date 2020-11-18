#include "pml_hash.h"

// int main() {
//     PMLHash hash("/home/pmem/file");
//     for (uint64_t i = 1; i <= HASH_SIZE * TABLE_SIZE; i++) {
//         hash.insert(i, i);
//     }
//     for (uint64_t i = 1; i <= HASH_SIZE; i++) {
//         uint64_t val;
//         hash.search(i, val);
//         cout << "key: " << i << "\nvalue: " << val << endl;
//     }

//     for (uint64_t i = HASH_SIZE * TABLE_SIZE + 1; 
//          i <= (HASH_SIZE + 1) * TABLE_SIZE; i++) {
//         hash.insert(i, i);
//     }
//     for (uint64_t i = HASH_SIZE * TABLE_SIZE + 1;
//          i <= (HASH_SIZE + 1) * TABLE_SIZE; i++) {
//         uint64_t val;
//         hash.search(i, val);
//         cout << "key: " << i << "\nvalue: " << val << endl;
//     }
//     return 0;
// }