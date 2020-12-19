#include "pml_hash.h"
#include <iomanip>
#include <fstream>
#include <iostream>
#include <dirent.h>
#include <vector>
#include <cstring>
#include <ctime>
#include <algorithm>

using namespace std;

//存放文件夹中的文件的名称
vector<string> filelist;
//文件夹的名称
const string dir_name = "../benchmark/";

void OpenDir()
{
<<<<<<< HEAD
    DIR *dir;
    struct dirent *mydirent;
    //判断文件夹打开是否成功
    if ((dir = opendir(dir_name.c_str())) == NULL)
    {
        cout << "fail to open " << dir_name << endl;
    }
    //取出文件夹中的文件
    while ((mydirent = readdir(dir)) != NULL)
    {
        // d_type为8，表示读入的文件类型为txt
        if ((int)mydirent->d_type == 8)
            filelist.push_back(mydirent->d_name);
    }
    //关闭文件夹
    closedir(dir);
    //排序，使得read和write比例相同的read和load文件放在一起，
    //方便后续执行
    sort(filelist.begin(), filelist.end());
    for (uint64_t i = 0; i < filelist.size(); i++)
    {
        cout << "filename: " << filelist[i] << endl;
    }
    cout << endl;
}

void Load(string filename, PMLHash &hash)
{
    //打开文件
    fstream file;
    file.open(dir_name + filename, ios::in);
    cout << "start to init" << endl;
    string cmd, data;
    time_t start = clock();
    //记录数据量
    uint64_t data_num = 0;
    //记录插入失败的数据量
    uint64_t fail_num = 0;
    //开始读取文件
    while (!file.eof())
    {
        cmd = "";
        //读入操作符，若为空表示终止
        file >> cmd;
        if (cmd.empty())
            break;
        file >> data;
        //key和value的值相同
        uint64_t key = atoi(data.c_str());
        uint64_t value = value;
        //由于为load，只有插入操作
        //进行插入操作
        if (hash.insert(key, value) == -1)
            fail_num++;
        data_num++;
    }
    time_t end = clock();
    file.close();
    double cost = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    cout << "Finished, use time: " << fixed << setprecision(5) << cost << "ms" << endl;
    cout << "total number: " << data_num << endl;
    cout << "fail number: " << fail_num << endl;
    cout << endl;
}

void Run(string filename, PMLHash &hash)
{
    //打开文件
    fstream file;
    file.open(dir_name + filename, ios::in);
    cout << "start to run " << endl;
    string cmd, data;
    time_t start = clock();
    //记录各个操作数据量
    uint64_t insert_num = 0, search_num = 0, update_num = 0, remove_num = 0, total_num = 0;
    //记录操作失败的数据量
    uint64_t fail_num = 0;
    //开始读取文件
    while (!file.eof())
    {
        cmd = "";
        //读入操作符，若为空表示终止
        file >> cmd;
        if (cmd.empty())
            break;
        file >> data;
        //key和value的值相同
        uint64_t key = atoi(data.c_str());
        uint64_t value = value;
        if (cmd == "INSERT")
        {
            if (hash.insert(key, value) == -1)
                fail_num++;
            insert_num++;
        }
        else if (cmd == "READ")
        {
            if (hash.search(key, value) == -1)
                fail_num++;
            search_num++;
        }
        else if (cmd == "UPDATE")
        {
            if (hash.update(key, value) == -1)
                fail_num++;
            update_num++;
        }
        else if (cmd == "REMOVE")
        {
            if (hash.remove(key) == -1)
                fail_num++;
            remove_num++;
        }
        else
        {
            cout << "invalid operation " << endl;
        }
        total_num++;
    }
    time_t end = clock();
    file.close();
    double cost = ((double)(end - start)) / CLOCKS_PER_SEC;
    cout << "Finished, use time: " << fixed << setprecision(5) << cost * 1000 << "ms" << endl;
    cout << "total number: " << total_num << endl;
    cout << "insert number: " << insert_num << endl;
    cout << "read number: " << search_num << endl;
    cout << "update number: " << update_num << endl;
    cout << "remove number: " << remove_num << endl;
    cout << "fail number: " << fail_num << endl;
    cout << "OPS: " << (uint64_t)(total_num / cost) << endl;
    cout << endl;
}

int main()
{
    //打开文件夹
    OpenDir();
    for (uint64_t i = 0; i < filelist.size() / 2; i++)
    {
        cout << "Task " << i + 1 << endl;
        cout << "operation with " << filelist[2 * i] << " and " << filelist[2 * i + 1] << endl;
        //依次对每对load和run文件进行操作
        PMLHash hash("/mnt/pmemdir/file");
        Load(filelist[2 * i], hash);
        Run(filelist[2 * i + 1], hash);
        //清空上个文件对的内容
        system("rm /mnt/pmemdir/file");
=======
    clock_t start_time = clock();
    PMLHash hash("/mnt/pmemdir/file");
    int insert_failed = 0;
    int search_failed = 0;
    int update_failed = 0;
    int remove_failed = 0;
    cout << "start insert" << endl;
    for (uint64_t i = 1; i <= 100000; i++)
    {
        if (hash.insert(i, i) == -1)
            ++insert_failed;
    }
    cout << "insert failed: " << insert_failed << endl;
    cout << "start search" << endl;
    for (uint64_t i = 1; i <= 100000; i++)
    {
        uint64_t val;
        if (hash.search(i, val) == -1)
            ++search_failed;
    }
    cout << "search failed: " << search_failed << endl;
    cout << "start update" << endl;
    for (uint64_t i = 1; i <= 100000; i++)
    {
        if (hash.update(i, i + 1) == -1)
            ++update_failed;
    }
    cout << "update failed: " << update_failed << endl;
    cout << "start remove" << endl;
    for (uint64_t i = 1; i <= 100000; i++)
    {
        if (hash.remove(i) == -1)
            ++remove_failed;
>>>>>>> fix: test main.cc
    }
    cout << "remove failed: " << remove_failed << endl;
    cout << "finished" << endl;
    clock_t end_time = clock();
    cout << "serial time: " << (double)(end_time - start_time) / CLOCKS_PER_SEC << endl;
    return 0;
}