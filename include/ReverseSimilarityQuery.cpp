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
#include <chrono>
#include <thread>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
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

        // node.reserve(n.size());
        // for (BIGNUM* bn : n) {
        //     node.push_back(BN_dup(bn)); // 确保每个 BIGNUM 都是独立的
        // }
        // index = i;
    }
    // 默认构造函数
    HeapNode() {
        node = vector<BIGNUM*>();
        index = -1;
    }

    ~HeapNode() {
        // for (BIGNUM* bn : node) {
        //     BN_free(bn); // 释放 BIGNUM 对象
        // }
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
 * @return 状态码，1：成功；0：失败
 */
int dealData(char* fileString_x, char* fileString_y) {
    // 读取未经处理的数据集x和y
    int lineNumber = 1;
    vector<BIGNUM*> data = readBIGNUMsFromFile(fileString_x, lineNumber);
    if (data.empty()) {
        return 0;
    }
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
            } else if (node < heap.top()) {
                heap.pop();
                heap.push(node);
            }
        }
        vector<HeapNode> tempHeap(heap.size());

        for (int l = tempHeap.size() - 1; l >= 0; l--) {
            HeapNode t(heap.top().node, heap.top().index);
            tempHeap[l] = t;
            // tempHeap[l].node = heap.top().node;
            heap.pop();
        }

        heap_x[i] = tempHeap;
    }

    // 释放临时变量
    BN_free(square);

    return 1;
}

// 打印花费的时间
void printTime111(clock_t start_time,char * desc){
    clock_t end_time = clock();
    double execution_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC * 1000;
    printf("%s的时间是：%f 毫秒\n",desc, execution_time);
    fflush(stdout);
}


/**
 * @Method: 发起反向相似性查询
 * @param fileString 读取数据的地址
 * @param resultFilePath 输出数据的地址
 * @return 状态码，1：成功；0：失败
 */
int reverseSQ(char* fileString, char* resultFilePath) {
    // 读取两行数据，第一行含有一个数据k；第二行为数据集y中的一个数据，空格分隔
    vector<vector<BIGNUM*>> data_list(2);
    data_list[0] = readBIGNUMsFromFile(fileString, 1);
    data_list[1] = readBIGNUMsFromFile(fileString, 2);

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

    // // 减少不必要的比较操作
    // vector<int> result_indices;
    // result_indices.reserve(Data_x.size());
    //
    // for (int i = 0; i < Data_x.size(); ++i) {
    //     HeapNode t(data_list[1], i);
    //     if (t < heap_x[i][k - 1]) {
    //         result_indices.push_back(i);
    //     }
    // }

    // ------------------------------------------------------------------------
    // 获取系统的CPU核心数量
    // int num_processes = sysconf(_SC_NPROCESSORS_ONLN);
    int num_processes = 12;
    cout << "Number of CPU cores: " << num_processes << endl;

    // 创建多个临时文件用于存储子进程结果
    vector<string> temp_files;
    for (int p = 0; p < num_processes; ++p) {
        temp_files.push_back("/tmp/temp_result_" + to_string(p) + ".txt");
    }

    for (int p = 0; p < num_processes; ++p) {
        pid_t pid = fork();
        if (pid == 0) {
            // 子进程代码
            vector<int> local_results;
            for (int i = p; i < Data_x.size(); i += num_processes) {
                HeapNode t(data_list[1], i);
                if (t < heap_x[i][k - 1]) {
                    local_results.push_back(i);
                }
            }
            // 将结果写入临时文件
            ofstream temp_file(temp_files[p]);
            for (int idx : local_results) {
                temp_file << idx << endl;
            }
            temp_file.close();
            _exit(0);
        }
    }

    // 父进程代码
    for (int p = 0; p < num_processes; ++p) {
        wait(NULL); // 等待所有子进程完成
    }

    vector<int> result_indices;
    // 读取所有临时文件并汇总结果
    for (const auto& temp_file : temp_files) {
        ifstream infile(temp_file);
        int idx;
        while (infile >> idx) {
            result_indices.push_back(idx);
        }
        infile.close();
        remove(temp_file.c_str()); // 删除临时文件
    }


    // ------------------------------------------------------------------------


    // 将结果写入文件
    clock_t start_time = clock();


    ofstream outfile(resultFilePath);
    if (outfile.is_open()) {
        for (int idx : result_indices) {
            for (BIGNUM* val : Data_x[idx]) {
                char* bn_str = BN_bn2dec(val);
                outfile << bn_str << " ";
                OPENSSL_free(bn_str);
            }
            outfile << endl;
        }
        outfile.close();
    } else {
        cerr << "Unable to open file " << resultFilePath << endl;
        return 0;
    }

    printTime111(start_time, "写入文件");


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
 * @return 状态码，1：成功；0：失败
 */
int freeRawData() {
    if (rawData_x.empty() && rawData_y.empty() && Data_x.empty() && Data_y.empty() && heap_x.empty()) {
        return 0;
    }
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
    return 1;
}