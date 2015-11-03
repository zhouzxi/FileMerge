/**********************************************************************
* 版权所有 (C)2015, Zhou Zhaoxiong。
*
* 文件名称：FileMerge.c
* 文件标识：无
* 内容摘要：演示相同前缀文件内容的合并
* 其它说明：无
* 当前版本：V1.0
* 作    者：Zhou Zhaoxiong
* 完成日期：20150707
*
**********************************************************************/
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ftw.h>

// 重定义数据类型
typedef signed   int        INT32;
typedef unsigned int        UINT32;
typedef unsigned char       UINT8;
typedef unsigned short int  UINT16;

// 结构体定义
// 扫描到的文件名链表
typedef struct _List
{
    void         *pData;
    struct _List *pNext;
}FileNameList_T;

// 函数声明
INT32 SelectFlies(const struct dirent *pDir);
FileNameList_T *InsertList(FileNameList_T* pCurNode, void *pData, UINT16 iDataLen);
void ClearList(FileNameList_T *pFileNameList);
INT32 ScanFlieAndMerge(UINT8 *pszScanDir);
INT32 MergeFile(UINT8 *pszMergeFileName, FileNameList_T *ptFileNameList);


/****************************************************************
* 功能描述: 主函数
* 输入参数: 无
* 输出参数: 无
* 返 回 值: 0-执行成功
           -1-执行失败
* 其他说明: 无
* 修改日期       版本号        修改人        修改内容
* -------------------------------------------------------------
* 20150707        V1.0     Zhou Zhaoxiong     创建
****************************************************************/
INT32 main(void)
{
    UINT8   szScanDir[256]  = {0};
    INT32   iRetValue       = 0;

    // 获取扫描路径名
    snprintf(szScanDir, sizeof(szScanDir)-1, "%s/zhouzx/test/FileMerge/TestFile", getenv("HOME"));

    // 调用函数执行文件的扫描与合并
    iRetValue = ScanFlieAndMerge(szScanDir);
    if (iRetValue == 0)   // 扫描与合并成功
    {
        printf("Exec ScanFlieAndMerge successfully!\n");
        return 0;
    }
    else
    {
        printf("Exec ScanFlieAndMerge failed!\n");
        return -1;
    }
}


/**********************************************************************
* 功能描述：根据前缀选择文件
* 输入参数：dir-目录
* 输出参数：无
* 返 回 值：0-失败   1-成功
* 其它说明：无
* 修改日期         版本号      修改人          修改内容
* --------------------------------------------------------------------
* 20150707         V1.0    ZhouZhaoxiong        创建
***********************************************************************/
INT32 SelectFlies(const struct dirent *pDir)
{
    INT32 iPrefixLen  = 0;
    INT32 iLoopFlag   = 0;
    INT32 iSelectResult = 0;

    if (pDir == NULL)
    {
        printf("SelectFlies:input parameter is NULL!\n");
        return 0;
    }

    // 匹配文件前缀
    iPrefixLen = strlen("Test_");       // 前缀为"Test_"
    iSelectResult = (0 == strncmp(pDir->d_name, "Test_", iPrefixLen));
    if (iSelectResult == 1)            // 找到了匹配前缀的文件
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/**********************************************************************
* 功能描述：将数据插入到内存链表中
* 输入参数：pCurNode-当前节点
            pData-待插入数据
            iDataLen-数据长度
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期         版本号      修改人          修改内容
* --------------------------------------------------------------------
* 20150707         V1.0    ZhouZhaoxiong        创建
***********************************************************************/
FileNameList_T *InsertList(FileNameList_T *pCurNode, void *pData, UINT16 iDataLen)
{
    FileNameList_T *pNewNode = NULL;

    if (NULL == pCurNode || NULL == pData)
    {
        printf("InsertList:input parameter(s) is NULL!\n");
        return NULL;
    }

    if (NULL != (pNewNode = (FileNameList_T *)malloc(sizeof(*pNewNode) + iDataLen)))
    {
        pNewNode->pData = pNewNode + 1;
        memset(pNewNode->pData, 0x00, iDataLen);
        memcpy(pNewNode->pData, pData, iDataLen);

        // 加入到当前节点的后面
        pNewNode->pNext = pCurNode->pNext;
        pCurNode->pNext = pNewNode;
    }

    return pNewNode;
}

/**********************************************************************
* 功能描述：清空内存链表
* 输入参数：pFileNameList-链表数据
* 输出参数：无
* 返 回 值：无
* 其它说明：无
* 修改日期         版本号      修改人          修改内容
* --------------------------------------------------------------------
* 20150707         V1.0    ZhouZhaoxiong        创建
***********************************************************************/
void ClearList(FileNameList_T *pFileNameList)
{
    FileNameList_T *pCur  = NULL;

    if (pFileNameList == NULL)
    {
        printf("InsertList:input parameter is NULL!\n");
        return;
    }

    for (pCur = pFileNameList->pNext; NULL != pCur; pCur = pFileNameList->pNext)
    {
        pFileNameList->pNext = pCur->pNext;

        free(pCur);
        pCur = NULL;
    }
}


/**********************************************************************
* 功能描述：扫描文件并进行合并
* 输入参数：pszScanDir-扫描目录
* 输出参数：无
* 返 回 值：0-成功   其它-失败
* 其它说明：无
* 修改日期         版本号      修改人          修改内容
* --------------------------------------------------------------------
* 20150707         V1.0    ZhouZhaoxiong        创建
***********************************************************************/
INT32 ScanFlieAndMerge(UINT8 *pszScanDir)
{
    INT32  iScanDirRet           = 0;
    UINT32 iIdxNum               = 0;
    INT32  iRetVal               = 0;
    UINT8  szScanFileName[512]   = {0};
    UINT8  szResultFileName[512] = {0};

    FileNameList_T  tFileName  = {0};
    FileNameList_T *ptFileName = {0};

    struct dirent **ppDirEnt = NULL;

    if (pszScanDir == NULL)
    {
        printf("ScanFlieAndMerge:input parameter is NULL!\n");
        return -1;
    }

    // 扫描目录, 获取文件
    iScanDirRet = scandir(pszScanDir, &ppDirEnt, SelectFlies, alphasort);
    if (iScanDirRet < 0)
    {
        printf("ScanFlieAndMerge:exec scandir failed, path=%s\n", pszScanDir);
        return -2;
    }

    if (iScanDirRet == 0)
    {
        printf("ScanFlieAndMerge: no file in directory %s\n", pszScanDir);
        return -3;
    }

    ptFileName = &tFileName;

    // 将扫描到的文件插入内存链表中
    for (iIdxNum = 0; iIdxNum < iScanDirRet; iIdxNum ++)
    {
        snprintf(szScanFileName, sizeof(szScanFileName) - 1, "%s/%s", pszScanDir, ppDirEnt[iIdxNum]->d_name);
        if (NULL == (ptFileName = InsertList(ptFileName, szScanFileName, strlen(szScanFileName) + 1)))
        {
            printf("ScanFlieAndMerge: exec InsertList failed, FileName=%s\n", szScanFileName);
            break;
        }
    }

    // 释放链表空间
    for (iIdxNum = 0; iIdxNum < iScanDirRet; iIdxNum ++)
    {
        free(ppDirEnt[iIdxNum]);
        ppDirEnt[iIdxNum] = NULL;
    }

    free(ppDirEnt);
    ppDirEnt = NULL;

    // 获取带路径的结果文件名
    snprintf(szResultFileName, sizeof(szResultFileName) - 1, "%s/zhouzx/test/FileMerge/ResultFile/ResultFile.txt", getenv("HOME"));

    iRetVal = MergeFile(szResultFileName, &tFileName);
    if (iRetVal == 0)    // 合并成功
    {
        printf("ScanFlieAndMerge: exec MergeFile successfully, ResultFileName=%s\n", szResultFileName);

        // 清空内存链表
        ClearList(&tFileName);
        return 0;
    }
    else
    {
        printf("ScanFlieAndMerge: exec MergeFile failed, ResultFileName=%s\n", szResultFileName);

        // 清空内存链表
        ClearList(&tFileName);
        return -4;
    }
}


/**********************************************************************
* 功能描述： 将多个文件合并为一个
* 输入参数： pszMergeFileName-合并之后的文件名
             ptFileNameList-被合并的文件名链表
* 输出参数： 无
* 返 回 值： 0-成功   其它-失败
* 其它说明： 无
* 修改日期        版本号            修改人         修改内容
* --------------------------------------------------------------
* 20150707        V1.0          ZhouZhaoxiong        创建
***********************************************************************/
INT32 MergeFile(UINT8 *pszMergeFileName, FileNameList_T *ptFileNameList)
{
    FILE   *pFTmp   = NULL;
    FILE   *pFSrc   = NULL;
    INT32   iRetVal = 0;
    UINT8   szContentBuf[256]  = {0};
    UINT8   szTmpFileName[512] = {0};

    FileNameList_T *ptScanFileName = NULL;

    if (pszMergeFileName == NULL || ptFileNameList == NULL)
    {
        printf("MergeFile:input parameter(s) is NULL!\n");
        return -1;
    }

    // 先将内容写入临时文件中,完成之后再写入正式文件中
    // 临时文件名为"正式文件名.tmp"
    snprintf(szTmpFileName, sizeof(szTmpFileName) - 1, "%s%s", pszMergeFileName, ".tmp");
    if (NULL == (pFTmp = fopen(szTmpFileName, "wt")))
    {
        printf("MergeFile:open %s failed!\n", szTmpFileName);
        return -2;
    }

    // 将内容写入临时文件中
    for (ptScanFileName = ptFileNameList->pNext; ptScanFileName != NULL; ptScanFileName = ptScanFileName->pNext)
    {
        if (NULL == (pFSrc = fopen(ptScanFileName->pData, "r")))
        {
            printf("MergeFile:open %s failed!\n", ptScanFileName->pData);
            break;
        }

        while (NULL != fgets(szContentBuf, sizeof(szContentBuf) - 1, pFSrc))   // 将文件内容取出来
        {
            if (fputs(szContentBuf, pFTmp) <= 0)   // 将文件内容放入临时文件中
            {
                printf("MergeFile:exec fputs failed, ScanFileName=%s, TmpFileName=%s\n", ptFileNameList->pData, szTmpFileName);
                break;
            }
        }

        if (0 == feof(pFSrc))   // 文件内容未完全从扫描文件中读出, 异常退出
        {
            printf("MergeFile:do not reach the end of file %s\n", ptFileNameList->pData);
            fclose(pFSrc);
            pFSrc = NULL;
            break;
        }
        else
        {
            fclose(pFSrc);
            pFSrc = NULL;
        }
    }

    // 使用完文件之后要将其关闭并将文件指针置为空
    fclose(pFTmp);
    pFTmp = NULL;

    // 将临时文件更名为正式文件
    iRetVal = rename(szTmpFileName, pszMergeFileName);
    if (iRetVal == 0)
    {
        printf("MergeFile:rename %s to %s successfully!\n", szTmpFileName, pszMergeFileName);
        return 0;
    }
    else
    {
        printf("MergeFile:rename %s to %s failed!\n", szTmpFileName, pszMergeFileName);
        return -3;
    }    
}
