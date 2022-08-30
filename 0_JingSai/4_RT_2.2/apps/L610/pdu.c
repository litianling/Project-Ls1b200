/*
 * pdu.c
 *
 * created: 2022/4/26
 *  author: 
 */

#include "stdio.h"
#include "string.h"
#include "pdu.h"




/*****************************************************************************************
功能:
	字符转整数

参数:
	u8Data		字符

返回值:
	转换后的整数
******************************************************************************************/
INT8U myatoi( INT8U u8Data )
{

	if ( u8Data >= '0' && u8Data <= '9' )
	{
		return ( u8Data - '0' );
	}
	else if ( u8Data >= 'A' && u8Data <= 'F' )
	{
		return ( u8Data - 'A' + 10 );
	}
	else if ( u8Data >= 'a' && u8Data <= 'f' )
	{
		return ( u8Data - 'a' + 10 );
	}
	else
	{
		return 0;
	}
}


/***************************************************************************************************************
功能：
	可打印字符串转换为字节数据
	如："C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}

参数：
	pSrc			源字符串指针
	pDst			目标数据指针
	u32SrcLen	源字符串长度

返回值：
	目标数据长度
****************************************************************************************************************/
INT32U PDU_String2Bytes(INT8U* pSrc, INT8U* pDst, INT32U u32SrcLen)
{
	INT32U	u32Ret = 0;
	INT32U	n;

	for ( n=0; n<u32SrcLen;  )
	{
		//输出高4位
		pDst[u32Ret] = myatoi(pSrc[n]) << 4 ;
		pDst[u32Ret] |= myatoi(pSrc[n+1]);

		n	+=	2;
		u32Ret++;
	}

	return u32Ret;
}

/***************************************************************************************************************
功能：
	两两颠倒的字符串转换为正常顺序的字符串
	如："683127226152F4" --> "8613722216254"

参数：
	pSrc			源字符串指针
	pDst			目标数据指针
	u32SrcLen	源字符串长度

返回值：
	目标数据长度
****************************************************************************************************************/
void PDU_SerializeNumbers( INT8U* pSrc, INT8U* pDst, INT8U u8SrcLen )
{
	INT8U n;

	for ( n=0; n<u8SrcLen; n+=2 )
	{
		pDst[n] 	= pSrc[n+1];
		pDst[n+1]	= pSrc[n];
	}

	if ( pDst[n-1] == 'F' )
	{
		pDst[n-1] = 0;
	}
	else
	{
		pDst[n]   = 0;
	}
}



/***************************************************************************************************************
功能：
	PDU  UCS2解码

参数：
	pSrc			源字符串指针
	pDst			目标数据指针
	u32SrcLen	源字符串长度

返回值：
	目标数据长度
****************************************************************************************************************/
INT8U PDU_UCS2_Decode( INT8U* pSrc, INT8U* pDst, INT8U u8SrcLen )
{
	INT8U	n = 0;
	INT8U	u8DestLen = 0;
	INT8U	u8High, u8Low;

	if ( u8SrcLen & 0x03 )
	{
		u8SrcLen = u8SrcLen >> 2;
		u8SrcLen = u8SrcLen << 2;
	}

	for ( ; n<u8SrcLen; )
	{
		u8High = (myatoi(pSrc[n])<<4) | myatoi(pSrc[n+1]);
		u8Low  = (myatoi(pSrc[n+2])<<4) | myatoi(pSrc[n+3]);

		if ( u8High == 0 )
		{
			pDst[u8DestLen++] = u8Low;
		}
		else
		{
			pDst[u8DestLen++] = u8High;
			pDst[u8DestLen++] = u8Low;
		}
		n += 4;
	}
	pDst[u8DestLen] = 0;

	return u8DestLen;
}


/***************************************************************************************************************
功能：
	PDU  7BIT解码
	GSM7编码规则就是：将第一个字符的最高位去掉，将第二个字符的最低位移入
	第一个字符的最高位，将第二个字符右移一位。此时第二个字符最高位空出两
	个bit。同理，将第三个字符的最低两位移入第二个字符的最高两位。一次类推，
	第八个字符的低7位移入第7个字符的高7位
	简单看来，就是将ASCII的字符串倒置，然后去掉每个字符的最高位，再倒置回来。
	“12345678”倒置为“87654321”：二进制串为：
	0011100000110111001101100011010100110100001100110011001000110001
	去掉每一字节的最高位：
	01110000-11011101-10110011-01010110-10001100-11011001-00110001

参数：
	pSrc			源字符串指针
	pDst			目标数据指针
	u32SrcLen	源字符串长度

返回值：
	目标数据长度
****************************************************************************************************************/
INT8U PDU_7BIT_Decode( INT8U* pSrc, INT8U* pDst, INT8U u8SrcLen )
{
	INT8U	u8Buff[7];
	INT8U	u8Mask[7] = {0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01};
	INT8U	n;
	INT8U	u8RemainLen = u8SrcLen;
	INT8U	u8CurLen;
	INT8U	u8OffSet = 0;
	INT8U	u8DestLen = 0;
	INT8U	u8Tail;

	if ( u8RemainLen & 1 )
	{
		u8RemainLen --;
	}

	while ( u8RemainLen > 0 )
	{
		u8CurLen = u8RemainLen;

		if ( u8RemainLen > 14 )
		{
			u8CurLen = 14;
		}

		for ( n=0; n<u8CurLen; n+=2 )
		{
			u8Buff[n>>1] = (myatoi(pSrc[u8OffSet+n])<<4 ) | myatoi( pSrc[u8OffSet+n+1]);
		}

		//将7比特变成8比特
		u8Tail = 0;
		for ( n=0; n<(u8CurLen>>1); n++ )
		{
			pDst[u8DestLen++] = ((u8Buff[n] & u8Mask[n])<<n) + u8Tail;
			u8Tail			  = (u8Buff[n] & (0xff-u8Mask[n]))>>(7-n);
		}
		//第0个字节的尾巴1位。。。第6个字节的尾巴7位
		//构成一个完整字节
		if ( n == 7 )
		{
			pDst[u8DestLen++] = u8Tail;
		}

		u8RemainLen -= u8CurLen;
		u8OffSet	+= u8CurLen;
	}
	pDst[u8DestLen++] = 0;

	return u8DestLen;
}

/***************************************************************************************************************
功能：
	PDU  7BIT解码

参数：
	pSrc			源字符串指针
	pDst			目标数据指针
	u32SrcLen	源字符串长度

返回值：
	目标数据长度
****************************************************************************************************************/
INT8U PDU_8BIT_Decode( INT8U* pSrc, INT8U* pDst, INT8U u8SrcLen )
{
	INT8U	n;

	if ( u8SrcLen & 1 )
	{
		u8SrcLen--;
	}

	for ( n=0; n<u8SrcLen; n+=2 )
	{
		pDst[n>>1] = (myatoi(pSrc[n])<<4) | myatoi(pSrc[n+1]);
	}
	pDst[u8SrcLen>>1] = 0;

	return (u8SrcLen>>1);
}


/***************************************************************************************************************
功能：
	PDU解码

参数：
	pu8Data		源数据，未解码的PDU短信
	u8Len		源数据长度
	psSmPara	解码后的数据存放地点

返回值：
	无
****************************************************************************************************************/
void PDU_Decode( INT8U *pu8Data, INT8U u8Len, SM_PARA *psSmPara )
{
	INT8U	*pSrc = pu8Data;
	INT8U	u8DestLen;
	INT8U	u8Tmp;

	//smsc信息
	PDU_String2Bytes(pSrc, &u8Tmp, 2);
	//SMSC字串长度
	u8Tmp = (u8Tmp - 1)<<1;
	//忽略SMSC地址格式
	pSrc += 4;

	//SMSC号码
	PDU_SerializeNumbers( pSrc, psSmPara->u8SCA, u8Tmp);
	pSrc += u8Tmp;

	//TPDU段参数，回复地址等
	PDU_String2Bytes(pSrc, &u8Tmp, 2);
	pSrc += 2;

	//取回复地址
	PDU_String2Bytes(pSrc, &u8Tmp, 2);
	if ( u8Tmp & 1 )
	{
		u8Tmp += 1;
	}
	//忽略TP-RA格式
	pSrc += 4;

	// 取TP-RA号码
	PDU_SerializeNumbers(pSrc, psSmPara->u8TPA, u8Tmp);
	pSrc += u8Tmp;

	//TPDU段协议标识、编码方式、用户信息等
	//取协议标识(TP-PID)
	PDU_String2Bytes(pSrc, &(psSmPara->u8TP_PID),2);
	pSrc += 2;

	//取编码方式TP-DCS
	PDU_String2Bytes(pSrc, &(psSmPara->u8TP_DCS),2);
	pSrc += 2;

	//服务时间戳字符串TP-SCTS
	PDU_SerializeNumbers(pSrc, psSmPara->u8TP_SCTS, 14);
	pSrc += 14;

	//用户信息长度 TP-UDL
	PDU_String2Bytes(pSrc, &u8Tmp, 2);
	pSrc += 2;

	if ( psSmPara->u8TP_DCS == PDU_USC2 )
	{
		//USC2解码
		PDU_UCS2_Decode(pSrc, psSmPara->u8TP_UD, u8Tmp<<1);
	}
	else if ( psSmPara->u8TP_DCS == PDU_7BIT )
	{
		//7bit解码
		u8DestLen = ( u8Tmp & 7) ? ((int)u8Tmp * 7 / 4 + 2) : ((int)u8Tmp * 7 / 4);
		PDU_7BIT_Decode(pSrc, psSmPara->u8TP_UD, u8DestLen);
	}
	else if ( psSmPara->u8TP_DCS == PDU_8BIT )
	{
		//8比特解码
		PDU_8BIT_Decode(pSrc, psSmPara->u8TP_UD, u8Tmp<<1);
	}
}

/*
int main()
{
	SM_PARA	sSmsPara;
	//char strEng[] = "0891683108705505F0040D91683115103340F600004150229072012305AA1A0C3602";
	char strEng[] = "0891683108100005F0000D91688108215684F40000224062410361231141EA6AD83C4E7B226499CD7E8B1A0AA";
	char strChn[] = "0891683108705505F0040D91683115103340F60008415022900312230C002A00350030003000235927";

	PDU_Decode( (INT8U *)strEng, strlen(strEng), &sSmsPara );

	printf("%s\r\n", (char*)sSmsPara.u8TP_UD);

	return 0;
}
*/
