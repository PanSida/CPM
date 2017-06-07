#ifndef _HARDIF_H_
#define _HARDIF_H_
#include "Define.h"

typedef struct _STR_SMS_CFG
{
	int ulHandleGSMSend;	//���Ͷ��о��
	int ulHandleGSMRecv;	//���ն��о��
}STR_SMS_CFG,*PSTR_SMS_CFG;

typedef int (*ADCValueNotice_CallBack)(void* userData, int iChannel, unsigned char* value, int len);//�����������ݱ仯֪ͨ�ص�
/******************************************************************************/
/*                    �豸�ͺš�������CPM��                                   */
/******************************************************************************/
//��ʼ��
bool 	HardIF_Initialize(ADCValueNotice_CallBack ADCNotic_CallBack, void* userData, STR_SMS_CFG strSmsCfg);
bool	HardIF_Terminate();
//��������ģ������д
int 	HardIF_GetADValue(int Id, float& fInValue);
int 	HardIF_SetADValue(int Id, int iOutValue);
//���ŷ��͡�����绰
int 	HardIF_DoGSMAction(PGSM_MSG);
int		HardIF_GetGSMMsg(GSM_MSG& outMsg);
int		HardIF_DeleteSms(int iIndex);
char*	HardIF_GetGSMIMSI();
//�Ʋ���
void 	HardIF_BackLightSet( int portNum, int status );
void 	HardIF_AllLightOFF();
void 	HardIF_AllLightON();
void 	HardIF_GSMLightOFF();
void 	HardIF_GSMLightON();
void	HardIF_DBGLightOFF();
void	HardIF_DBGLightON();
void	HardIF_PLALightOFF();
void	HardIF_PLALightON();
//����GPIO����
int 	HardIF_GetGD0();
//485����
//ÿ��485��newһ���࣬��ֹ��ͻ
class CHardIF_485
{
public:
	CHardIF_485(int upperId, int iBaudRate, int iDataBytes, char cCheckMode, int iStopBit, int iReadTimeOut, int typeProc);
	//destructor
	virtual ~CHardIF_485(void);
public:
	int Exec485(unsigned char* respBuf, int& nRespLen, BYTE* reqBuf, int nReqLen);
private:
	int m_upperId;
	int m_iBaudRate;
	int m_iDataBytes;
	char m_cCheckMode;
	int m_iStopBit;
	int m_iReadTimeOut;
	
	int m_typeProc;				//0,PollProc; 1,ActionProc;
};

#pragma pack()
#endif
