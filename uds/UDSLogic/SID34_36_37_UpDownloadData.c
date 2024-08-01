#include "SID34_36_37_UpDownloadData.h"
#include "service_cfg.h"
#include "uds_service.h"
#include "SID10_SessionControl.h"

#define DOWNLOAD    1
#define UPLOAD      2
#define maxNumberOfBlockLength 512 // 使用0x36服务时, 一次最多发送(ECU接收)字节数
uint8_t UpDownLoadReq = 0;
uint8_t encryptingMethod = 0;
uint8_t compressionMethod = 0;
uint32_t memoryAddress = 0;
uint32_t memorySize = 0;
uint8_t blockSequenceCounter = 0;

uint8_t file_type = 0; // 文件下载类型, flash driver / app
// uint32_t crc_server = 0; // crc校验, 边接收边进行crc校验
uint32_t crc_check_address; // 待crc校验数据的存放地址 给31服务使用
uint32_t crc_check_size; // 待crc校验数据的大小byte

/******************************************************************************
* 函数名称: bool_t service_34_check_len(const uint8_t* msg_buf, uint16_t msg_dlc)
* 功能说明: 检查 34 服务数据长度是否合法
* 输入参数: uint16_t msg_dlc         --数据长度
* 输出参数: 无
* 函数返回: TRUE: 合法; FALSE: 非法
* 其它说明: 无
******************************************************************************/
bool_t service_34_check_len(const uint8_t* msg_buf, uint16_t msg_dlc)
{
	bool_t ret = FALSE;
	
	(void)msg_buf;
	if(msg_dlc >= 5)
		ret = TRUE;

	return ret;
}

/******************************************************************************
* 函数名称: bool_t service_36_check_len(const uint8_t* msg_buf, uint16_t msg_dlc)
* 功能说明: 检查 36 服务数据长度是否合法
* 输入参数: uint16_t msg_dlc         --数据长度
* 输出参数: 无
* 函数返回: TRUE: 合法; FALSE: 非法
* 其它说明: 无
******************************************************************************/
bool_t service_36_check_len(const uint8_t* msg_buf, uint16_t msg_dlc)
{
	bool_t ret = FALSE;
	
	(void)msg_buf;
    if(UpDownLoadReq == DOWNLOAD)
    {
        if(msg_dlc >= 3)
		ret = TRUE;
    }
    else
    {
	if(msg_dlc >= 2)
		ret = TRUE;
    }
	return ret;
}

/******************************************************************************
* 函数名称: bool_t service_37_check_len(const uint8_t* msg_buf, uint16_t msg_dlc)
* 功能说明: 检查 37 服务数据长度是否合法
* 输入参数: uint16_t msg_dlc         --数据长度
* 输出参数: 无
* 函数返回: TRUE: 合法; FALSE: 非法
* 其它说明: 无
******************************************************************************/
bool_t service_37_check_len(const uint8_t* msg_buf, uint16_t msg_dlc)
{
	bool_t ret = FALSE;
	
	(void)msg_buf;
	if(msg_dlc >= 1)
		ret = TRUE;

	return ret;
}


/******************************************************************************
* 函数名称: void service_34_RequestDownload(const uint8_t* msg_buf, uint16_t msg_dlc)
* 功能说明: 34 服务 - 请求下载
* 输入参数: uint8_t*    msg_buf         --数据首地址
    　　　　uint8_t     msg_dlc         --数据长度
* 输出参数: 无
* 函数返回: 无
* 其它说明: 无
******************************************************************************/
void service_34_RequestDownload(const uint8_t* msg_buf, uint16_t msg_dlc)
{
    uint8_t rsp_buf[6];
	uint8_t mslen,malen;

    rsp_buf[0] = USD_GET_POSITIVE_RSP(SID_34);
	rsp_buf[1] = 0x40;
	rsp_buf[2] = maxNumberOfBlockLength >> 24;
    rsp_buf[3] = maxNumberOfBlockLength >> 16;
    rsp_buf[4] = maxNumberOfBlockLength >> 8;
    rsp_buf[5] = maxNumberOfBlockLength;

    compressionMethod = msg_buf[1] >> 4; // 0: 无压缩
    encryptingMethod  = msg_buf[1] & 0x0f; // 0: 无加密
    mslen = msg_buf[2] >> 4; // memory size bytes
    malen = msg_buf[2] & 0x0f; // memory address bytes

    UpDownLoadReq = DOWNLOAD;
    blockSequenceCounter = 0;
    memoryAddress = 0;
    memorySize = 0;
    for(uint8_t i = 0; i < malen; i++)
    {
        memoryAddress |= ((uint32_t)msg_buf[3 + i] << (i * 8));
    }
    for(uint8_t i = 0; i < mslen; i++)
    {
        memorySize |= ((uint32_t)msg_buf[3 + malen + i] << (i * 8));
    }

    if((mslen > 4 )|| (malen > 4 )) // 限制 memorySize parameter memoryAddress parameter 长度小于4字节
    {
        uds_negative_rsp(SID_34, NRC_REQUEST_OUT_OF_RANGE);
    }
    // 还需存判断下载地址和大小是否在正常范围内 通过下载地址判断发送的是flash driver 还是 app
    else
	{
        uds_positive_rsp(rsp_buf, 6);
    }
}

void ProgramDataToFlash(const uint8_t* msg_buf, uint16_t lenth)
{
    /*需要考虑加密压缩算法*/

}

/******************************************************************************
* 函数名称: void service_36_TransferData(const uint8_t* msg_buf, uint16_t msg_dlc)
* 功能说明: 36 服务 - 数据传输
* 输入参数: uint8_t*    msg_buf         --数据首地址
    　　　　uint8_t     msg_dlc         --数据长度
* 输出参数: 无
* 函数返回: 无
* 其它说明: 无
******************************************************************************/
void service_36_TransferData(const uint8_t* msg_buf, uint16_t msg_dlc)
{
    static uint8_t  LastblockSequenceCounter = 0;
    uint8_t rsp_buf[6];

    rsp_buf[0] = USD_GET_POSITIVE_RSP(SID_36);
	rsp_buf[1] = msg_buf[1]; // blockSequenceCounter

    LastblockSequenceCounter = blockSequenceCounter;
    blockSequenceCounter = msg_buf[1]; 

    if((UpDownLoadReq != DOWNLOAD) && (UpDownLoadReq != UPLOAD))
    {
        uds_negative_rsp(SID_36, NRC_REQUEST_SEQUENCE_ERROR);
    }
    else if((blockSequenceCounter - LastblockSequenceCounter) > 1)
    {
        uds_negative_rsp(SID_36, NRC_WRONG_BLOCK_SEQUENCE_COUNTER);
    }
    else
    {
        if(UpDownLoadReq == DOWNLOAD)
        {
            if(LastblockSequenceCounter == blockSequenceCounter)//不刷写，但回复正响应
            {
                // 重复的请求, 目标ECU会立即发送正响应消息, 而无需再次写入数据到其内存中
            }
            else
            {
                ProgramDataToFlash(msg_buf[2], msg_dlc - 2); // 刷写数据，msg_dlc长度减去SID 与 blockSequenceCounter
                // Flash Driver 需要先下载
                // 然后是App数据
            }
            uds_positive_rsp(rsp_buf, 2);
        }
    }
}

/******************************************************************************
* 函数名称: void service_37_RequestTransferExit(const uint8_t* msg_buf, uint16_t msg_dlc)
* 功能说明: 37 服务 - 请求结束传输
* 输入参数: uint8_t*    msg_buf         --数据首地址
    　　　　uint8_t     msg_dlc         --数据长度
* 输出参数: 无
* 函数返回: 无
* 其它说明: 无
******************************************************************************/
void service_37_RequestTransferExit(const uint8_t* msg_buf, uint16_t msg_dlc)
{
    uint8_t rsp_buf[6];

    rsp_buf[0] = USD_GET_POSITIVE_RSP(SID_37);

    blockSequenceCounter = msg_buf[1]; 

    if((UpDownLoadReq != DOWNLOAD) && (UpDownLoadReq != UPLOAD))
    {
        uds_negative_rsp(SID_37, NRC_REQUEST_SEQUENCE_ERROR); // 之前压根儿没收到过上传下载请求(0x34 or 0x35)
    }
    else
    {
        UpDownLoadReq = 0;
        uds_positive_rsp(rsp_buf, 1);   
    }
    	
}
/****************EOF****************/