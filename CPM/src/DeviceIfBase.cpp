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
#include <stdio.h>
#include <error.h>
#include "Shared.h"
#include "Init.h"
#include "SqlCtl.h"
#include "LabelIF.h"
#include "SOIF.h"
#include "DeviceIfBase.h"



#ifdef  DEBUG
#define DEBUG_IF
#endif

#ifdef DEBUG_IF
#define DBG_IF(a)		printf a;
#else
#define DBG_IF(a)	
#endif

/******************************************************************************/
/*                       �ֲ��궨��                                           */
/******************************************************************************/

/******************************************************************************/
/*                       �ֲ�����                                             */
/******************************************************************************/

CDeviceIfBase::CDeviceIfBase()
{
	memset(m_szCurDeviceAttrId, 0, sizeof(m_szCurDeviceAttrId));
}

CDeviceIfBase::~CDeviceIfBase(void)
{		
	Terminate();
}

bool CDeviceIfBase::Terminate()
{
	TRACE(CDeviceIfBase::Terminate());

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
int CDeviceIfBase::Initialize(int iChannelId, TKU32 unHandleCtrl)
{
	//ͨ����
	m_iChannelId = iChannelId;

	//���������ڴ���
	m_unHandleCtrl	= unHandleCtrl;
	
	//�豸��ѯ�߳�
	char buf[256] = {0};	
	sprintf(buf, "%s %d %s", "CDeviceIfBase", iChannelId,  "CDeviceIfBaseThrd");
	if (!ThreadPool::Instance()->CreateThread(buf, &CDeviceIfBase::MainThrd, true,this))
	{
		DBG(("pCommThrdCtrl Create failed!\n"));
		return SYS_STATUS_CREATE_THREAD_FAILED;
	}
	return SYS_STATUS_SUCCESS; 
}

//�����豸��ѯ�߳�
void * CDeviceIfBase::MainThrd(void *arg)
{
	CDeviceIfBase *_this = (CDeviceIfBase *)arg;
	_this->MainThrdProc();
	return NULL;
}

/******************************************************************************/
/* Function Name  : CommMsgCtrlFunc                                           */
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
			/*********************************************************************************************/
			//��ʵ������
			int iPollRet = 0;
			if (1 == pPollNode->isSOIF)
			{
				iPollRet = SOIF_PollProc(this->m_iChannelId, pPollNode, respBuf, respLen);
			}
			else
			{
				iPollRet = this->PollProc(pPollNode, respBuf, respLen);				
			}
			if(SYS_STATUS_DEVICE_COMMUNICATION_ERROR == iPollRet && 3 > pPollNode->retryCount)
			{
				pPollNode->retryCount++;
				g_DevInfoContainer.UpdateDeviceRollRetryCount(deviceNode);
				break;
			}
			/*********************************************************************************************/
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
						int iDecRslt = LabelIF_DecodeByFormat(pPollNode, respBuf, respLen, &stQueryResultEx);
						if (SYS_STATUS_DATA_DECODE_NORMAL == iDecRslt)
						{	
							
							bSendMsg = true;
							bDeviceStatus = true;//�豸����
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
						//͸������(�����к����豸����ж�)
						if((1 == pPollNode->isTransparent)
							|| (0 == strncmp(stQueryResult.DevId, DEV_TYPE_CNOOC_DEAL, 3)
							&& 0 == strncmp(stQueryResult.DevId + DEVICE_ID_LEN, DEV_TYPE_CNOOC_DEAL_ATTID, DEVICE_SN_LEN)))
						{
							SendMsg_Ex(g_ulHandleTransparent, MUCB_DEV_TRANSPARENT, (char*)&stQueryResultEx, sizeof(QUERY_INFO_CPM_EX));
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
			int iDecRslt = LabelIF_DecodeByFormat(pPollNode, respBuf, respLen, &stQueryResultEx);
			if (SYS_STATUS_DATA_DECODE_NORMAL == iDecRslt)
			{	
				bSendMsg = true;
				bDeviceStatus = true;//�豸����
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

bool CDeviceIfBase::CheckTime(struct tm *st_tmNowTime, DEVICE_INFO_POLL device_node)
{
    bool ret = false;
    time_t tm_now = 0;
    tm_now = mktime(st_tmNowTime);

    if( (device_node.startTime.tm_hour < device_node.endTime.tm_hour)         //��ʼ������ʱ����ͬһ��
        || ( (device_node.startTime.tm_hour==device_node.endTime.tm_hour) && (device_node.startTime.tm_min<device_node.endTime.tm_min) ) )
    {
        if( st_tmNowTime->tm_hour > device_node.startTime.tm_hour )            //
        {
            if( st_tmNowTime->tm_hour < device_node.endTime.tm_hour )
            {
                ret = true;
            }
            else if( (st_tmNowTime->tm_hour == device_node.endTime.tm_hour) && (st_tmNowTime->tm_min <= device_node.endTime.tm_hour))
            {
                ret =true;
            }
        }
        else if( st_tmNowTime->tm_hour == device_node.startTime.tm_hour )
        {
            if( st_tmNowTime->tm_min >= device_node.startTime.tm_min )
            {
                if( st_tmNowTime->tm_hour < device_node.endTime.tm_hour )
                {
                    ret = true;
                }
                else if( (st_tmNowTime->tm_hour == device_node.endTime.tm_hour) && (st_tmNowTime->tm_min <= device_node.endTime.tm_min) )
                {
                    ret = true;
                }
            }
        }
    }
    else                      //��ʼ������ʱ�����
    {
        if( (st_tmNowTime->tm_hour > device_node.startTime.tm_hour) || (st_tmNowTime->tm_hour < device_node.endTime.tm_hour) )
        {
            ret = true;
        }
        else if( ( st_tmNowTime->tm_hour == device_node.startTime.tm_hour) && ( st_tmNowTime->tm_min >= device_node.startTime.tm_min) )
        {
            ret = true;
        }
        else if( ( st_tmNowTime->tm_hour == device_node.endTime.tm_hour) && ( st_tmNowTime->tm_min <= device_node.endTime.tm_min) )
        {
            ret = true;
        }
    }

    return ret;
}


void CDeviceIfBase::DoAction(ACTION_MSG actionMsg)
{
	//���Ҷ�Ӧ�����豸
	char szActionId[DEVICE_ACT_ID_LEN_TEMP] = {0};
	DEVICE_INFO_ACT devNode;
	memset((unsigned char*)&devNode, 0, sizeof(DEVICE_INFO_ACT));
	memcpy(szActionId, actionMsg.DstId, DEVICE_ID_LEN);
	
	memcpy(szActionId + DEVICE_ID_LEN, actionMsg.ActionSn, DEVICE_ACTION_SN_LEN);
	int actionRet = SYS_STATUS_FAILED;

	DBG((" CDeviceIfBase DoAction .DeviceId[%s].\n", szActionId));
	do
	{
		if (0 == strcmp(ACTION_SET_DEFENCE, actionMsg.ActionSn) || 0 == strcmp(ACTION_REMOVE_DEFENCE, actionMsg.ActionSn))//����������
		{
			int status = 0;
			if(0 == strcmp(ACTION_SET_DEFENCE, actionMsg.ActionSn))
			{
				status = SYS_ACTION_SET_DEFENCE;
			}
			else
			{
				status = SYS_ACTION_REMOVE_DEFENCE;
			}
			if(g_DevInfoContainer.HashDeviceDefenceSet(actionMsg.DstId, status))
				actionRet = SYS_STATUS_SUCCESS;
		}
		else
		{
			DEVICE_DETAIL_INFO stDeviceDetail;
			memset((unsigned char*)&stDeviceDetail, 0, sizeof(DEVICE_DETAIL_INFO));
			memcpy(stDeviceDetail.Id, actionMsg.DstId, DEVICE_ID_LEN);
			if(SYS_STATUS_SUCCESS != g_DevInfoContainer.GetDeviceDetailNode(stDeviceDetail))//��ȡ�豸
			{
				actionRet = SYS_STATUS_DEVICE_NOT_EXIST;
				break;
			}
			if(DEVICE_NOT_USE == stDeviceDetail.cStatus)//�豸�ѳ���
			{
				actionRet = SYS_STATUS_DEVICE_REMOVE_DEFENCE;
				break;
			}

			if(SYS_STATUS_SUCCESS != g_DevInfoContainer.GetActionNode(szActionId, devNode))//��ȡ�豸����
			{
				DBG(("��ȡ�豸��������ʧ��[%s]\n", szActionId));
				actionRet = SYS_STATUS_DEVICE_ACTION_NOT_EXIST;
				break;
			}
			if ('0' == devNode.cBeOn)//����δ����
			{
				DBG(("���豸����δ����[%s]\n", szActionId));
				actionRet = SYS_STATUS_DEVICE_ACTION_NOT_USE;
				break;
			}
			int actionCount = 3;
			actionRet = SYS_STATUS_DEVICE_COMMUNICATION_ERROR;
			while(SYS_STATUS_DEVICE_COMMUNICATION_ERROR == actionRet && actionCount-- > 0)
				actionRet = ActionProc(&devNode, actionMsg);
		}
	}while(false);

	CDevInfoContainer::SendToActionSource(actionMsg, actionRet);
}


/******************************************************************************/
/* Function Name  : CheckOnLineStatus                                            */
/* Description    : ���������ж�		                                      */
/* Input          : DEVICE_INFO_POLL *p_node	        ��Ѳ���      */
/*		          : int nLen							���ݳ���      */
/* Output         : none                   */
/* Return         : int ret          0:�ޱ仯��1:���������ܲɼ��� 2:���Բɼ����ɹ�                        */
/******************************************************************************/
int CDeviceIfBase::CheckOnLineStatus(DEVICE_INFO_POLL *p_node, bool bStatus, QUERY_INFO_CPM stQueryResult)
{
	int ret = SYS_STATUS_SUCCESS;
	QUERY_INFO_CPM stOnlinNotice;

	memcpy((unsigned char*)&stOnlinNotice, (unsigned char*)&stQueryResult, sizeof(QUERY_INFO_CPM));

	bool isAlreadyOffLine = false;
    if( bStatus)        //���յ�������
    {
        if(0 == p_node->offLineCnt)        //
        {
           ret = SYS_STATUS_DEVICE_ATTR_ONLINE;
		}
		p_node->offLineCnt = p_node->offLineTotle;
    }
    else
    {
		if (0 == p_node->offLineCnt)//�豸����������
		{
			isAlreadyOffLine = true;
			ret = SYS_STATUS_DEVICE_ATTR_OFFLINE;
			stOnlinNotice.Cmd = 0;
			memset(stOnlinNotice.Value, 0, sizeof(stOnlinNotice.Value));
			strcpy(stOnlinNotice.Value, "NULL");
			stOnlinNotice.VType = CHARS;
			stOnlinNotice.DataStatus = 1;
			DBG(("[%s] ��������1\n", stOnlinNotice.DevId));
			if(!SendMsg_Ex(g_ulHandleAnalyser, MUCB_DEV_ANALYSE, (char*)&stOnlinNotice, sizeof(QUERY_INFO_CPM)))
			{
				DBG(("DeviceIfBase DecodeAndSendToAnalyser 4 SendMsg_Ex failed  handle[%d]\n", g_ulHandleAnalyser));
			}
		}
        else if(0 < p_node->offLineCnt && 0 == --p_node->offLineCnt)    //�豸��������
        {
            ret = SYS_STATUS_DEVICE_ATTR_OFFLINE;
			stOnlinNotice.Cmd = 0;
			memset(stOnlinNotice.Value, 0, sizeof(stOnlinNotice.Value));
			strcpy(stOnlinNotice.Value, "NULL");
			stOnlinNotice.VType = CHARS;
			stOnlinNotice.DataStatus = 1;
			DBG(("[%s] ��������2\n", stOnlinNotice.DevId));
			if(!SendMsg_Ex(g_ulHandleAnalyser, MUCB_DEV_ANALYSE, (char*)&stOnlinNotice, sizeof(QUERY_INFO_CPM)))
			{
				DBG(("DeviceIfBase DecodeAndSendToAnalyser 5 SendMsg_Ex failed  handle[%d]\n", g_ulHandleAnalyser));
			}
        }
		DBG(("[%s] ���������� p_node->offLineCnt[%d]\n", stOnlinNotice.DevId, p_node->offLineCnt));
    }

	bool bSendMsg = false;
	switch(ret)
	{
		case SYS_STATUS_DEVICE_ATTR_OFFLINE:
			{
				//�ж��Ƿ��豸����lhy
				if (!g_DevInfoContainer.IsDeviceOnline(p_node->Id))
				{
					sprintf(stOnlinNotice.Value, "%d", DEVICE_OFFLINE);

					if (isAlreadyOffLine)
						stOnlinNotice.Cmd = 3;
					else
						stOnlinNotice.Cmd = 1;
					stOnlinNotice.DevId[DEVICE_ID_LEN] = '\0';
					strncpy(stOnlinNotice.DevId + DEVICE_ID_LEN, "0000", DEVICE_SN_LEN);
					memset(stOnlinNotice.Value, 0, sizeof(stOnlinNotice.Value));
					sprintf(stOnlinNotice.Value, "%d", DEVICE_OFFLINE);
					bSendMsg = true;
				}
			}
			break;
		case SYS_STATUS_DEVICE_ATTR_ONLINE:
			//�ж��Ƿ������豸����lhy
			{
				if (!g_DevInfoContainer.IsDeviceOnline(p_node->Id))
				{
					stOnlinNotice.Cmd = 1;
					stOnlinNotice.DevId[DEVICE_ID_LEN] = '\0';
					
					memset(stOnlinNotice.Value, 0, sizeof(stOnlinNotice.Value));
					sprintf(stOnlinNotice.Value, "%d", DEVICE_ONLINE);
					bSendMsg = true;
				}
			}
			break;
	}

	if(bSendMsg)
	{
		if(!SendMsg_Ex(g_ulHandleAnalyser, MUCB_DEV_ANALYSE, (char*)&stOnlinNotice, sizeof(QUERY_INFO_CPM)))
		{
			DBG(("DeviceIfBase DecodeAndSendToAnalyser 6 SendMsg_Ex failed  handle[%d]\n", g_ulHandleAnalyser));
		}
		else
		{
			DBG(("E----->A Cmd[%d]  devid[%s] time[%s] value[%s] vtype[%d]\n", stOnlinNotice.Cmd, stOnlinNotice.DevId, stOnlinNotice.Time, stOnlinNotice.Value, stOnlinNotice.VType));
		}
	}
	return ret;
}

/*************************************  END OF FILE *****************************************/
