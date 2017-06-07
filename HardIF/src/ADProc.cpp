/******************************************************************************/
/*    File    : switch_config.c                                               */
/*    Author  :                                                           */
/*    Creat   :                                                       */
/*    Function:                                                         */
/******************************************************************************/

/******************************************************************************/
/*                       ͷ�ļ�                                               */
/******************************************************************************/
#include <sys/time.h>
#include <sys/ioctl.h>
#include <string>
#include <stdlib.h>
#include <error.h>
#include "Shared.h"
#include "IniFile.h"
#include "ThreadPool.h"

#include "ADProc.h"

#include "Util.h"
#include "EmApi.h"


#ifdef  DEBUG
//#define DEBUG_IF
#endif

#ifdef DEBUG_IF
#define DBG_IF(a)		printf a;
#else
#define DBG_IF(a)	
#endif

/******************************************************************************/
/*                       �ֲ��궨��                                           */
/******************************************************************************/

int g_QueryIndex[25] = 
{
	//ģ��������A��
	INTERFACE_AD_A0,
	INTERFACE_AD_A1,
	INTERFACE_AD_A2,
	INTERFACE_AD_A3,

	//ģ��������B��
	INTERFACE_AD_B0,
	INTERFACE_AD_B1,
	INTERFACE_AD_B2,
	INTERFACE_AD_B3,

	//����������A��
	INTERFACE_DIN_A0,
	INTERFACE_DIN_A1,
	INTERFACE_DIN_A2,
	INTERFACE_DIN_A3,         //20

	//����������B��
	INTERFACE_DIN_B0,
	INTERFACE_DIN_B1,
	INTERFACE_DIN_B2,
	INTERFACE_DIN_B3,

	//�������������
	INTERFACE_DOUT_A2,
	INTERFACE_DOUT_A4,
	INTERFACE_DOUT_B2,
	INTERFACE_DOUT_B4,

	//�������������
	INTERFACE_DOUT_A1,
	INTERFACE_DOUT_A3,
	INTERFACE_DOUT_B1,
	INTERFACE_DOUT_B3,

	//�̵�����Դ���
	INTERFACE_AV_OUT_1
};
/******************************************************************************/
/*                       �ֲ�����                                             */
/******************************************************************************/

CADProc::CADProc(CADCBase* base_A, CADCBase* base_B)
{
	m_ADCBase_A = base_A;
	m_ADCBase_B = base_B;
	for (int i=0; i<25; i++)
	{
		memset((unsigned char*)&m_strDeviceAD[i], 0, sizeof(STR_DEVICE_AD));
		m_strDeviceAD[i].iChannel = g_QueryIndex[i];
		m_strDeviceAD[i].fValue = 0.00;
	}
	get_ini_int("Config.ini","INTER_CONFIG","TTLSampleCount", 10, &m_iTTLSampleCount);
	get_ini_int("Config.ini","INTER_CONFIG","ADSampleCount", 30, &m_iADSampleCount);
	if(0 == m_iTTLSampleCount)
	{
		m_iTTLSampleCount = 10;
	}
	if(0 == m_iADSampleCount)
	{
		m_iADSampleCount = 30;
	}
}

CADProc::~CADProc(void)
{		
	Terminate();
}

bool CADProc::Terminate()
{
	TRACE(CADProc::Terminate());

	if(0xFFFF != m_MainThrdHandle)
	{
		ThreadPool::Instance()->SetThreadMode(m_MainThrdHandle, false);
		if (pthread_cancel(m_MainThrdHandle)!=0)
			;//DBGLINE;
	}
	return true;
}

/******************************************************************************/
/* Function Name  : Initialize                                            */
/* Description    : �ӿڳ�ʼ��                                              */
/* Input          : int iChannelId         ͨ����                               */
/*                  TKU32 unHandleCtrl     ���������ڴ���                             */
/* Output         : none                                                      */
/* Return         : none                                                      */
/******************************************************************************/
bool CADProc::Initialize(CADCValueNotice_CallBack ADCNotic_CallBack, void* userData)
{
	//�豸��ѯ�߳�
	char buf[256] = {0};	
	sprintf(buf, "%s %s", "CADProc", "CADProcThrd");
	if (!ThreadPool::Instance()->CreateThread(buf, &CADProc::ADThrd, true,this))
	{
		DBG(("CADProc Create failed!\n"));
		return false;
	}
	NoticeCallBack = ADCNotic_CallBack;
	callbackData = userData;
	return true; 
}
/************************************************************************/
/* ��������ģ�������豸                                                 */
/************************************************************************/
//�豸��ѯ�߳�
void * CADProc::ADThrd(void *arg)
{
	CADProc *_this = (CADProc *)arg;
	_this->ADThrdProc();
	return NULL;
}

/******************************************************************************/
/* Function Name  : CommMsgCtrlFunc                                           */
/* Description    : �����豸��ѯ����                                          */
/* Input          : none                                                      */
/* Output         : none                                                      */
/* Return         : none                                                      */
/******************************************************************************/
void CADProc::ADThrdProc(void)
{
	usleep(500*1000);
	m_MainThrdHandle = pthread_self();

	printf("CADProc::ADThrdProc Start .... Pid[%d]\n", getpid());
	while(1)
	{
		pthread_testcancel();
		for(int i=0; i<25; i++)
		{
			PollProc(i);
		}
		usleep(10*1000);
	}
}

int CADProc::PollProc(int index)
{
	int ret = SYS_STATUS_FAILED;
	float fTemp = 0.0;
	do 
	{
		switch(m_strDeviceAD[index].iChannel)
		{	
			//����ģ��������
		case INTERFACE_AD_A0:
		case INTERFACE_AD_A1:
		case INTERFACE_AD_A2:
		case INTERFACE_AD_A3:
			{
				CLockAD.lock();
				if (m_ADCBase_A->ADGet(m_strDeviceAD[index].iChannel-INTERFACE_AD_A0, fTemp))
				{
					if(fTemp >= 4.0 && fTemp <= 35.0)
					{
						for(int i=0; i<m_iADSampleCount-1; i++)
						{
							m_strDeviceAD[index].fTempValue[i] = m_strDeviceAD[index].fTempValue[i+1];
						}
						m_strDeviceAD[index].fTempValue[m_iADSampleCount-1] = fTemp;
						m_strDeviceAD[index].iErrCount = 0;
						ret = SYS_STATUS_SUCCESS;
					}
					else
					{
						m_strDeviceAD[index].iErrCount++;
						if (0 != m_strDeviceAD[index].iErrCount && 0 == m_strDeviceAD[index].iErrCount%20)
						{
							m_strDeviceAD[index].iErrCount = 30;
							m_strDeviceAD[index].fValue = 0.0;
						}
						ret = SYS_STATUS_SUCCESS;
					}
				}
				CLockAD.unlock();
			}
			break;
			//��ѹģ��������
		case INTERFACE_AD_B0:
		case INTERFACE_AD_B1:
		case INTERFACE_AD_B2:
		case INTERFACE_AD_B3:
			{
				CLockAD.lock();
				if (m_ADCBase_B->ADGet(m_strDeviceAD[index].iChannel-INTERFACE_AD_B0, fTemp))
				{
					if(fTemp >= 0.0 && fTemp <=10.0)
					{						
						for(int i=0; i<m_iADSampleCount-1; i++)
						{
							m_strDeviceAD[index].fTempValue[i] = m_strDeviceAD[index].fTempValue[i+1];
						}
						m_strDeviceAD[index].fTempValue[m_iADSampleCount-1] = fTemp;
						m_strDeviceAD[index].iErrCount = 0;
						ret = SYS_STATUS_SUCCESS;
					}
					else
					{
						m_strDeviceAD[index].iErrCount++;
						if (0 != m_strDeviceAD[index].iErrCount && 0 == m_strDeviceAD[index].iErrCount%20)
						{
							m_strDeviceAD[index].iErrCount = 30;
							m_strDeviceAD[index].fValue = 0.0;
						}
						ret = SYS_STATUS_SUCCESS;
					}
				}	
				CLockAD.unlock();
			}
			break;
			//����������A��
		case INTERFACE_DIN_A0:
		case INTERFACE_DIN_A1:
		case INTERFACE_DIN_A2:
		case INTERFACE_DIN_A3:
			{
				unsigned char diValue = 0;
				bool needUpdate = false;
				STR_DEVICE_AD strTemp;
				CLockAD.lock();
				if (m_ADCBase_A->DIGet(m_strDeviceAD[index].iChannel - INTERFACE_DIN_A0, diValue))
				{
					if(m_strDeviceAD[index].fValue != (float)diValue)//ֵ�����仯
					{
						//DBG(("ChannelId[%d] Value[%f] oldValue[%f] count[%d]\n", m_strDeviceAD[index].iChannel-INTERFACE_DIN_A0, m_strDeviceAD[index].fValue, (float)diValue, m_strDeviceAD[index].iTempCount));
						if (m_iTTLSampleCount == m_strDeviceAD[index].iTempCount + 1)
						{
							needUpdate = true;
							m_strDeviceAD[index].iTempCount = 0;
							m_strDeviceAD[index].fValue = (float)diValue;							
							memcpy((unsigned char*)&strTemp, (unsigned char*)&m_strDeviceAD[index], sizeof(STR_DEVICE_AD));
						}
						else
							m_strDeviceAD[index].iTempCount++;
					}
					else
						m_strDeviceAD[index].iTempCount = 0;
					ret = SYS_STATUS_SUCCESS;
				}
				CLockAD.unlock();
				if(needUpdate)
					NoticeCallBack(callbackData, strTemp.iChannel, (unsigned char*)&diValue, sizeof(unsigned char));
			}
			break;
			//����������B��
		case INTERFACE_DIN_B0:
		case INTERFACE_DIN_B1:
		case INTERFACE_DIN_B2:
		case INTERFACE_DIN_B3:
			{
				unsigned char diValue = 0;
				bool needUpdate = false;
				STR_DEVICE_AD strTemp;
				CLockAD.lock();
				if (m_ADCBase_B->DIGet(m_strDeviceAD[index].iChannel - INTERFACE_DIN_B0, diValue))
				{
					if(m_strDeviceAD[index].fValue != (float)diValue)
					{
						//DBG(("ChannelId[%d] Value[%f] oldValue[%f] count[%d]\n", m_strDeviceAD[index].iChannel-INTERFACE_DIN_A0, m_strDeviceAD[index].fValue, (float)diValue, m_strDeviceAD[index].iTempCount));
						if (m_iTTLSampleCount == m_strDeviceAD[index].iTempCount + 1)
						{
							needUpdate = true;
							m_strDeviceAD[index].iTempCount = 0;
							m_strDeviceAD[index].fValue = (float)diValue;
							memcpy((unsigned char*)&strTemp, (unsigned char*)&m_strDeviceAD[index], sizeof(STR_DEVICE_AD));
						}
						else
							m_strDeviceAD[index].iTempCount++;
					}
					else
						m_strDeviceAD[index].iTempCount = 0;
					ret = SYS_STATUS_SUCCESS;
				}	
				CLockAD.unlock();
				if(needUpdate)
					NoticeCallBack(callbackData, strTemp.iChannel, (unsigned char*)&diValue, sizeof(unsigned char));
			}
			break;
		}
	} while (false);
	return ret;
}

/*
�ⲿ���ú���
*/
int CADProc::GetADValue(int iChannel, float& fOutValue)
{
	CLockAD.lock();
	int ret = SYS_STATUS_FAILED;
	switch(iChannel)
	{	
		//����ģ��������
	case INTERFACE_AD_A0:
	case INTERFACE_AD_A1:
	case INTERFACE_AD_A2:
	case INTERFACE_AD_A3:
		//��ѹģ��������
	case INTERFACE_AD_B0:
	case INTERFACE_AD_B1:
	case INTERFACE_AD_B2:
	case INTERFACE_AD_B3:
		for (int i=0; i<25; i++)
		{
			if (m_strDeviceAD[i].iChannel == iChannel && m_strDeviceAD[i].iErrCount < 30)
			{
				float szTempValue[512] = {0};
				memcpy((unsigned char*)szTempValue, (unsigned char*)&m_strDeviceAD[i].fTempValue[0], m_iADSampleCount*sizeof(float));
				QuickSort(szTempValue, m_iADSampleCount);
				int count = m_iADSampleCount/3;
				float fTempValue = 0.0;
				for(int j=count; j<(count*2 + m_iADSampleCount%3); j++)
				{
					fTempValue += szTempValue[j];
			//		DBG(("getAdValue:iChannel[%d] fTempValue[%f]\n", m_strDeviceAD[i].iChannel, szTempValue[j]));

				}
				fOutValue = fTempValue/(count + m_iADSampleCount%3);
				ret = SYS_STATUS_SUCCESS;
				break;
			}
		}
		break;
		//����������A��
	case INTERFACE_DIN_A0:
	case INTERFACE_DIN_A1:
	case INTERFACE_DIN_A2:
	case INTERFACE_DIN_A3:
		//����������B��
	case INTERFACE_DIN_B0:
	case INTERFACE_DIN_B1:
	case INTERFACE_DIN_B2:
	case INTERFACE_DIN_B3:

	case INTERFACE_DOUT_A1:
	case INTERFACE_DOUT_A2:
	case INTERFACE_DOUT_A3:
	case INTERFACE_DOUT_A4:
	case INTERFACE_DOUT_B1:
	case INTERFACE_DOUT_B2:
	case INTERFACE_DOUT_B3:
	case INTERFACE_DOUT_B4:
	case INTERFACE_AV_OUT_1:
		for (int i=0; i<25; i++)
		{			
			if (m_strDeviceAD[i].iChannel == iChannel && m_strDeviceAD[i].iErrCount < 30)
			{
				DBG(("getAdValue: NeedChannel[%d] ChannelId[%d] Value[%f] count[%d] ErrorC[%d]\n", iChannel, m_strDeviceAD[i].iChannel, m_strDeviceAD[i].fValue, m_strDeviceAD[i].iTempCount,  m_strDeviceAD[i].iErrCount));
				fOutValue = m_strDeviceAD[i].fValue;
				ret = SYS_STATUS_SUCCESS;
				break;
			}
		}
		break;
	}
	CLockAD.unlock();
	return ret;
}

int CADProc::SetADValue(int iChannel, int iOutValue)
{
	int ret = SYS_STATUS_FAILED;
	CLockAD.lock();
	switch(iChannel)
	{	//���������A��
		case INTERFACE_DOUT_A1:
		case INTERFACE_DOUT_A2:
		case INTERFACE_DOUT_A3:
		case INTERFACE_DOUT_A4:
			ret = SYS_STATUS_SUCCESS;
			switch(iOutValue)
			{
			case 0:
			case 1:
				m_ADCBase_A->DOSet(iChannel, iOutValue);
				break;
			case 2://300ms����,lhy
				m_ADCBase_A->DOSet(iChannel, 0);
				usleep(300*1000);
				m_ADCBase_A->DOSet(iChannel, 1);
				break;
			case 3://300ms����,lhy
				m_ADCBase_A->DOSet(iChannel, 1);
				usleep(300*1000);
				m_ADCBase_A->DOSet(iChannel, 0);
				break;
			}
		//���������B��
		case INTERFACE_DOUT_B1:
		case INTERFACE_DOUT_B2:
		case INTERFACE_DOUT_B3:
		case INTERFACE_DOUT_B4:

			ret = SYS_STATUS_SUCCESS;	
			switch(iOutValue)
			{
			case 0:
			case 1:
				m_ADCBase_B->DOSet(iChannel, iOutValue);
				break;
			case 2://300ms����,lhy
				m_ADCBase_B->DOSet(iChannel, 0);
				usleep(300*1000);
				m_ADCBase_B->DOSet(iChannel, 1);
				break;
			case 3://300ms����,lhy
				m_ADCBase_B->DOSet(iChannel, 1);
				usleep(300*1000);
				m_ADCBase_B->DOSet(iChannel, 0);
				break;
			}
			break;
		//�̵������
		case INTERFACE_AV_OUT_1:
			ret = SYS_STATUS_SUCCESS;	
			m_ADCBase_A->DOSetE(5, iOutValue);
			break;
	}
	CLockAD.unlock();
	return ret;
}

/*************************************  END OF FILE *****************************************/
