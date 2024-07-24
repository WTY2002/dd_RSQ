#include <iostream>
#include <SHE.h>
#include <PHE.h>
#include <ReverseSimilarityQuery.h>
#include <cmath>
#include <queue>
#include <openssl/bn.h>
#include <chrono>
using namespace std::chrono;

// 打印花费的时间
void printTime(clock_t start_time,char * desc){
    clock_t end_time = clock();
    double execution_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC * 1000;
    printf("%s的时间是：%f 毫秒\n",desc, execution_time);
    fflush(stdout);
}

struct test {
    vector<int> a;
    bool operator<(const test &b) const {
        int sum1 = 0, sum2 = 0;

        for (int i = 0; i < a.size(); i++) {
            sum1 += a[i];
            sum2 += b.a[i];
        }
        if (sum1 < sum2) {
            return true;
        } else {
            return false;
        }
    }
    test(vector<int> a) : a(a) {}
    test() {
        a = vector<int>();
    }
};

int main()
{

    char* algoName = "inner_product";
    char* fileString = "/root/wty/data1.txt";
    char* fileString2 = "/root/wty/data2.txt";
    char* query = "/root/wty/query.txt";
    char* resultFilePath = "/root/wty/result.txt";


    clock_t start_time1 = clock();

    dealData(fileString, fileString2);

    printTime(start_time1, "预处理数据");

    auto start_time = high_resolution_clock::now(); // 记录函数开始时间

    reverseSQ(query, resultFilePath);

    auto end_time = high_resolution_clock::now(); // 记录函数结束时间
    duration<double, milli> total_duration = end_time - start_time;
    printf("查询的总时间是：%f 毫秒\n", total_duration.count());
    fflush(stdout);


    freeRawData();





    // freeRawData();
    // freeRawData();
    // freeRawData();
    //
    // // deal(algoName, fileString, resultFilePath);
    // dealData(fileString, fileString2);
    //
    // reverseSQ(query, resultFilePath);
    // freeRawData();
    //
    // cout << "-----------------------------------" << endl;
    //
    // dealData(fileString, fileString2);
    //
    // reverseSQ(query, resultFilePath);
    // freeRawData();
    //
    // cout << "-----------------------------------" << endl;
    //
    // dealData(fileString, fileString2);
    //
    // reverseSQ(query, resultFilePath);
    // freeRawData();
    //
    // cout << "-----------------------------------" << endl;
    //
    // freeRawData();

    return 0;
}
