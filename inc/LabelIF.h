#ifndef _LABLE_IF_H_
#define _LABLE_IF_H_

#include "Define.h"

#pragma pack(1)
enum _CPM_STREAM_TYPE
{
	STREAM_CHARS,			//�ַ���
	STREAM_BYTES,			//�ֽ���
    STREAM_BCD,				//BCD��
    STREAM_BYTES_33,		//��0x33�ֽ��������ܱ�ר��
	STREAM_BCD_2,			//BCD��2
	STREAM_CHARS_EMERSON	//BCD��2
};
//LabelIF_DecodeByFormat���ؽ��lsd
typedef struct DECODE__RESULT_
{
	int Ret;			//����������ɹ���ʧ�ܣ�
	int DataResult;		//���ݽ�����������쳣��
	int YCType;			//�쳣�����������ύAgent��
	int BZType;			//�Ǳ�׼���ݣ��ύso����������
	int TCType;			//͸�����䣨�ύ͸�����У����ύ��
}DECODE_RESULT, *PDECODE_RESULT;

typedef struct FORMAT__TIME_
{
    int TI;
    int M;
    int Y;
    int MO;
    int D;
    int H;
    int MI;
    int S;
}FORMAT_TIME, *PFORMAT_TIME;

typedef struct FORMAT__CHECK_
{
    int DS;       //������ʼλ��
    int DE;       //���ݽ���λ��
    int CS;       //У������ʼλ��
    int CE;       //У�������λ��
    int CL;       //У���볤��
    int T;        //У�����������
    int C;        //У�����Ƿ���иߵ�λ�任
}FORMAT_CHECK, *PFORMAT_CHECK;

typedef struct FORMAT__HEAD_
{
    char DT[5];              //���ݵ�λ���ͣ����¶�(01)��ʪ��(02)��
	int ST;                  //��������
	int T;                   //�������ͣ������Ρ������͡��ַ�����
	char BZ[2];				//�Ƿ��׼����lsd
	char TC[2];				//�Ƿ�͸������lsd
}FORMAT_HEAD, *PFORMAT_HEAD;

typedef struct FORMAT__PARA_
{
	int DataLen;        //���ݳ���
	int ChangeBytes;        //�ߵ�λ�任
	int DataType;        //��ֵ����
	int StartBytes;        //��ʼλ��
	int EndBytes;
    int RT;       //ϵ������  0���� 1��� 2��ȡ���
    int RL;       //��ϵ����Ҫ��ȡʱ����ȡ����
    int RST;      //ϵ����ֵ����
    float R;      //������ϵ��,��ȡʱΪ��ʼ��ַ
    char DC[3];      //���ݼ��㷽��   ֱ�ӽ�ȡ/ϵ��ֵ/��ȡֵ*ϵ��ֵ
	int LenType;        //���ݳ������ͣ�0���棬1����
	int BitPos;		//��λȡ��ֵʱ��λ��
}FORMAT_PARA, *PFORMAT_PARA;

typedef struct FORMAT_MAP_TABLE_
{
	float ValueX;
	float ValueY;
}FORMAT_MAP_TABLE, *PFORMAT_MAP_TABLE;


#pragma pack()

/******************************************************************************/
/*						 LabelIF   ��������                                             */
/******************************************************************************/
void 	LabelIF_GetDeviceQueryPrivatePara(char* config_str, PDEVICE_INFO_POLL stDeviceInfoPoll);
void 	LabelIF_GetDeviceAttrQueryPrivatePara(char* config_str, PDEVICE_INFO_POLL stDeviceInfoPoll);
void 	LabelIF_GetDeviceActionPrivatePara(char* config_str, PDEVICE_INFO_ACT stDeviceInfoPoll);
void 	LabelIF_GetDeviceValueLimit(char* config_str, PDEVICE_INFO_POLL stDeviceInfoPoll);
//��ȡ����ʱ������
void	LabelIF_GetDefenceDate(char *strTimeSet, DEVICE_INFO_POLL *hash_node);
//
int 	LabelIF_DecodeActionResponse(PDEVICE_INFO_ACT device_node, unsigned char* buf, int bufLen);
bool 	LabelIF_CheckRecvFormat(char* checkFormat, unsigned char* buf, int bufLen);
//
int 	LabelIF_EncodeFormat(char* format, char* self_id, char* szCmd, char* pInData, unsigned char* pOutBuf, int nBufSize);
int 	LabelIF_DecodeByFormat(PDEVICE_INFO_POLL pDevInfo, unsigned char* pInBuf, int inLen, PQUERY_INFO_CPM_EX stOutBuf);
int 	LabelIF_RangeCheck(PDEVICE_INFO_POLL pDevInfo, int vType, char* recValue, char* lastValue);
bool 	LabelIF_StandarCheck(int vType, char* value, char* standard);
/******************************************************************************/
/*						 ˽�к���                                             */
/******************************************************************************/
char* GetLable(char* format, char* lableName, int& len, char* outBuf);
char* GetCalLable(char* format, char* lableName, int sublableName, int& len, char* outBuf, unsigned char* tableType);

int GenerateTable(const char* split1, const char* split2, char* inData, PFORMAT_MAP_TABLE mapTable, int size);//L=1,C=1
int GetHeadValue(const char* split, char* Inbuf, PFORMAT_HEAD pFormatHead);
int GetCheckValue(const char* split, char* Inbuf, PFORMAT_CHECK pFormatCheck);
int GetValue(const char* Type, char* Inbuf, PFORMAT_PARA pFormatPara);
int GetValueTime(const char* Type, char* Inbuf, PFORMAT_TIME pFormatTime);
int GetValueByTable(unsigned char calType, float calValue, unsigned char tableType, void* curValue, PFORMAT_PARA rdFormat, FORMAT_HEAD headFormat, PFORMAT_MAP_TABLE mapTable, int mapSize, float* outValue);


int strindex(char *str,char *substr);
int define_sscanf(char *src, char *dst, char* outBuf);

int FormatID(char* format, char* id, char* outBuf, int nBufSize);
int FormatCMD(char* format, char* szCmd, char* outBuf, int nBufSize);
int FormatData(char* format, char* data, char* outBuf, int nBufSize);
int FormatCheck(char* format, char* outBuf, int nBufSize);
int DecodeByFormatEx(const char* D_Format, char* Comparison_table, char* devId,  unsigned char* pInBuf, int inLen, char* stOutBuf);

bool f_valueLimitCheck(int vType, char* value, char* standard, char* LastValue);


#endif
