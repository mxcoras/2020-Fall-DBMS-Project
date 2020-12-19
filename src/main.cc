#include "pml_hash.h"
#include <fstream>
#include <iostream>
#include <dirent.h>
#include <regex>
#include <vector>
#include <string.h>
#include <cstdio>
#include <time.h>
#include <iomanip>
using namespace std;

vector <string> Filelist;
int testNum;
string dir_name = "../benchmark/";

uint64_t change_to_uint64(string s){
    uint64_t ret=0;
    for (int i=0; i<8; i++){
        ret <<= 8;
        ret +=s[i];
    }
    return ret;
}
void getFileList(){
    DIR *dir;
    struct dirent *dirp;
    //判断文件夹打开是否成功
    if ((dir=opendir(dir_name.c_str()))==NULL){
        cout << "fail to open " << dir_name << endl; 
    }
    //取出文件夹中的文件
    while ((dirp = readdir(dir)) != NULL){
        Filelist.push_back(dirp->d_name);
    }
    //关闭文件夹
    closedir(dir);
    //排序，使得read和write比例相同的read和load文件放在一起，
    //方便后续执行
    sort(Filelist.begin(),Filelist.end());
    //load和run各占一半
    testNum=Filelist.size()/2;
    for (int i=0; i<testNum; i++){
        cout << "[FIND FILE] " << Filelist[i*2] << " and " << Filelist[i*2+1] << endl;
    }
}
void loadFile(string filename, PMLHash& hash){
    //打开文件
    fstream file;
    file.open(dir_name + filename, ios::in);
    
    cout << "start to init with " << filename << endl;
    string cmd,data;
    time_t start = clock();
    //记录数据量
    long long dataNum = 0;
    //记录插入失败的数据量
    long long failNum = 0;
    //开始读取文件
    while (!file.eof()){
        //清空数据
        cmd = data = "";
        //读入操作符，若为空表示终止
        file >> cmd;
        if (cmd == "") break;
        file >> data;
        //由于取8个字节为键值，所以取数字字符串的前8个字符，每一个字符占一个字节，换算成uint64_t
        uint64_t key = change_to_uint64(data.substr(0,8));
        uint64_t value = atoi(data.substr(8).c_str());
        //由于为load，只有插入操作
        //进行插入操作
        if (hash.insert(key,value)==-1) failNum++;
        dataNum++;
    }
    time_t end = clock();
    file.close();
    cout << "init finished, use time: " <<  fixed << setprecision(5) << ((double)(end-start))/CLOCKS_PER_SEC << endl;
    cout << "total number: " << dataNum << endl;
    cout << "fail number: " << failNum << endl;
}
void runFile(string filename, PMLHash& hash){
    //打开文件
    fstream file;
    file.open(dir_name + filename, ios::in);
    
    cout << "start to init with " << filename << endl;
    string cmd,data;
    time_t start = clock();
    //记录各个操作数据量
    long long insertNum = 0, searchNum = 0, updateNum = 0, removeNum = 0, totalNum = 0;
    //记录操作失败的数据量
    long long failNum = 0;
    //开始读取文件
    while (!file.eof()){
        //清空数据
        cmd = data = "";
        //读入操作符，若为空表示终止
        file >> cmd;
        if (cmd == "") break;
        file >> data;
        //由于取8个字节为键值，所以取数字字符串的前8个字符，每一个字符占一个字节，换算成uint64_t
        uint64_t key = change_to_uint64(data.substr(0,8));
        uint64_t value = atoi(data.substr(8).c_str());
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
            cout << "[error] invalid operation " << endl;
        }
        totalNum++;
    }
    time_t end = clock();
    file.close();
    double spendtime = ((double)(end-start))/CLOCKS_PER_SEC;
    cout << "run finished, use time: " <<  fixed << setprecision(5) << spendtime << endl;
    cout << "total number: " << totalNum << endl;
    cout << "insert/total: " <<  (double)insertNum/(double)totalNum << endl;
    cout << "read/total: " <<  (double)searchNum/(double)totalNum << endl;
    cout << "update/total: " <<  (double)updateNum/(double)totalNum << endl;
    cout << "remove/total: " <<  (double)removeNum/(double)totalNum << endl;
    cout << "fail/total: " << (double)failNum/(double)totalNum << endl;
    cout << "OPS: " << (long long)(totalNum/spendtime) << endl;
}
int main()
{
    PMLHash hash("/mnt/pmemdir/file");
    
    return 0;
}