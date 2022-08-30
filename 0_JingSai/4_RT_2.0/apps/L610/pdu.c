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
����:
	�ַ�ת����

����:
	u8Data		�ַ�

����ֵ:
	ת���������
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
���ܣ�
	�ɴ�ӡ�ַ���ת��Ϊ�ֽ�����
	�磺"C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}

������
	pSrc			Դ�ַ���ָ��
	pDst			Ŀ������ָ��
	u32SrcLen	Դ�ַ�������

����ֵ��
	Ŀ�����ݳ���
****************************************************************************************************************/
INT32U PDU_String2Bytes(INT8U* pSrc, INT8U* pDst, INT32U u32SrcLen)
{
	INT32U	u32Ret = 0;
	INT32U	n;

	for ( n=0; n<u32SrcLen;  )
	{
		//�����4λ
		pDst[u32Ret] = myatoi(pSrc[n]) << 4 ;
		pDst[u32Ret] |= myatoi(pSrc[n+1]);

		n	+=	2;
		u32Ret++;
	}

	return u32Ret;
}

/***************************************************************************************************************
���ܣ�
	�����ߵ����ַ���ת��Ϊ����˳����ַ���
	�磺"683127226152F4" --> "8613722216254"

������
	pSrc			Դ�ַ���ָ��
	pDst			Ŀ������ָ��
	u32SrcLen	Դ�ַ�������

����ֵ��
	Ŀ�����ݳ���
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
���ܣ�
	PDU  UCS2����

������
	pSrc			Դ�ַ���ָ��
	pDst			Ŀ������ָ��
	u32SrcLen	Դ�ַ�������

����ֵ��
	Ŀ�����ݳ���
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
���ܣ�
	PDU  7BIT����
	GSM7���������ǣ�����һ���ַ������λȥ�������ڶ����ַ������λ����
	��һ���ַ������λ�����ڶ����ַ�����һλ����ʱ�ڶ����ַ����λ�ճ���
	��bit��ͬ�����������ַ��������λ����ڶ����ַ��������λ��һ�����ƣ�
	�ڰ˸��ַ��ĵ�7λ�����7���ַ��ĸ�7λ
	�򵥿��������ǽ�ASCII���ַ������ã�Ȼ��ȥ��ÿ���ַ������λ���ٵ��û�����
	��12345678������Ϊ��87654321���������ƴ�Ϊ��
	0011100000110111001101100011010100110100001100110011001000110001
	ȥ��ÿһ�ֽڵ����λ��
	01110000-11011101-10110011-01010110-10001100-11011001-00110001

������
	pSrc			Դ�ַ���ָ��
	pDst			Ŀ������ָ��
	u32SrcLen	Դ�ַ�������

����ֵ��
	Ŀ�����ݳ���
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

		//��7���ر��8����
		u8Tail = 0;
		for ( n=0; n<(u8CurLen>>1); n++ )
		{
			pDst[u8DestLen++] = ((u8Buff[n] & u8Mask[n])<<n) + u8Tail;
			u8Tail			  = (u8Buff[n] & (0xff-u8Mask[n]))>>(7-n);
		}
		//��0���ֽڵ�β��1λ��������6���ֽڵ�β��7λ
		//����һ�������ֽ�
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
���ܣ�
	PDU  7BIT����

������
	pSrc			Դ�ַ���ָ��
	pDst			Ŀ������ָ��
	u32SrcLen	Դ�ַ�������

����ֵ��
	Ŀ�����ݳ���
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
���ܣ�
	PDU����

������
	pu8Data		Դ���ݣ�δ�����PDU����
	u8Len		Դ���ݳ���
	psSmPara	���������ݴ�ŵص�

����ֵ��
	��
****************************************************************************************************************/
void PDU_Decode( INT8U *pu8Data, INT8U u8Len, SM_PARA *psSmPara )
{
	INT8U	*pSrc = pu8Data;
	INT8U	u8DestLen;
	INT8U	u8Tmp;

	//smsc��Ϣ
	PDU_String2Bytes(pSrc, &u8Tmp, 2);
	//SMSC�ִ�����
	u8Tmp = (u8Tmp - 1)<<1;
	//����SMSC��ַ��ʽ
	pSrc += 4;

	//SMSC����
	PDU_SerializeNumbers( pSrc, psSmPara->u8SCA, u8Tmp);
	pSrc += u8Tmp;

	//TPDU�β������ظ���ַ��
	PDU_String2Bytes(pSrc, &u8Tmp, 2);
	pSrc += 2;

	//ȡ�ظ���ַ
	PDU_String2Bytes(pSrc, &u8Tmp, 2);
	if ( u8Tmp & 1 )
	{
		u8Tmp += 1;
	}
	//����TP-RA��ʽ
	pSrc += 4;

	// ȡTP-RA����
	PDU_SerializeNumbers(pSrc, psSmPara->u8TPA, u8Tmp);
	pSrc += u8Tmp;

	//TPDU��Э���ʶ�����뷽ʽ���û���Ϣ��
	//ȡЭ���ʶ(TP-PID)
	PDU_String2Bytes(pSrc, &(psSmPara->u8TP_PID),2);
	pSrc += 2;

	//ȡ���뷽ʽTP-DCS
	PDU_String2Bytes(pSrc, &(psSmPara->u8TP_DCS),2);
	pSrc += 2;

	//����ʱ����ַ���TP-SCTS
	PDU_SerializeNumbers(pSrc, psSmPara->u8TP_SCTS, 14);
	pSrc += 14;

	//�û���Ϣ���� TP-UDL
	PDU_String2Bytes(pSrc, &u8Tmp, 2);
	pSrc += 2;

	if ( psSmPara->u8TP_DCS == PDU_USC2 )
	{
		//USC2����
		PDU_UCS2_Decode(pSrc, psSmPara->u8TP_UD, u8Tmp<<1);
	}
	else if ( psSmPara->u8TP_DCS == PDU_7BIT )
	{
		//7bit����
		u8DestLen = ( u8Tmp & 7) ? ((int)u8Tmp * 7 / 4 + 2) : ((int)u8Tmp * 7 / 4);
		PDU_7BIT_Decode(pSrc, psSmPara->u8TP_UD, u8DestLen);
	}
	else if ( psSmPara->u8TP_DCS == PDU_8BIT )
	{
		//8���ؽ���
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
