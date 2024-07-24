/**
* @author: WTY
* @date: 2024/7/9
* @description: Definition of constants, operations, and header files
*/

#ifndef REVERSESIMILARITYQUERY_H
#define REVERSESIMILARITYQUERY_H

#include<cstring>
using namespace std;



/**
 * @Method: 数据预计算
 * @param char* fileString_x 读取数据集x的地址
 * @param char* fileString_y 读取数据集y的地址
 * @return 状态码，1：成功；0：失败
 */
int dealData(char* fileString_x, char* fileString_y);



/**
 * @Method: 发起反向相似性查询
 * @param fileString 读取数据的地址
 * @param resultFilePath 输出数据的地址
 * @return 状态码，1：成功；0：失败
 */
int reverseSQ(char* fileString, char* resultFilePath);

/**
 * @Method: 释放所用分配的内存
 * @return 状态码，1：成功；0：失败
 */
int freeRawData();


#endif //REVERSESIMILARITYQUERY_H
