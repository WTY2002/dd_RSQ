/**
* @author: WTY
* @date: 2024/7/9
* @description: Definition of constants, operations, and header files
*/

#ifndef REVERSESIMILARITYQUERY_H
#define REVERSESIMILARITYQUERY_H

#include<cstring>


#ifdef _WIN32
#define EXPORT_SYMBOL __declspec(dllexport)
#else
#define EXPORT_SYMBOL __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	/**
	 * @Method: 数据预计算
	 * @param char* fileString_x 读取数据集x的地址
	 * @param char* fileString_y 读取数据集y的地址
	 */
	EXPORT_SYMBOL void dealData(char* fileString_x, char* fileString_y);

	/**
	 * @Method: 发起反向相似性查询
	 * @param fileString 读取数据的地址
	 * @param resultFilePath 输出数据的地址
	 * @return 状态码，1：成功；0：失败
	 */
	EXPORT_SYMBOL int reverseSQ(char* fileString, char* resultFilePath);

	/**
	 * @Method: 释放所用分配的内存
	 */
	EXPORT_SYMBOL void freeRawData();


#ifdef __cplusplus
}
#endif

using namespace std;



#endif //REVERSESIMILARITYQUERY_H
