/*
 * pdu.h
 *
 * created: 2022/4/26
 *  author: 
 */

#ifndef _PDU_H
#define _PDU_H

typedef unsigned char			INT8U;
typedef signed char				INT8S;
typedef unsigned short int		INT16U;
typedef	signed short int		INT16S;
typedef unsigned int			INT32U;
typedef signed int				INT32S;

//用户信息编码方式
#define		PDU_7BIT		0
#define		PDU_8BIT		4
#define		PDU_USC2		8

//短消息参数结构，编码解码共用
typedef	struct __sm_para__
{
	INT8U	u8SCA[16];		//短消息服务中心号码
	INT8U	u8TPA[16];		//目标号码，回复号码
	INT8U	u8TP_PID;		//用户信息协议标识
	INT8U	u8TP_DCS;		//用户信息编码方式
	INT8U	u8TP_SCTS[16];	//服务时间戳字符串
	INT8U	u8TP_UD[161];	//原始用户信息，编码前或解码后
}SM_PARA, *PSM_PARA;


INT8U myatoi( INT8U u8Data );
INT32U PDU_String2Bytes(INT8U* pSrc, INT8U* pDst, INT32U u32SrcLen);
void PDU_SerializeNumbers( INT8U* pSrc, INT8U* pDst, INT8U u8SrcLen );
INT8U PDU_UCS2_Decode( INT8U* pSrc, INT8U* pDst, INT8U u8SrcLen );
INT8U PDU_7BIT_Decode( INT8U* pSrc, INT8U* pDst, INT8U u8SrcLen );
INT8U PDU_8BIT_Decode( INT8U* pSrc, INT8U* pDst, INT8U u8SrcLen );
void PDU_Decode( INT8U *pu8Data, INT8U u8Len, SM_PARA *psSmPara );


#endif // _PDU_H

