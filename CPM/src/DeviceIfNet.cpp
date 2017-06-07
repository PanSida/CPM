/******************************************************************************/
/*    File    : DeviceIf485.cpp                                               */
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
#include "Init.h"
#include "SqlCtl.h"

#include "DeviceIfNet.h"
//#include "HardIF.h"
#include "LabelIF.h"



/******************************************************************************/
/*                       �ֲ��궨��                                           */
/******************************************************************************/

/******************************************************************************/
/*                       �ֲ�����                                             */
/******************************************************************************/

CDeviceIfNet::CDeviceIfNet()
{
	memset(m_szCurDeviceAttrId, 0, sizeof(m_szCurDeviceAttrId));
}

CDeviceIfNet::~CDeviceIfNet(void)
{		
	Terminate();
}

bool CDeviceIfNet::Terminate()
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
int CDeviceIfNet::Initialize(int iChannelId, TKU32 unHandleCtrl)
{
	int ret = SYS_STATUS_FAILED;
	if(SYS_STATUS_SUCCESS != (ret = CDeviceIfBase::Initialize(iChannelId, unHandleCtrl))) 
		return ret;

	ret = SYS_STATUS_SUCCESS;
	return ret; 
}

/******************************************************************************/
/* Function Name  : MainThrdProc									          */
/* Description    : �����豸��ѯ����                                          */
/* Input          : none                                                      */
/* Output         : none                                                      */
/* Return         : none                                                      */
/******************************************************************************/
void CDeviceIfBase::MainThrdProc(void)
{
	time_t tm_NowTime;
	struct tm *st_tmNowTime;

	struct timeval tmval_NowTime;
	QUEUEELEMENT pMsg = NULL;

	usleep(500*1000);
	m_MainThrdHandle = pthread_self();


	char buf[256] = {0};	
	sprintf(buf, "%s %d start....Pid[%d]\n", "CDeviceIfBase", m_iChannelId, getpid());
	DBG((buf));

	while(1)
	{
		pthread_testcancel();
		CLockQuery.lock();
		do 
		{
			//�鿴�Ƿ��ж���������Ҫִ��
			if( MMR_OK == g_MsgMgr.GetFirstMsg(m_unHandleCtrl, pMsg, 100))
			{
				ACTION_MSG actionMsg;
				PACTION_MSG pstActMsg = (PACTION_MSG)pMsg;
				memset(&actionMsg, 0, sizeof(ACTION_MSG));
				memcpy(&actionMsg,  pstActMsg, sizeof(ACTION_MSG));
				MM_FREE(pMsg);
				pMsg = NULL;
				DoAction(actionMsg);
				usleep( 10 * 1000 );
				break;			
			}		
			//��ȡ��ǰʱ��
			time(&tm_NowTime);    		
			gettimeofday(&tmval_NowTime, NULL);
			st_tmNowTime = localtime(&tm_NowTime);

			DEVICE_INFO_POLL deviceNode;
			if(SYS_STATUS_SUCCESS != g_DevInfoContainer.GetNextPollAttrNode(m_iChannelId, m_szCurDeviceAttrId, deviceNode))//��ȡ��Ѳ���
			{
				memset(m_szCurDeviceAttrId, 0, sizeof(m_szCurDeviceAttrId));
				break;
			}
			memcpy(m_szCurDeviceAttrId, deviceNode.ChannelNextId, DEVICE_ATTR_ID_LEN);
			m_szCurDeviceAttrId[DEVICE_ATTR_ID_LEN] = '\0';

			//�����豸���еĵ�ǰģʽ
			DEVICE_DETAIL_INFO stDeviceDetail;
			memset((unsigned char*)&stDeviceDetail, 0, sizeof(DEVICE_DETAIL_INFO));
			memcpy(stDeviceDetail.Id, deviceNode.Id, DEVICE_ID_LEN);
			if(SYS_STATUS_SUCCESS != g_DevInfoContainer.GetDeviceDetailNode(stDeviceDetail))
				break;			
			if(DEVICE_NOT_USE == stDeviceDetail.cStatus)//�豸�ѳ���
				break;

			PDEVICE_INFO_POLL pPollNode = &deviceNode;
			//�Ƿ���������ɼ�
			if ('0' == pPollNode->cBeOn)//����δ����
			{
				break;
			}

			//�Ƿ��ڲ���ʱ��
			if (false == CheckTime(st_tmNowTime, *pPollNode))
			{
				break;
			}

			//�ж��Ƿ�����ѯʱ��		
			if ((0 == pPollNode->Frequence 
				|| (unsigned long)((tmval_NowTime.tv_sec - pPollNode->stLastValueInfo.LastTime.tv_sec)*1000
				+ (tmval_NowTime.tv_usec - pPollNode->stLastValueInfo.LastTime.tv_usec)/1000) < pPollNode->Frequence)
				)//�����������ɼ�
			{
				break;
			}

			unsigned char respBuf[MAX_PACKAGE_LEN_TERMINAL] = {0};
			int respLen = 0;

			//����������
			if (DEVICE_ATTR_ID_LEN == strlen(pPollNode->V_Id))
			{
				bool bSendMsg = false;
				bool bDeviceStatus = false;
				QUERY_INFO_CPM stQueryResult;
				LAST_VALUE_INFO stLastValueInfo;
				memset((unsigned char*)&stLastValueInfo, 0, sizeof(LAST_VALUE_INFO));
				stQueryResult.Cmd = 0x00;
				strncpy(stQueryResult.DevId, pPollNode->Id, DEVICE_ATTR_ID_LEN);
				Time2Chars(&tm_NowTime, stQueryResult.Time);
				DBG(("����������[%s] ��Ӧ����[%s]\n", pPollNode->Id, pPollNode->V_Id));
				int iRet = g_DevInfoContainer.GetAttrValue(pPollNode->V_Id, stLastValueInfo);

				if (SYS_STATUS_SUCCESS == iRet &&  0 != strcmp(stLastValueInfo.Value, "NULL"))
				{
					memcpy(stQueryResult.Type, stLastValueInfo.Type, 4);
					stQueryResult.VType = stLastValueInfo.VType;
					memcpy(stQueryResult.Value, stLastValueInfo.Value, sizeof(stQueryResult.Value));
					bDeviceStatus = true;//�豸����

					stQueryResult.DataStatus = LabelIF_StandarCheck(stQueryResult.VType, stQueryResult.Value, pPollNode->szStandard)?0:1;

					//У�鲢�����ɹ�,����������
					memcpy(pPollNode->stLastValueInfo.Type, stQueryResult.Type, 4);
					pPollNode->stLastValueInfo.VType = stQueryResult.VType;
					memcpy(pPollNode->stLastValueInfo.Value, stQueryResult.Value, sizeof(pPollNode->stLastValueInfo.Value));
					memcpy(pPollNode->stLastValueInfo.szValueUseable, stQueryResult.Value, sizeof(pPollNode->stLastValueInfo.Value));

					pPollNode->stLastValueInfo.DataStatus = stQueryResult.DataStatus;
					bSendMsg = true;

				}
				else
				{
					pPollNode->stLastValueInfo.DataStatus = 1;

					pPollNode->stLastValueInfo.VType = CHARS;
					memset(pPollNode->stLastValueInfo.Value, 0, sizeof(pPollNode->stLastValueInfo.Value));
					memcpy(pPollNode->stLastValueInfo.Value, "NULL", 4);
				}

				//�豸״̬���¼��
				CheckOnLineStatus(pPollNode, bDeviceStatus, stQueryResult);

				if (bSendMsg)
				{
					if(!SendMsg_Ex(g_ulHandleAnalyser, MUCB_DEV_ANALYSE, (char*)&stQueryResult, sizeof(QUERY_INFO_CPM)))
					{
						DBG(("DeviceIfBase DecodeAndSendToAnalyser 1 SendMsg_Ex failed  handle[%d]\n", g_ulHandleAnalyser));
					}
					else
					{
						DBG(("E----->A   devid[%s] time[%s] value[%s] vtype[%d]\n", stQueryResult.DevId, stQueryResult.Time, stQueryResult.Value, stQueryResult.VType));
					}
				}
				memcpy((unsigned char*)(&(deviceNode.stLastValueInfo.LastTime)), (unsigned char*)(&tmval_NowTime), sizeof(struct timeval));
				g_DevInfoContainer.UpdateDeviceRoll(deviceNode);
				break;
			}

			//��ʵ������
			int iPollRet = this->PollProc(pPollNode, respBuf, respLen);
			if(SYS_STATUS_DEVICE_COMMUNICATION_ERROR == iPollRet && 3 > pPollNode->retryCount)
			{
				pPollNode->retryCount++;
				g_DevInfoContainer.UpdateDeviceRollRetryCount(deviceNode);
				break;
			}

			//��¼��ǰ���
			char curAttrId[DEVICE_ATTR_ID_LEN_TEMP] = {0};
			strcpy(curAttrId, pPollNode->Id);
			while(true) 
			{
				bool bSendMsg = false;
				QUERY_INFO_CPM_EX stQueryResultEx;
				QUERY_INFO_CPM stQueryResult;

				do 
				{//������Ѳ��Ӧ����
					bool bDeviceStatus = false;

					memset((unsigned char*)&stQueryResult, 0, sizeof(QUERY_INFO_CPM));
					Time2Chars(&tm_NowTime, stQueryResult.Time);
					stQueryResult.Cmd = 0x00;//��Ѳ��������
					memcpy(stQueryResult.DevId, pPollNode->Id,  DEVICE_ATTR_ID_LEN);

					memset((unsigned char*)&stQueryResultEx, 0, sizeof(QUERY_INFO_CPM_EX));
					Time2Chars(&tm_NowTime, stQueryResultEx.Time);
					stQueryResultEx.Cmd = 0x00;//��Ѳ��������
					memcpy(stQueryResultEx.DevId, pPollNode->Id,  DEVICE_ATTR_ID_LEN);

					//���ݸ�ʽУ��
					if( SYS_STATUS_SUCCESS == iPollRet && LabelIF_CheckRecvFormat(pPollNode->Rck_Format, respBuf, respLen))
					{
						bDeviceStatus = true;//�豸����
						int iDecRslt = LabelIF_DecodeByFormat(pPollNode, respBuf, respLen, &stQueryResultEx);
						if (SYS_STATUS_DATA_DECODE_NORMAL == iDecRslt)
						{	
							bSendMsg = true;
							//У�鲢�����ɹ�,����������
							memcpy((unsigned char*)&stQueryResult, (unsigned char*)&stQueryResultEx, sizeof(QUERY_INFO_CPM));
							memcpy(stQueryResult.Value, stQueryResultEx.Value, sizeof(stQueryResult.Value));
							stQueryResult.DataStatus = stQueryResultEx.DataStatus;

							memcpy(pPollNode->stLastValueInfo.Type, stQueryResult.Type, 4);
							pPollNode->stLastValueInfo.VType = stQueryResult.VType;
							if('1' == pPollNode->cIsNeedIdChange)//��Ҫ��ֵת��Ϊ�û�����
							{
								char szUserId[21] = {0};
								char szUserName[21] = {0};
								if(SYS_STATUS_SUCCESS != g_SqlCtl.GetUserInfoByCardId(stQueryResult.Value, szUserId, szUserName))
									break;
								memset(pPollNode->stLastValueInfo.Value, 0, sizeof(pPollNode->stLastValueInfo.Value));
								strcpy(pPollNode->stLastValueInfo.Value, szUserName);
							}
							else
							{
								memcpy(pPollNode->stLastValueInfo.Value, stQueryResult.Value, sizeof(pPollNode->stLastValueInfo.Value));
								memcpy(pPollNode->stLastValueInfo.szValueUseable, stQueryResult.Value, sizeof(pPollNode->stLastValueInfo.Value));
							}

							stQueryResult.DataStatus = LabelIF_StandarCheck(stQueryResult.VType, stQueryResult.Value, pPollNode->szStandard)?0:1;
							pPollNode->stLastValueInfo.DataStatus = stQueryResult.DataStatus;
						}
						else
						{
							pPollNode->offLineCnt --;
							if (SYS_STATUS_DATA_DECODE_UNNOMAL_AGENT == iDecRslt)
							{
								bSendMsg = true;
							}
						}
					}
					else
					{
						pPollNode->stLastValueInfo.DataStatus = 1;

						pPollNode->stLastValueInfo.VType = CHARS;
						memset(pPollNode->stLastValueInfo.Value, 0, sizeof(pPollNode->stLastValueInfo.Value));
						memcpy(pPollNode->stLastValueInfo.Value, "NULL", 4);
					}

					//�豸״̬���¼��
					if (INTERFACE_DIN_A0 > atoi(pPollNode->Upper_Channel) || INTERFACE_DIN_B3 < atoi(pPollNode->Upper_Channel))//���������⴦��
					{
						CheckOnLineStatus(pPollNode, bDeviceStatus, stQueryResult);
					}
					if (bSendMsg)
					{
						//�к�������ҵ��
						if(0 == strncmp(stQueryResult.DevId, DEV_TYPE_CNOOC_DEAL, 3)
							&& 0 == strncmp(stQueryResult.DevId + DEVICE_ID_LEN, DEV_TYPE_CNOOC_DEAL_ATTID, DEVICE_SN_LEN))
						{
							SendMsg_Ex(g_ulHandleCnoocDeal, MUCB_DEV_CNOOC_DEAL, (char*)&stQueryResultEx, sizeof(QUERY_INFO_CPM_EX));
						}
						//��е�ֹ�
						else if(0 == strncmp(stQueryResult.DevId, DEV_TYPE_READER, DEVICE_TYPE_LEN)
							&& 0 == strncmp(stQueryResult.DevId + DEVICE_ID_LEN, DEV_TYPE_READER_ATTID, DEVICE_SN_LEN))
						{
							SCH_CARD_ID stId;
							memset((unsigned char*)&stId, 0, sizeof(SCH_CARD_ID));
							memcpy(stId.Id, stQueryResultEx.Value, MAX_PACKAGE_LEN_TERMINAL);
							memcpy(stId.DevId, stQueryResultEx.DevId, DEVICE_ATTR_ID_LEN);
							SendMsg_Ex(g_ulHandleSchReader, MUCB_DEV_SCH, (char*)&stId, sizeof(SCH_CARD_ID));
						}
						else if(!SendMsg_Ex(g_ulHandleAnalyser, MUCB_DEV_ANALYSE, (char*)&stQueryResult, sizeof(QUERY_INFO_CPM)))
						{
							DBG(("DeviceIfBase DecodeAndSendToAnalyser 2 SendMsg_Ex failed  handle[%d]\n", g_ulHandleAnalyser));
						}
						else
						{
							DBG(("E----->A   devid[%s] time[%s] value[%s] vtype[%d]\n", stQueryResult.DevId, stQueryResult.Time, stQueryResult.Value, stQueryResult.VType));
						}
					}
				} while (false);

				//���½����Ϣ,�����������ߴ���,��Ѳʱ��
				memcpy((unsigned char*)(&(deviceNode.stLastValueInfo.LastTime)), (unsigned char*)(&tmval_NowTime), sizeof(struct timeval));
				g_DevInfoContainer.UpdateDeviceRoll(deviceNode);

				if(0 == strlen(pPollNode->NextId) || 0 == strncmp(curAttrId, pPollNode->NextId, DEVICE_ID_LEN + DEVICE_SN_LEN))//��ȡ��ͬrd���õ�����
					break;

				DBG(("pPollNode->NextId[%s] deviceNode.id[%s]\n", pPollNode->NextId, deviceNode.Id));
				if (SYS_STATUS_SUCCESS != g_DevInfoContainer.GetNextAttrNode(curAttrId, pPollNode->NextId, deviceNode))
				{
					DBG(("GetNextAttrNode[%s][%s]failed\n", pPollNode->Id, pPollNode->NextId))
						break;
				}

				pPollNode = &deviceNode;
			}
		} while (false);
		CLockQuery.unlock();
		usleep( 10 * 1000 );
	}
}

/******************************************************************************/
/* Function Name  : QueryAttrProc                                           */
/* Description    : �豸ʵʱ��ѯ                                          */
/* Input          : none                                                      */
/* Output         : none                                                      */
/* Return         : none                                                      */
/******************************************************************************/
void CDeviceIfBase::QueryAttrProc(DEVICE_INFO_POLL& deviceNode)
{
	QUERY_INFO_CPM stQueryResult;

	QUERY_INFO_CPM_EX stQueryResultEx;
	PDEVICE_INFO_POLL pPollNode = &deviceNode;
	bool bDeviceStatus = false;
	bool bSendMsg = false;

	memset((unsigned char*)&stQueryResult, 0, sizeof(QUERY_INFO_CPM));
	stQueryResult.Cmd = 0x00;
	memcpy(stQueryResult.DevId, pPollNode->Id,  DEVICE_ATTR_ID_LEN);

	memset((unsigned char*)&stQueryResultEx, 0, sizeof(QUERY_INFO_CPM_EX));
	stQueryResultEx.Cmd = 0x00;
	memcpy(stQueryResultEx.DevId, pPollNode->Id,  DEVICE_ATTR_ID_LEN);

	//��ȡ��ǰʱ��  	
	time_t tm_NowTime;
	time(&tm_NowTime);  
	Time2Chars(&tm_NowTime, stQueryResult.Time);
	Time2Chars(&tm_NowTime, stQueryResultEx.Time);

	unsigned char respBuf[MAX_PACKAGE_LEN_TERMINAL] = {0};
	int respLen = 0;

	struct timeval tmval_NowTime;	
	gettimeofday(&tmval_NowTime, NULL);

	CLockQuery.lock();

	DBG(("QueryAttrProc ------------Start--------------- atrId[%s]\n", deviceNode.Id));
	//����������
	if (DEVICE_ATTR_ID_LEN == strlen(pPollNode->V_Id))
	{
		LAST_VALUE_INFO stLastValue;
		memset((unsigned char*)&stLastValue, 0, sizeof(LAST_VALUE_INFO));

		int iRet = g_DevInfoContainer.GetAttrValue(pPollNode->V_Id, stLastValue);

		if (SYS_STATUS_SUCCESS == iRet &&  0 != strcmp(stLastValue.Value, "NULL"))
		{
			strncpy(stQueryResult.Type, stLastValue.Type, 4);
			stQueryResult.VType = stLastValue.VType;
			memcpy(stQueryResult.Value, stLastValue.Value, sizeof(stQueryResult.Value));
			bDeviceStatus = true;//�豸����

			stQueryResult.DataStatus = LabelIF_StandarCheck(stQueryResult.VType, stQueryResult.Value, pPollNode->szStandard)?0:1;

			//У�鲢�����ɹ�,����������
			memcpy(pPollNode->stLastValueInfo.Type, stQueryResult.Type, 4);
			pPollNode->stLastValueInfo.VType = stQueryResult.VType;
			memcpy(pPollNode->stLastValueInfo.Value, stQueryResult.Value, sizeof(pPollNode->stLastValueInfo.Value));
			pPollNode->stLastValueInfo.DataStatus = stQueryResult.DataStatus;		
		}
		else
		{
			pPollNode->stLastValueInfo.DataStatus = 1;

			pPollNode->stLastValueInfo.VType = CHARS;
			memset(pPollNode->stLastValueInfo.Value, 0, sizeof(pPollNode->stLastValueInfo.Value));
			memcpy(pPollNode->stLastValueInfo.Value, "NULL", 4);
		}

		//�豸״̬���¼��
		CheckOnLineStatus(pPollNode, bDeviceStatus, stQueryResult);
	}
	else//��ʵ������
	{		
		respLen = sizeof(respBuf);
		this->PollProc(pPollNode, respBuf, respLen);

		//���ݸ�ʽУ��
		if( LabelIF_CheckRecvFormat(pPollNode->Rck_Format, respBuf, respLen))
		{
			bDeviceStatus = true;//�豸����
			int iDecRslt = LabelIF_DecodeByFormat(pPollNode, respBuf, respLen, &stQueryResultEx);
			if (SYS_STATUS_DATA_DECODE_NORMAL == iDecRslt)
			{	
				//У�鲢�����ɹ�,����������
				memcpy((unsigned char*)&stQueryResult, (unsigned char*)&stQueryResultEx, sizeof(QUERY_INFO_CPM));
				memcpy(stQueryResult.Value, stQueryResultEx.Value, sizeof(stQueryResult.Value));
				stQueryResult.DataStatus = stQueryResultEx.DataStatus;

				//У�鲢�����ɹ�,����������
				memcpy(pPollNode->stLastValueInfo.Type, stQueryResult.Type, 4);
				pPollNode->stLastValueInfo.VType = stQueryResult.VType;
				memcpy(pPollNode->stLastValueInfo.Value, stQueryResult.Value, sizeof(pPollNode->stLastValueInfo.Value));

				stQueryResult.DataStatus = LabelIF_StandarCheck(stQueryResult.VType, stQueryResult.Value, pPollNode->szStandard)?0:1;
				pPollNode->stLastValueInfo.DataStatus = stQueryResult.DataStatus;
			}
			else
			{
				pPollNode->offLineCnt --;
				if (SYS_STATUS_DATA_DECODE_UNNOMAL_AGENT == iDecRslt)
				{
					bSendMsg = true;
				}
			}
		}
		else
		{
			pPollNode->stLastValueInfo.DataStatus = 1;			
			pPollNode->stLastValueInfo.VType = CHARS;
			memset(pPollNode->stLastValueInfo.Value, 0, sizeof(pPollNode->stLastValueInfo.Value));
			memcpy(pPollNode->stLastValueInfo.Value, "NULL", 4);
		}

		//�豸״̬���¼��
		if (INTERFACE_AD_A0 >= atoi(pPollNode->Upper_Channel) || INTERFACE_DIN_B3 <= atoi(pPollNode->Upper_Channel))//���������⴦��
		{
			CheckOnLineStatus(pPollNode, bDeviceStatus, stQueryResult);
		}
	}

	if (bSendMsg)
	{
		if(!SendMsg_Ex(g_ulHandleAnalyser, MUCB_DEV_ANALYSE, (char*)&stQueryResult, sizeof(QUERY_INFO_CPM)))
		{
			DBG(("DeviceIfBase DecodeAndSendToAnalyser 3 SendMsg_Ex failed  handle[%d]\n", g_ulHandleAnalyser));
		}
		else
		{
			DBG(("E----->A   devid[%s] time[%s] value[%s] vtype[%d]\n", stQueryResult.DevId, stQueryResult.Time, stQueryResult.Value, stQueryResult.VType));
		}
	}
	memcpy((unsigned char*)(&(deviceNode.stLastValueInfo.LastTime)), (unsigned char*)(&tmval_NowTime), sizeof(struct timeval));
	g_DevInfoContainer.UpdateDeviceRoll(deviceNode);

	DBG(("QueryAttrProc ------------End--------------- atrId[%s]\n", deviceNode.Id));
	CLockQuery.unlock();
}




/******************************************************************************/
/* Function Name  : PollProc                                                  */
/* Description    : ��Ѳִ�к���		                                      */
/* Input          : PDEVICE_INFO_POLL device_node	            ��Ѳ���      */
/* Output         : none													  */
/* Return         : int ret          0:�ɹ���1:�豸����Ӧ                     */
/******************************************************************************/

int CDeviceIfNet::PollProc(PDEVICE_INFO_POLL device_node, unsigned char* respBuf, int& nRespLen)
{
	int ret = SYS_STATUS_FAILED;
	BYTE reqBuf[MAX_PACKAGE_LEN_TERMINAL] = {0};
	int nLen = 0;

	//int iSendLen = 0;
	//int iNeedLen = nRespLen;
	nRespLen = 0;

	//����
	HardIF_BackLightSet(m_iChannelId, 1);
	do 
	{
		if (0 >= strlen(device_node->R_Format))
			break;

		//��֯����
		nLen = LabelIF_EncodeFormat(device_node->R_Format,device_node->Self_Id, NULL, NULL, reqBuf, sizeof(reqBuf));
		if (0 >= nLen)
			break;

		CHardIF_485* hardIF_485 = new CHardIF_485(m_iChannelId, device_node->BaudRate, device_node->databyte, device_node->checkMode[0], device_node->stopbyte, device_node->timeOut, 0);
		ret = hardIF_485->Exec485(respBuf, nRespLen, reqBuf, nLen);
		delete hardIF_485;
		if (0 < nRespLen && MAX_PACKAGE_LEN_TERMINAL > nRespLen)
		{
			ret = SYS_STATUS_SUCCESS;
		}
		else
		{
			ret = SYS_STATUS_DEVICE_COMMUNICATION_ERROR;
		}
	} while (false);

	HardIF_BackLightSet(m_iChannelId, 0);
	return ret;
}

//����ִ�к���
int CDeviceIfNet::ActionProc(PDEVICE_INFO_ACT device_node, ACTION_MSG& actionMsg)
{
	int ret = SYS_STATUS_FAILED;//�ύ�ɹ�
	BYTE reqBuf[MAX_PACKAGE_LEN_TERMINAL] = {0};
	BYTE respBuf[MAX_PACKAGE_LEN_TERMINAL] = {0};

	int nLen = 0;
	int nRespLen = 0;
	//	int iSendLen = 0;
	int channelid = atoi(device_node->Upper_Channel);

	char szData[256] = {0};
	memcpy(szData, actionMsg.ActionValue, sizeof(szData));
	DBG(("ActionProc channelid:[%d] ID[%s] szData[%s]\n", channelid, device_node->Id, szData));


	//����
	HardIF_BackLightSet(channelid, 1);
	do 
	{
		if (0 >= strlen(device_node->W_Format))
		{
			ret = SYS_STATUS_ACTION_WFORMAT_ERROR;
			break;
		}
		//��֯����
		nLen = LabelIF_EncodeFormat(device_node->W_Format,device_node->Self_Id, NULL, szData, reqBuf, sizeof(reqBuf));
		if (0 >= nLen)
		{
			ret = SYS_STATUS_ACTION_ENCODE_ERROR;
			break;
		}

		CHardIF_485* hardIF_485 = new CHardIF_485(channelid, device_node->BaudRate, device_node->databyte, device_node->checkMode[0], device_node->stopbyte, device_node->timeOut, 1);
		ret = hardIF_485->Exec485(respBuf, nRespLen, reqBuf, nLen);
		delete hardIF_485;

		if(strlen(device_node->WD_Format) <= 0)//���ݸ�ʽ����У�飬ֱ�ӷ����ύ�ɹ�
			break;

		if (0 < nRespLen && MAX_PACKAGE_LEN_TERMINAL > nRespLen)
		{
			ret = LabelIF_DecodeActionResponse(device_node, respBuf, nRespLen);
		}


	} while (false);


	HardIF_BackLightSet(channelid, 0);

	return ret;
}
/*************************************  END OF FILE *****************************************/
