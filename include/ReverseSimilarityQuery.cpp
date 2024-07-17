/**
* @author: WTY
* @date: 2024/7/9
* @description: Definition of constants, operations, and header files
*/

#include "ReverseSimilarityQuery.h"
#include "SHE.h"
#include "PHE.h"
#include<iostream>
#include <sstream>
#include <cstring>
#include <fstream>
#include <queue>
#include <openssl/bn.h>

#ifdef _WIN32
#define EXPORT_SYMBOL __declspec(dllexport)
#else
#define EXPORT_SYMBOL __attribute__((visibility("default")))
#endif
using namespace std;

BN_CTX* CTXX = BN_CTX_new();

// 未经处理的原始数据
vector<vector<BIGNUM*>> rawData_x;
vector<vector<BIGNUM*>> rawData_y;

// 数据集x和y的存储形式
vector<vector<BIGNUM*>> Data_x;
vector<vector<BIGNUM*>> Data_y;

// 定义最大堆的大小
int k_max;

// 定义堆中数据元素的结构体
struct HeapNode {
    // y中的数据点
    vector<BIGNUM*> node;
    // 数据点x中对应的索引
    int index;

    bool operator<(const HeapNode& other) const {
        // return BN_cmp(distance, other.distance) < 0;
        BIGNUM* temp = BN_CTX_get(CTXX);
        BN_zero(temp);
        BIGNUM* temp2 = BN_CTX_get(CTXX);
        BIGNUM* temp3 = BN_CTX_get(CTXX);
        for (int i = 0; i < node.size() - 1; i++) {
            BN_set_word(temp2, 2);
            BN_mul(temp2, temp2, Data_x[index][i], CTXX);
            BN_sub(temp3, node[i], other.node[i]);
            BN_mul(temp2, temp2, temp3, CTXX);
            BN_add(temp, temp, temp2);
        }
        BN_sub(temp, temp, node[node.size() - 1]);
        BN_add(temp, temp, other.node[other.node.size() - 1]);

        // 生成两个k_M比特的随机数r1, r2
        BIGNUM* r1 = generateRandom(20);
        BIGNUM* r2 = generateRandom(20);

        // 要保证r1 > r2 > 0
        while (BN_cmp(r1, r2) <= 0) {
            BN_free(r1);
            BN_free(r2);
            r1 = generateRandom(20);
            r2 = generateRandom(20);
        }

        // temp = r1 * temp - r2
        BN_mul(temp, temp, r1, CTXX);
        BN_sub(temp, temp, r2);

        BN_zero(temp2);
        BN_free(temp3);
        BN_free(r1);
        BN_free(r2);
        if (BN_cmp(temp, temp2) == -1) {
            // 释放临时变量
            BN_free(temp);
            BN_free(temp2);
            return false;
        }
        // 释放临时变量
        BN_free(temp);
        BN_free(temp2);
        return true;
    }
    HeapNode(vector<BIGNUM*> n, int i) {
        node = n;
        index = i;
    }
    // 默认构造函数
    HeapNode() {
        node = vector<BIGNUM*>();
        index = -1;
    }

};

// 存储数据集x中每个数据的最大堆
vector<vector<HeapNode>> heap_x;

// 将 BIGNUM 转换为 int
int BN_to_int(BIGNUM* bn) {
    if (BN_is_negative(bn)) {
        BIGNUM* neg_bn = BN_dup(bn);
        BN_set_negative(neg_bn, 0);
        unsigned long word = BN_get_word(neg_bn);
        BN_free(neg_bn);
        return -static_cast<int>(word);
    } else {
        return static_cast<int>(BN_get_word(bn));
    }
}


/**
 * @Method: 数据预计算
 * @param char* fileString_x 读取数据集x的地址
 * @param char* fileString_y 读取数据集y的地址
 */
EXPORT_SYMBOL void dealData(char* fileString_x, char* fileString_y) {
    // 读取未经处理的数据集x和y
    int lineNumber = 1;
    vector<BIGNUM*> data = readBIGNUMsFromFile(fileString_x, lineNumber);
    while (!data.empty()) {
        lineNumber++;
        rawData_x.push_back(data);
        data = readBIGNUMsFromFile(fileString_x, lineNumber);
    }

    lineNumber = 1;
    data = readBIGNUMsFromFile(fileString_y, lineNumber);
    while (!data.empty()) {
        lineNumber++;
        rawData_y.push_back(data);
        data = readBIGNUMsFromFile(fileString_y, lineNumber);
    }

    // 判空
    if (rawData_x.empty() || rawData_y.empty()) {
        cerr << "Error: Failed to read data" << endl;
    }

    // 定义临时变量
    BIGNUM* square = BN_CTX_get(CTXX);

    // 将数据集x和y转化为存储形式
    for (int i = 0; i < rawData_x[0].size(); i++) {
        vector<BIGNUM*> temp;
        BIGNUM* Sum_of_squares = BN_CTX_get(CTXX);
        BN_zero(Sum_of_squares);
        for (int j = 0; j < rawData_x.size(); j++) {
            temp.push_back(rawData_x[j][i]);
            // 计算rawData_x[j][i]，并累加到Sum_of_squares
            BN_mul(square, rawData_x[j][i], rawData_x[j][i], CTXX);
            BN_add(Sum_of_squares, Sum_of_squares, square);
        }
        temp.push_back(Sum_of_squares);
        Data_x.push_back(temp);
    }

    for (int i = 0; i < rawData_y[0].size(); i++) {
        vector<BIGNUM*> temp;
        BIGNUM* Sum_of_squares = BN_CTX_get(CTXX);
        BN_zero(Sum_of_squares);
        for (int j = 0; j < rawData_y.size(); j++) {
            temp.push_back(rawData_y[j][i]);
            // 计算rawData_x[j][i]，并累加到Sum_of_squares
            BN_mul(square, rawData_y[j][i], rawData_y[j][i], CTXX);
            BN_add(Sum_of_squares, Sum_of_squares, square);
        }
        temp.push_back(Sum_of_squares);
        Data_y.push_back(temp);
    }

    // 设置最大堆的大小
    k_max = 6;

    // 设置heap_x的容量
    heap_x.resize(Data_x.size());

    for (int i = 0; i < Data_x.size(); i++) {
        // 定义最大堆
        priority_queue<HeapNode> heap;

        for (int j = 0; j < Data_y.size(); j++) {

            HeapNode node(Data_y[j], i);

            if (heap.size() < k_max) {
                heap.push(node);
            }
            else if (node < heap.top()) {
                heap.pop();
                heap.push(node);
            }
        }
        vector<HeapNode> tempHeap(k_max);

        for (int l = k_max - 1; l >= 0; l--) {
            HeapNode t(heap.top().node, heap.top().index);
            tempHeap[l] = t;
            // tempHeap[l].node = heap.top().node;
            heap.pop();
        }

        heap_x[i] = tempHeap;
    }
    // 释放临时变量
    BN_free(square);
}

/**
 * @Method: 发起反向相似性查询
 * @param fileString 读取数据的地址
 * @param resultFilePath 输出数据的地址
 * @return 状态码，1：成功；0：失败
 */
EXPORT_SYMBOL int reverseSQ(char* fileString, char* resultFilePath) {
    // 读取两行数据，第一行含有一个数据k；第二行为数据集y中的一个数据，空格分隔
    vector<vector<BIGNUM*>> data_list(2);
    data_list[0] = readBIGNUMsFromFile(fileString, 1);
    data_list[1] = readBIGNUMsFromFile(fileString, 2);

    vector<vector<BIGNUM*>> res;

    // 将data_list[0][0]转为int类型
    int k = BN_to_int(data_list[0][0]);

    BIGNUM* distance = BN_CTX_get(CTXX);
    BIGNUM* temp = BN_CTX_get(CTXX);
    BN_zero(temp);
    for (int i = 0; i < data_list[1].size(); i++) {
        BN_mul(distance, data_list[1][i], data_list[1][i], CTXX);
        BN_add(temp, temp, distance);
    }

    data_list[1].push_back(BN_dup(temp)); //添加一个辅助元素，方便后续比较

    for (int i = 0; i < Data_x.size(); i++) {
        HeapNode t(data_list[1], i);

        if (t < (heap_x[i][k - 1])) {
            res.push_back(Data_x[i]);
        }
    }

    // 输出res
    for (int i = 0; i < res.size(); i++) {
        for (int j = 0; j < res[i].size(); j++) {
            cout << BN_bn2dec(res[i][j]) << " ";
        }
        cout << endl;
    }

    // 将res写入文件
    ofstream outfile(resultFilePath);
    if (outfile.is_open()) {
        for (int i = 0; i < res.size(); i++) {
            for (int j = 0; j < res[i].size(); j++) {
                char* bn_str = BN_bn2dec(res[i][j]);
                outfile << bn_str << " ";
                OPENSSL_free(bn_str);
            }
            outfile << endl;
        }
    }
    else {
        cerr << "Unable to open file " << resultFilePath << endl;
        return 0;
    }

    // 释放data_list
     for (int i = 0; i < data_list.size(); i++) {
         for (int j = 0; j < data_list[i].size(); j++) {
             BN_free(data_list[i][j]);
         }
     }

    return 1;
}

/**
 * @Method: 释放所用分配的内存
 */
EXPORT_SYMBOL void freeRawData() {
    // 释放rawData_x
    for (int i = 0; i < rawData_x.size(); i++) {
        for (int j = 0; j < rawData_x[i].size(); j++) {
            BN_free(rawData_x[i][j]);
        }
    }
    rawData_x.clear();

    // 释放rawData_y
    for (int i = 0; i < rawData_y.size(); i++) {
        for (int j = 0; j < rawData_y[i].size(); j++) {
            BN_free(rawData_y[i][j]);
        }
    }
    rawData_y.clear();

    Data_x.clear();
    Data_y.clear();
    heap_x.clear();
}