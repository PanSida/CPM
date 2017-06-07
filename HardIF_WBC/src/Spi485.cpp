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

#include "Util.h"
#include "EmApi.h"

/******************************************************************************/
/*                       �ֲ��궨��                                           */
/******************************************************************************/

/******************************************************************************/
/*                       �ֲ�����                                             */
/******************************************************************************/

CDeviceIfSpi485::CDeviceIfSpi485()
{
	memset(m_szCurDeviceAttrId, 0, sizeof(m_szCurDeviceAttrId));
}

CDeviceIfSpi485::~CDeviceIfSpi485(void)
{		
	Terminate();
}

bool CDeviceIfSpi485::Terminate()
{
	TRACE(CDeviceIfBase::Terminate());

	return true;
}

/******************************************************************************/
/* Function Name  : Initialize                                                */
/* Description    : �ӿڳ�ʼ��                                                */
/* Input          : int iChannelId         ͨ����                             */
/*                  TKU32 unHandleCtrl     ���������ڴ���                   */
/* Output         : none                                                      */
/* Return         : none                                                      */
/******************************************************************************/
int CDeviceIfSpi485::Initialize(int iChannelId, TKU32 unHandleCtrl)
{
	int ret = SYS_STATUS_FAILED;
	if(SYS_STATUS_SUCCESS != (ret = CDeviceIfBase::Initialize(iChannelId, unHandleCtrl))) 
		return ret;

	ret = SYS_STATUS_SUCCESS;
	return ret; 
}

/******************************************************************************/
/* Function Name  : PollProc                                                  */
/* Description    : ��Ѳִ�к���		                                      */
/* Input          : PDEVICE_INFO_POLL device_node	            ��Ѳ���      */
/* Output         : none													  */
/* Return         : int ret          0:�ɹ���1:�豸����Ӧ                     */
/******************************************************************************/
int CDeviceIfSpi485::PollProc(PDEVICE_INFO_POLL device_node, unsigned char* respBuf, int& nRespLen)
{
	int ret = SYS_STATUS_FAILED;
	BYTE reqBuf[MAX_PACKAGE_LEN_TERMINAL] = {0};
	int nLen = 0;
	int baudRateEnum = 0;

	nRespLen = 0;	

	DBG(("CDeviceIfSpi485::PollProc  id[%s]\n", device_node->Id));
	//����
	lightStatusSet(m_iChannelId, LIGHT_ON);
	do 
	{
		if (0 >= strlen(device_node->R_Format))
			break;

		//��֯����
		nLen = EncodeFormat(device_node->R_Format,device_node->Self_Id, NULL, NULL, reqBuf, sizeof(reqBuf));
		if (0 >= nLen)
			break;

		switch(m_iChannelId)
		{
		case INTERFACE_RS485_A0:
		case INTERFACE_RS485_A1:
		case INTERFACE_RS485_A2:
			{
				do 
				{ 
					baudRateEnum = CADCBase::f_getBaudRateEnum( device_node->BaudRate );
					g_CADCBaseA.SetBaudRate(m_iChannelId, (ADC_BAUDRATE)baudRateEnum);
					if( device_node->checkMode[0] == 'O' )
					{
						g_CADCBaseA.SetSerialOpt(m_iChannelId, device_node->stopbyte, 1);
					}
					else if( device_node->checkMode[0] == 'E' )
					{
						g_CADCBaseA.SetSerialOpt(m_iChannelId, device_node->stopbyte, 2);
					}
					else
					{
						g_CADCBaseA.SetSerialOpt(m_iChannelId, device_node->stopbyte, 0);
					}

					//�򴮿ڷ�������
					if (nLen > g_CADCBaseA.SerialSend(m_iChannelId, reqBuf, nLen))
					{
						ret = SYS_STATUS_DEVICE_COMMUNICATION_ERROR;
						break;
					}

					nRespLen = g_CADCBaseA.SerialRecv(m_iChannelId, respBuf, MAX_PACKAGE_LEN_TERMINAL, device_node->timeOut);
					if (nRespLen > 0)
					{
						ret = SYS_STATUS_SUCCESS;
					}
					else
					{
						ret = SYS_STATUS_DEVICE_COMMUNICATION_ERROR;
					}
				} while (false);           
			}
			break;

			//485�ӿ�B��
		case INTERFACE_RS485_B0:
		case INTERFACE_RS485_B1:
		case INTERFACE_RS485_B2:
			{
				do 
				{ 
					baudRateEnum = CADCBase::f_getBaudRateEnum( device_node->BaudRate );
					g_CADCBaseB.SetBaudRate(m_iChannelId, (ADC_BAUDRATE)baudRateEnum);
					if( device_node->checkMode[0] == 'O' )
					{
						g_CADCBaseB.SetSerialOpt(m_iChannelId, device_node->stopbyte, 1);
					}
					else if( device_node->checkMode[0] == 'E' )
					{
						g_CADCBaseB.SetSerialOpt(m_iChannelId, device_node->stopbyte, 2);
					}
					else
					{
						g_CADCBaseB.SetSerialOpt(m_iChannelId, device_node->stopbyte, 0);
					}

					if (nLen > g_CADCBaseB.SerialSend(m_iChannelId, reqBuf, nLen))
					{
						ret = SYS_STATUS_DEVICE_COMMUNICATION_ERROR;
						break;
					}

					nRespLen = g_CADCBaseB.SerialRecv(m_iChannelId, respBuf, MAX_PACKAGE_LEN_TERMINAL, device_node->timeOut);
					if (nRespLen > 0)
					{
						ret = SYS_STATUS_SUCCESS;
					}
					else
					{
						ret = SYS_STATUS_DEVICE_COMMUNICATION_ERROR;
					}

				} while (false);           
			}
			break;
		}
	} while (false);

	lightStatusSet(m_iChannelId, LIGHT_OFF);
	return ret;
}

//����ִ�к���
int CDeviceIfSpi485::ActionProc(PDEVICE_INFO_ACT device_node, ACTION_MSG& actionMsg)
{
	int ret = SYS_STATUS_FAILED;//�ύ�ɹ�
	BYTE reqBuf[MAX_PACKAGE_LEN_TERMINAL] = {0};
	BYTE respBuf[MAX_PACKAGE_LEN_TERMINAL] = {0};

	int nLen = 0;
	int nRespLen = 0;
	int baudRateEnum = 0;

	char szData[256] = {0};
	memcpy(szData, actionMsg.ActionValue, sizeof(actionMsg.ActionValue));

	
	//����
	lightStatusSet(m_iChannelId, LIGHT_ON);
	do 
	{
		if (0 >= strlen(device_node->W_Format))
		{
			ret = SYS_STATUS_ACTION_WFORMAT_ERROR;
			break;
		}
		//��֯����
		nLen = EncodeFormat(device_node->W_Format,device_node->Self_Id, NULL, szData, reqBuf, sizeof(reqBuf));
		if (0 >= nLen)
		{
			ret = SYS_STATUS_ACTION_ENCODE_ERROR;
			break;
		}

		switch(m_iChannelId)
		{
			//485�ӿ�A��
		case INTERFACE_RS485_A0:
		case INTERFACE_RS485_A1:
		case INTERFACE_RS485_A2:
			{
				baudRateEnum = CADCBase::f_getBaudRateEnum( device_node->BaudRate );
				g_CADCBaseA.SetBaudRate(m_iChannelId, (ADC_BAUDRATE)baudRateEnum);
				if( device_node->checkMode[0] == 'O' )
				{
					g_CADCBaseA.SetSerialOpt(m_iChannelId, device_node->stopbyte, 1);
				}
				else if( device_node->checkMode[0] == 'E' )
				{
					g_CADCBaseA.SetSerialOpt(m_iChannelId, device_node->stopbyte, 2);
				}
				else
				{
					g_CADCBaseA.SetSerialOpt(m_iChannelId, device_node->stopbyte, 0);
				}
				do
				{
					if (nLen > g_CADCBaseA.SerialSend(m_iChannelId, reqBuf, nLen))
					{
						ret = SYS_STATUS_DEVICE_COMMUNICATION_ERROR;
						break;
					}

					ret = SYS_STATUS_SUBMIT_SUCCESS;

					nRespLen = g_CADCBaseA.SerialRecv(m_iChannelId, respBuf, MAX_PACKAGE_LEN_TERMINAL, device_node->timeOut);

					if(strlen(device_node->WD_Format) <= 0)//���ݸ�ʽ����У�飬ֱ�ӷ����ύ�ɹ�
						break;
					ret = SYS_STATUS_DEVICE_COMMUNICATION_ERROR;
					if (0 < nRespLen && MAX_PACKAGE_LEN_TERMINAL > nRespLen)
					{
						ret = DecodeActionResponse(device_node, respBuf, nRespLen);
					}

				}while(false);
			}
			break;
		case INTERFACE_RS485_B0:
		case INTERFACE_RS485_B1:
		case INTERFACE_RS485_B2:
			{
				baudRateEnum = CADCBase::f_getBaudRateEnum( device_node->BaudRate );
				g_CADCBaseB.SetBaudRate(m_iChannelId, (ADC_BAUDRATE)baudRateEnum);
				if( device_node->checkMode[0] == 'O' )
				{
					g_CADCBaseB.SetSerialOpt(m_iChannelId, device_node->stopbyte, 1);
				}
				else if( device_node->checkMode[0] == 'E' )
				{
					g_CADCBaseB.SetSerialOpt(m_iChannelId, device_node->stopbyte, 2);
				}
				else
				{
					g_CADCBaseB.SetSerialOpt(m_iChannelId, device_node->stopbyte, 0);
				}
				do
				{
					if (nLen > g_CADCBaseB.SerialSend(m_iChannelId, reqBuf, nLen))
					{
						ret = SYS_STATUS_DEVICE_COMMUNICATION_ERROR;	
						break;
					}

					ret = SYS_STATUS_SUBMIT_SUCCESS;

					nRespLen = g_CADCBaseB.SerialRecv(m_iChannelId, respBuf, MAX_PACKAGE_LEN_TERMINAL, device_node->timeOut);

					if(strlen(device_node->WD_Format) <= 0)//���ݸ�ʽ����У�飬ֱ�ӷ����ύ�ɹ�
						break;
					ret = SYS_STATUS_DEVICE_COMMUNICATION_ERROR;
					if (0 < nRespLen && MAX_PACKAGE_LEN_TERMINAL > nRespLen)
					{
						ret = DecodeActionResponse(device_node, respBuf, nRespLen);
					}
				}while(false);
			}
			break;
		}
	} while (false);

	lightStatusSet(m_iChannelId, LIGHT_OFF);

	return ret;
}

/*************************************  END OF FILE *****************************************/
