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
vector <string> Filelist;
//文件夹的名称
const string dir_name = "../benchmark/";

void OpenDir(){
    DIR *dir;
    struct dirent *mydirent;
    //判断文件夹打开是否成功
    if ((dir=opendir(dir_name.c_str()))==NULL){
        cout << "fail to open " << dir_name << endl; 
    }
    //取出文件夹中的文件
    while ((mydirent = readdir(dir)) != NULL){
        // d_type为8，表示读入的文件类型为txt
        if ((int)mydirent->d_type==8)
            Filelist.push_back(mydirent->d_name);
    }
    //关闭文件夹
    closedir(dir);
    //排序，使得read和write比例相同的read和load文件放在一起，
    //方便后续执行
    sort(Filelist.begin(),Filelist.end());
    for (uint64_t i=0; i<Filelist.size(); i++){
        cout << "filename: " << Filelist[i] << endl;
    }
    cout << endl;
}
void Load(string filename, PMLHash& hash){
    //打开文件
    fstream file;
    file.open(dir_name + filename, ios::in);
    cout << "start to init" << endl;
    string cmd,data;
    time_t start = clock();
    //记录数据量
    long long dataNum = 0;
    //记录插入失败的数据量
    long long failNum = 0;
    //开始读取文件
    while (!file.eof()){
        cmd="";
        //读入操作符，若为空表示终止
        file >> cmd;
        if (cmd.empty()) break;
        file >> data;
        //key和value的值相同
        uint64_t key = atoi(data.c_str());
        uint64_t value =value ;
        //由于为load，只有插入操作
        //进行插入操作
        if (hash.insert(key,value)==-1) failNum++;
        dataNum++;
    }
    time_t end = clock();
    file.close();
    double cost = ((double)(end-start))/CLOCKS_PER_SEC;
    cout << "Finished, use time: " <<  fixed << setprecision(5) << cost << "s" << endl;
    cout << "total number: " << dataNum << endl;
    cout << "fail number: " << failNum << endl;
    cout << endl;
}
void Run(string filename, PMLHash& hash){
    //打开文件
    fstream file;
    file.open(dir_name + filename, ios::in);
    cout << "start to run " << endl;
    string cmd,data;
    time_t start = clock();
    //记录各个操作数据量
    long long insertNum = 0, searchNum = 0, updateNum = 0, removeNum = 0, totalNum = 0;
    //记录操作失败的数据量
    long long failNum = 0;
    //开始读取文件
    while (!file.eof()){
        cmd="";
        //读入操作符，若为空表示终止
        file >> cmd;
        if (cmd.empty()) break;
        file >> data;
        //key和value的值相同
        uint64_t key = atoi(data.c_str());
        uint64_t value = value;
        if (cmd == "INSERT"){
            if (hash.insert(key, value)== -1) failNum++;
            insertNum++;
        }
        else if (cmd == "READ"){
            if (hash.search(key, value)== -1) failNum++;
            searchNum++;
        }
        else if (cmd == "UPDATE"){
            if (hash.update(key, value)== -1) failNum++;
            updateNum++;
        }
        else if (cmd == "REMOVE"){
            if (hash.remove(key)== -1) failNum++;
            removeNum++;
        }
        else {
            cout << "invalid operation " << endl;
        }
        totalNum++;
    }
    time_t end = clock();
    file.close();
    double cost = ((double)(end-start))/CLOCKS_PER_SEC;
    cout << "Finished, use time: " <<  fixed << setprecision(5) << cost << "s" << endl;
    cout << "total number: " << totalNum << endl;
    cout << "insert/total: " <<  (double)insertNum/(double)totalNum << endl;
    cout << "read/total: " <<  (double)searchNum/(double)totalNum << endl;
    cout << "update/total: " <<  (double)updateNum/(double)totalNum << endl;
    cout << "remove/total: " <<  (double)removeNum/(double)totalNum << endl;
    cout << "fail/total: " << (double)failNum/(double)totalNum << endl;
    cout << "OPS: " << (long long)(totalNum/cost) << endl;
    cout << endl;
}
int main()
{
    //打开文件夹
    OpenDir();
    for (uint64_t i=0; i<Filelist.size()/2; i++){
        cout << "Test " << i+1 << endl;
        cout << "operation with " << Filelist[2*i] << " and " << Filelist[2*i+1] << endl;
        //依次对每对load和run文件进行操作
        PMLHash hash("/mnt/pmemdir/file");
        Load(Filelist[2*i], hash);
        Run(Filelist[2*i+1], hash);
        //清空上个文件对的内容
        system("rm /mnt/pmemdir/file");
    }
    return 0;
}