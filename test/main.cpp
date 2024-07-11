#include <iostream>
#include <SHE.h>
#include <PHE.h>
#include <ReverseSimilarityQuery.h>
#include <cmath>
#include <queue>
#include <openssl/bn.h>

#include "ReverseSimilarityQuery.h"


using namespace std;

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
    char* fileString = "/root/wty/data.txt";
    char* fileString2 = "/root/wty/data2.txt";
    char* query = "/root/wty/query.txt";
    char* resultFilePath = "/root/wty/result.txt";

    // deal(algoName, fileString, resultFilePath);
    dealData(fileString, fileString2);
    reverseSQ(query, resultFilePath);


    return 0;
}
