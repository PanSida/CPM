#include <string>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include "Shared.h"
#include "Init.h"
#include "Define.h"
#include "SqlCtl.h"
#include "Analyser.h"
#include "HardIF.h"

#ifdef  DEBUG
#define DEBUG_ANA
#endif
#ifdef DEBUG_ANA
#define DBG_ANA(a)		printf a;
#else
#define DBG_ANA(a)	
#endif


CAnalyser::CAnalyser()
{
}

CAnalyser::~CAnalyser()
{
}

bool CAnalyser::Initialize(TKU32 unHandleAnalyser)
{
    m_unHandleAnalyser = unHandleAnalyser;

    //-------------------------�����ϣ��---------------------------
    m_pActHashTable = new HashedInfo();
    if (!m_pActHashTable)
	{
		return false;
	}
	
    m_pAgentHashTable = new HashedAgentInfo();
	if (!m_pAgentHashTable)
	{
		return false;
	}	

    if (!InitAnalyseHash())           //������ϣ���ʼ��
	{
		return false;
	}

    //���ݴ����߳�
    char buf[256] = {0};
	sprintf(buf, "%s %s", "CAnalyser", "AnalyserThrd");
	if (!ThreadPool::Instance()->CreateThread(buf, &CAnalyser::pAnalyserThrd, true,this))
	{
		DBG(("pAnalyserThrd Create failed!\n"));
		return false;
	}

	DBG_ANA(("CAnalyser::Initialize success[%d]!\n ", unHandleAnalyser));
	return true; 
}

/******************************************************************************/
/* Function Name  : InitAnalyseHash                                           */
/* Description    : ��ϣ�����ݳ�ʼ��                                          */
/* Input          : none                                                      */
/* Output         : none                                                      */
/* Return         : none                                                      */
/******************************************************************************/
bool CAnalyser::InitAnalyseHash()
{
	DBG(("Now try to init actHash--------------->\n"));

	const char* format = "select device_roll.id,"
					" device_roll.sn,"
					" device_detail.status,"
					" device_roll.standard,"
					" device_attr.s_define "
						" from device_roll, "
						" device_detail,"
						" device_attr"
						" where device_roll.id = device_detail.id "
						" and device_roll.sn = device_attr.sn"
						" and substr(device_detail.id,1,%d) = device_attr.id "
						" group by device_roll.id, device_roll.sn;";
	char sqlData[1024] = {0};
	sprintf(sqlData, format, DEVICE_TYPE_LEN);
	if (!InitActHashData(sqlData))
	{
		DBG(("InitActHashData InitActHashData Failed ....\n"));
		return false;
	}
	//lhy ��ʼ����������
	format = "select SEQ,"
					" ID,"
					" SN,"
					" Condition,"
					" Valibtime,"
					" Interval,"
					" Times,"
					" D_ID,"
					" D_CMD,"
					" OBJECT,"
					" CTYPE"
					" from device_agent;";
	if (!InitAgentInfo(format))
	{
		DBG(("InitAgentInfo InitActHashData Failed ....\n"));
		return false;
	}
	return true;

}

/******************************************************************************/
/* Function Name  : InitActHashInfo                                           */
/* Description    : ��ϣ�����ݳ�ʼ��                                          */
/* Input          : none                                                      */
/* Output         : none                                                      */
/* Return         : none                                                      */
/******************************************************************************/
bool CAnalyser::InitActHashData(char* sqlData)
{
	if( 0 != g_SqlCtl.select_from_tableEx(sqlData, InitActHashCallback, (void *)this))
		return false;
	return true;
}

int CAnalyser::InitActHashCallback(void *data, int n_column, char **column_value, char **column_name)    //��ѯʱ�Ļص�����
{
	CAnalyser* _this = (CAnalyser*)data;
	_this->InitActHashCallbackProc(n_column, column_value, column_name);	
	return 0;
}

int CAnalyser::InitActHashCallbackProc(int n_column, char **column_value, char **column_name)
{
	int ret = -1;
	MSG_ANALYSE_HASH hash_node;

	memset(&hash_node, 0, sizeof(MSG_ANALYSE_HASH));

	memcpy(hash_node.key, column_value[0], strlen(column_value[0]));  
	memcpy(hash_node.key+DEVICE_ID_LEN, column_value[1], strlen(column_value[1]));  //��ָ��sn��

	//	DBG_ANA(("c0[%s] c1[%s] c2[%s] c3[%s] c4[%s] c5[%s] c6[%s] c7[%s]\n", column_value[0], column_value[1], column_value[2], column_value[3], column_value[4], column_value[5], column_value[6], column_value[7]));
	//DBG_ANA(("nodeId:[%s]  ", hash_node.key));
	hash_node.deviceStatus = atoi(column_value[2]);                           //��ȡ�豸״̬

	memcpy(hash_node.standard, column_value[3], strlen(column_value[3]));     //��ȡ��׼ֵ��Χ

	strcpy(hash_node.statusDef, trim(column_value[4], strlen(column_value[4])));
	//  DBG_ANA(("statusDef:[%s]\n", hash_node.statusDef));
	hash_node.actCnt = 0;
	memset((unsigned char*)&hash_node.agentSeq[0], 0, sizeof(int) * MAX_CNT_ACTFORMAT);
	memset((unsigned char*)(&hash_node.LastTime), 0, sizeof(struct timeval));

	hash_node.dataStatus = 1;
	CTblLock.lock();
 	m_pActHashTable->insert(HashedInfo::value_type(hash_node.key,hash_node));
	CTblLock.unlock();

	// DBG_ANA(("InitActHashCallbackProc end\n"));
	return ret;
}

bool CAnalyser::InitAgentInfo(const char* sqlData)
{
	if( 0 != g_SqlCtl.select_from_tableEx((char*)sqlData, InitAgentInfoCallback, (void *)this))
		return false;
	return true;
}
int CAnalyser::InitAgentInfoCallback(void *data, int n_column, char **column_value, char **column_name)    //��ѯʱ�Ļص�����
{
	CAnalyser* _this = (CAnalyser*)data;
	_this->InitAgentInfoCallbackProc(n_column, column_value, column_name);	
	return 0;
}

int CAnalyser::InitAgentInfoCallbackProc(int n_column, char **column_value, char **column_name)
{
	int ret = -1;
	AgentInfo stAgentInfo;
	
	memset((unsigned char*)&stAgentInfo, 0, sizeof(AgentInfo));
	
	stAgentInfo.iSeq = atoi(column_value[0]);//�������
	memcpy(stAgentInfo.szDevId, column_value[1], DEVICE_ID_LEN);//�豸���
	memcpy(stAgentInfo.szAttrId, column_value[2], DEVICE_SN_LEN);//���Ա��
	strcpy(stAgentInfo.szCondition, column_value[3]);
	strcpy(stAgentInfo.szValibtime, column_value[4]);
	stAgentInfo.iInterval = atoi(column_value[5]);
	stAgentInfo.iTimes = atoi(column_value[6]);
	strcpy(stAgentInfo.szActDevId, column_value[7]);
	strcpy(stAgentInfo.szActActionId, column_value[8]);
	strcpy(stAgentInfo.szActValue, column_value[9]);
	stAgentInfo.cType = column_value[10][0];

	AnalyseHashAgentAdd(stAgentInfo);
	return ret;
}
void CAnalyser::getAgentMsg(int* pAgentId, AgentInfo stAgent)
{
	*pAgentId = stAgent.iSeq;

	ACT_FORMAT act_format;
	ACT_FORMAT* pact_format = &act_format;
	memset((unsigned char*)pact_format, 0, sizeof(ACT_FORMAT));

	pact_format->iSeq = stAgent.iSeq;
	memcpy(pact_format->szCondition, stAgent.szCondition, sizeof(stAgent.szCondition));  //����
	memcpy(pact_format->szValibtime, stAgent.szValibtime, sizeof(stAgent.szValibtime));  //��Чʱ��
	pact_format->iInterval = stAgent.iInterval*60;										//�������
	pact_format->iTimes = stAgent.iTimes;												//��������
	memcpy(pact_format->szActDevId, stAgent.szActDevId, sizeof(pact_format->szActDevId));  
	memcpy(pact_format->szActActionId, stAgent.szActActionId, sizeof(pact_format->szActActionId));  
	memcpy(pact_format->szActValue, stAgent.szActValue, sizeof(pact_format->szActValue));  
	pact_format->time_lastTime = 0;              //�ϴ�����ʱ��
	pact_format->times_lastTime = 1;             //�ϴ���������

	HashedAgentInfo::iterator it;
    it = m_pAgentHashTable->find((int)act_format.iSeq);	
	if (it != m_pAgentHashTable->end())
	{
		m_pAgentHashTable->erase(it);
	}
	m_pAgentHashTable->insert(HashedAgentInfo::value_type((int)act_format.iSeq,act_format));
}

//���ݴ����߳�
void * CAnalyser::pAnalyserThrd(void* arg)
{
    CAnalyser *_this = (CAnalyser *)arg;
    _this->MsgAnalyzer();
    return NULL;
}

/******************************************************************************/
/* Function Name  : MsgAnalyzer                                               */
/* Description    : ��ѯ�ظ����ݽ�������                                      */
/* Input          : none                                                      */
/* Output         : none                                                      */
/* Return         : none                                                      */
/******************************************************************************/
void CAnalyser::MsgAnalyzer()
{
    MSG_ANALYSE_HASH hash_node;
    QUEUEELEMENT pMsg = NULL;
    char szDeviceId[DEVICE_ID_LEN_TEMP] = {0};
    char szAttrSn[DEVICE_SN_LEN_TEMP] = {0};

    time_t tm_NowTime = 0;
	time_t tm_UpdateTime = 0;

	struct tm *tmNow = NULL;

	bool bDoAction = false;
	struct timeval tmval_NowTime;

	time(&tm_UpdateTime); 
	tmNow = localtime(&tm_UpdateTime);
	tmNow->tm_hour = 0;
	tmNow->tm_min = 0;
	tmNow->tm_sec = 0;

	tm_UpdateTime = mktime(tmNow);

	printf("CAnalyser::MsgAnalyzer .... Pid[%d]\n", getpid());

	int lightCnt = 1;
    while(true)
    {
        pthread_testcancel();
        time(&tm_NowTime); 
		tmNow = localtime(&tm_NowTime);
		lightCnt--;
		if (0 == lightCnt)
		{
			HardIF_DBGLightON();                //ϵͳ��
		}
		//���첢�����賿0��������ݿⱸ��
        if( (tm_NowTime - tm_UpdateTime) > ONE_DAY_SECONDS && 0 == tmNow->tm_hour)
        {
			tm_UpdateTime += ONE_DAY_SECONDS;
			//ÿ�·�һ����Ϣ�������ֻ�
			if(1 == tmNow->tm_mday)
			{
				g_GSMCtrl.SendProInfoToSmsMonthly();
			}

			g_SqlCtl.UpdateHistoryDB();
		}

		//�Ӵ�����������ȡ����
		if(MMR_OK != g_MsgMgr.GetFirstMsg(m_unHandleAnalyser, pMsg, 100))
		{
			pMsg = NULL;
			usleep(10*1000);
			if (0 == lightCnt)
			{
				lightCnt = 10;
				HardIF_DBGLightOFF();                //ϵͳ��
			}
			continue;
		}
		do 
		{			
			QUERY_INFO_CPM* stMsgFromPoll = (QUERY_INFO_CPM*)pMsg;
			
			DBG_ANA(("A-------   devid:[%s]  TYPE:[%s]  TIME:[%s] VTYPE:[%d]  VALUE:[%s] Cmd[%d]\n", stMsgFromPoll->DevId, stMsgFromPoll->Type, stMsgFromPoll->Time, stMsgFromPoll->VType, stMsgFromPoll->Value, stMsgFromPoll->Cmd));
			memset(szDeviceId, 0, sizeof(szDeviceId));
			memcpy(szDeviceId, stMsgFromPoll->DevId, DEVICE_ID_LEN);

			memset(szAttrSn, 0, sizeof(szAttrSn));
			memcpy(szAttrSn, stMsgFromPoll->DevId+DEVICE_ID_LEN, DEVICE_SN_LEN);

			bDoAction = false;

			gettimeofday(&tmval_NowTime, NULL);
			
			//��ѯhash����ȡ����Դ����
			switch(stMsgFromPoll->Cmd)
			{
			case 0://�ɼ���������
				{
					memset((unsigned char*)&hash_node, 0, sizeof(MSG_ANALYSE_HASH));
					memcpy(hash_node.key, stMsgFromPoll->DevId, DEVICE_ATTR_ID_LEN);
					if( !GetHashData(hash_node))
					{
						DBG_ANA(("Can not find device id[%s]\n", hash_node.key));
						break;
					}

					if(NULL == stMsgFromPoll->Value || 0 == strlen(stMsgFromPoll->Value))
						break;
					char temp[MAX_PACKAGE_LEN_TERMINAL] = {0};
					//printf("A------1.3DataStatus[%d]\n", stMsgFromPoll->DataStatus);
					if (0 == strcmp(stMsgFromPoll->Value, "NULL"))
					{
						strcpy(temp, "NULL");
					}
					else if ( 0 < strlen(hash_node.statusDef))//��״̬������
					{
						sprintf(temp, "%d", atoi(stMsgFromPoll->Value));
					}
					else
					{
						memcpy(temp, stMsgFromPoll->Value, sizeof(stMsgFromPoll->Value));
					}		
					
					if (0 == stMsgFromPoll->DataStatus)
					{
						//�����ǰ����״̬Ϊ����������Ҫ���澯���е����ݸĳ��Զ�����
						if(1 == hash_node.dataStatus)
						{
							CSqlCtl::UpdateAlertInfoTable(szDeviceId, szAttrSn,  '2', stMsgFromPoll->Time);
							CSqlCtl::DeleteFromAlertNowTable(szDeviceId, szAttrSn, 0);
							//������������״̬
							hash_node.dataStatus = stMsgFromPoll->DataStatus;
							updateNodePara(hash_node, 4);

							DEVICE_INFO_POLL deviceNode;
							if(SYS_STATUS_SUCCESS == g_DevInfoContainer.GetAttrNode(stMsgFromPoll->DevId, deviceNode))
							{
								memset(deviceNode.stLastValueInfo.szLevel, 0, 4);
								memset(deviceNode.stLastValueInfo.szDescribe, 0, sizeof(deviceNode.stLastValueInfo.szDescribe));
								g_DevInfoContainer.UpdateDeviceAlarmValue(deviceNode);
							}
						}					
					}
//debug 3	pass	
					
					//���ݷ����仯,���߳�ʱ��δ�����ݲ���,�����ݲ��뵽���ݱ�
					unsigned long time_elipse = (tmval_NowTime.tv_sec - hash_node.LastTime.tv_sec)*1000 + (tmval_NowTime.tv_usec - hash_node.LastTime.tv_usec)/1000;
					
					if(f_valueChangedCheck(stMsgFromPoll->VType, stMsgFromPoll->Value, hash_node.unionLastValue) //���ݷ����仯
						|| time_elipse >= 3600000																 //����1Сʱδ��������
						|| stMsgFromPoll->VType == CHARS
						|| stMsgFromPoll->VType == BYTES
						|| stMsgFromPoll->VType == BCD						
						)																 
					{
						//debug 4	pass
						
						if (0 != strcmp(temp, "NULL"))
						{
							DEVICE_INFO_POLL deviceNode;
							if(SYS_STATUS_SUCCESS == g_DevInfoContainer.GetAttrNode(stMsgFromPoll->DevId, deviceNode))
							{
								if(1 == g_ModeCfg.DB_Insert)
									InsertDataTable(hash_node, "data", stMsgFromPoll->Time, temp, stMsgFromPoll->DataStatus, true);//���뵽���ݱ�
								if('2' == deviceNode.cLedger)//��Ҫ����ͳ��
								{//��ֵת��Ϊ�û�����
									char szUserId[21] = {0};
									char szUserName[21] = {0};
									if(SYS_STATUS_SUCCESS != g_SqlCtl.GetUserInfoByCardId(temp, szUserId, szUserName))
										break;
									//���뵽ATS��
									InsertATSTable(hash_node, (char*)"ATS", stMsgFromPoll->Time, temp, szUserId);
								}
							}
						}
						//debug 5	
						
						//��������ֵ�����¸���ʱ��lsd
						memcpy((unsigned char*)&hash_node.unionLastValue, stMsgFromPoll->Value, sizeof(UnionLastValue));
						memcpy((unsigned char*)(&hash_node.LastTime), (unsigned char*)(&tmval_NowTime), sizeof(struct timeval));
						updateNodePara(hash_node, 1);
						updateNodePara(hash_node, 2);
						//�����ϴ���ƽ̨
						if( g_upFlag == 1 )
						{
							DEVICE_DETAIL_INFO deviceInfo;
							memset((unsigned char*)&deviceInfo, 0, sizeof(DEVICE_DETAIL_INFO));
							memcpy(deviceInfo.Id, hash_node.key, DEVICE_ID_LEN);
							char szUnit[11] = {0};
							DEVICE_INFO_POLL stDevicePoll;
							if (SYS_STATUS_SUCCESS == g_DevInfoContainer.GetDeviceDetailNode(deviceInfo)
								&& SYS_STATUS_SUCCESS == g_DevInfoContainer.GetParaUnit(atoi(stMsgFromPoll->Type), szUnit)
								&& SYS_STATUS_SUCCESS == g_DevInfoContainer.GetAttrNode(stMsgFromPoll->DevId, stDevicePoll))
							{
								if (1 == stDevicePoll.isUpload)
								{
									char updateData[MAXMSGSIZE] = {0};
									memset(updateData, 0, sizeof(updateData));
									char* ptrBuf = updateData;
									memcpy(ptrBuf, hash_node.key, DEVICE_ID_LEN);//deviceId
									ptrBuf += DEVICE_ID_LEN;

									sprintf(ptrBuf, "%-30s", deviceInfo.szCName);//deviceName
									ptrBuf += 30;

									strncpy(ptrBuf, hash_node.key+DEVICE_ID_LEN, DEVICE_SN_LEN);//attrId
									ptrBuf += DEVICE_SN_LEN;

									sprintf(ptrBuf, "%-20s", stDevicePoll.szAttrName);//deviceName
									ptrBuf += 20;

									sprintf(ptrBuf, "%-20s", stMsgFromPoll->Time);
									ptrBuf += 20;

									sprintf(ptrBuf, "%-128s", stMsgFromPoll->Value);
									ptrBuf += 128;

									sprintf(ptrBuf, "%-10s", szUnit);
									ptrBuf += 10;

									Net_Up_Msg(CMD_ENVIRON_DATA_UPDATE, updateData, ptrBuf - updateData);
								}
							}
						}
					}
					else
					{
						DBG_ANA(("����������\n"));
					}
					//��Ҫִ����������
					bDoAction = true;
				}
				break;
			case 1://ϵͳ�澯��֪ͨ
				//DBG_ANA(("���ߡ���������\n"));
				{
					UpdateDeviceStatus(szDeviceId, stMsgFromPoll->Value, stMsgFromPoll->Time);
					g_DevInfoContainer.HashDeviceStatusUpdate(szDeviceId, atoi(stMsgFromPoll->Value));//�����豸����豸״̬

					memset((unsigned char*)&hash_node, 0, sizeof(MSG_ANALYSE_HASH));
					memcpy(hash_node.key, stMsgFromPoll->DevId, DEVICE_ATTR_ID_LEN);
					DBG_ANA(("ϵͳ֪ͨ id[%s]\n", hash_node.key));
					if( GetHashData(hash_node))
					{
						DoOffLineAct(szDeviceId, szAttrSn, &hash_node.agentSeq[0], hash_node.actCnt, false);
						//updateNodePara(hash_node, 3);
					}
				}
				break;
			case 2://�豸�����������
				DBG_ANA(("������������\n"));
				UpdateDeviceStatus(szDeviceId, stMsgFromPoll->Value, stMsgFromPoll->Time);
				break;
			case 3://ϵͳ�澯�ظ�֪ͨ
				{
					memset((unsigned char*)&hash_node, 0, sizeof(MSG_ANALYSE_HASH));
					memcpy(hash_node.key, stMsgFromPoll->DevId, DEVICE_ATTR_ID_LEN);
					DBG_ANA(("ϵͳ�ظ�֪ͨ id[%s]\n", hash_node.key));
					if( GetHashData(hash_node))
					{
						DoOffLineAct(szDeviceId, szAttrSn, &hash_node.agentSeq[0], hash_node.actCnt, true);
						//updateNodePara(hash_node, 3);
					}
				}
				break;
			case 4://�龰ģʽ����
				{
					memset((unsigned char*)&hash_node, 0, sizeof(MSG_ANALYSE_HASH));
					memcpy(hash_node.key, stMsgFromPoll->DevId, DEVICE_ATTR_ID_LEN);
					DBG_ANA(("�龰ģʽ֪ͨ id[%s]\n", hash_node.key));
					if( GetHashData(hash_node))
					{
						bDoAction = true;
					}
				}
				break;
			}
			//-------------------���ݽ�������-------------------
			if (bDoAction)
			{
				DecodeAct(szDeviceId, szAttrSn, stMsgFromPoll->VType, stMsgFromPoll->Value,  &hash_node.agentSeq[0], hash_node.actCnt, stMsgFromPoll->Time);    //������������
				
			}
		} while (false);
		if (NULL != pMsg)
		{
			MM_FREE(pMsg);
			pMsg = NULL;
		}
		if (0 == lightCnt)
		{
			lightCnt = 10;
			HardIF_DBGLightOFF();                //ϵͳ��
		}
    }
}
/******************************************************************************/
/* Function Name  : UpdateDeviceStatus                                            */
/* Description    : �豸״̬���                                            */
/* Input          : int   vType          ��ֵ����                             */
/*                  char* value          ����ֵ                               */
/*                  char* standard       ����ֵ��Χ                           */
/* Output         : none                                                      */
/* Return         : true       ������������Χ��                               */
/*                  false      ������������Χ��                               */
/******************************************************************************/
void CAnalyser::UpdateDeviceStatus(char* deviceId, char* status, char* strTime)
{
	char sqlData[1024] = {0};

	//����hash��״̬
	deviceStatusUpdate( deviceId, atoi(status));

	//�������ݿ��豸״̬
	strcat(sqlData, "status='");
	strcat(sqlData, status);
	strcat(sqlData, "' where id = '");
	strcat(sqlData, deviceId);
	strcat(sqlData, "'");
	g_SqlCtl.update_into_table("device_detail", sqlData, true);

	DBG_ANA(("UpdateDeviceStatus status[%d]\n", atoi(status)));
	//������쳣״̬������뵽�澯��
	
	switch(atoi(status))
	{
	case DEVICE_NOT_USE://����
		{
		}
		break;
	case DEVICE_OFFLINE://����
		{
			CSqlCtl::InsertAlertInfoTable(deviceId, (char*)"", strTime, ALARM_SERIOUS, (char*)"�豸����",'0',(char*)"");
			CSqlCtl::DeleteFromAlertNowTable(deviceId, (char*)"", 1);

			DEVICE_DETAIL_INFO deviceInfo;
			memset((unsigned char*)&deviceInfo, 0, sizeof(DEVICE_DETAIL_INFO));
			memcpy(deviceInfo.Id, deviceId, DEVICE_ID_LEN);
			if (SYS_STATUS_SUCCESS == g_DevInfoContainer.GetDeviceDetailNode(deviceInfo))
			{
				char updateData[MAXMSGSIZE] = {0};
				memset(updateData, 0, sizeof(updateData));
				char* ptrBuf = updateData;
				memcpy(ptrBuf, deviceId, DEVICE_ID_LEN);
				ptrBuf += DEVICE_ID_LEN;

				sprintf(ptrBuf, "%-30s", deviceInfo.szCName);
				ptrBuf += 30;

				sprintf(ptrBuf, "%-4s", "");
				ptrBuf += 4;

				sprintf(ptrBuf, "%-20s", "");
				ptrBuf += 20;

				sprintf(ptrBuf, "%d", ALARM_SERIOUS);
				ptrBuf += 1;

				sprintf(ptrBuf, "%-20s", strTime);
				ptrBuf += 20;

				sprintf(ptrBuf, "%-128s", "�豸����");
				ptrBuf += 128;
				Net_Up_Msg(CMD_ALARM_UPLOAD, updateData, ptrBuf - updateData);
			}
		}
		break;
	case DEVICE_ONLINE://����
		//CSqlCtl::InsertAlertInfoTable(deviceId, "", strTime, ALARM_ATTENTION, "�豸�ָ�����", '3', "");
		CSqlCtl::UpdateAlertInfoTable(deviceId, (char*)"", '2', strTime);
		CSqlCtl::DeleteFromAlertNowTable(deviceId, (char*)"", 0);

		DEVICE_DETAIL_INFO deviceInfo;
		memset((unsigned char*)&deviceInfo, 0, sizeof(DEVICE_DETAIL_INFO));
		memcpy(deviceInfo.Id, deviceId, DEVICE_ID_LEN);
		if (SYS_STATUS_SUCCESS == g_DevInfoContainer.GetDeviceDetailNode(deviceInfo))
		{
			char updateData[MAXMSGSIZE] = {0};
			memset(updateData, 0, sizeof(updateData));
			char* ptrBuf = updateData;
			memcpy(ptrBuf, deviceId, DEVICE_ID_LEN);
			ptrBuf += DEVICE_ID_LEN;

			sprintf(ptrBuf, "%-30s", deviceInfo.szCName);
			ptrBuf += 30;

			sprintf(ptrBuf, "%-4s", "");
			ptrBuf += 4;

			sprintf(ptrBuf, "%-20s", "");
			ptrBuf += 20;

			sprintf(ptrBuf, "%d", ALARM_ATTENTION);
			ptrBuf += 1;

			sprintf(ptrBuf, "%-20s", strTime);
			ptrBuf += 20;

			sprintf(ptrBuf, "%-128s", "�豸�ָ�����");
			ptrBuf += 128;
			Net_Up_Msg(CMD_ALARM_UPLOAD, updateData, ptrBuf - updateData);
		}
		break;
		
	default:
		DBG_ANA(("δ֪״̬ [%d]\n", atoi(status)));
	}
	
	//�豸״̬����ϴ���ƽ̨

	{
		char updateData[2048] = {0};
		memset(updateData, 0, sizeof(updateData));

		DEVICE_DETAIL_INFO deviceInfo;
		memset((unsigned char*)&deviceInfo, 0, sizeof(DEVICE_DETAIL_INFO));
		memcpy(deviceInfo.Id, deviceId, DEVICE_ID_LEN);

		if (SYS_STATUS_SUCCESS == g_DevInfoContainer.GetDeviceDetailNode(deviceInfo))
		{
			char* ptrBuf = updateData;
			memcpy(ptrBuf, deviceId, DEVICE_ID_LEN);
			ptrBuf += DEVICE_ID_LEN;

			sprintf(ptrBuf, "%-30s", deviceInfo.szCName);
			ptrBuf += 30;

			ptrBuf[0] = status[0];
			ptrBuf += 1;

			DBG_ANA(("�豸״̬���[%s]\n", updateData));
			Net_Up_Msg(CMD_DEVICE_STATUS_UPDATE, updateData, ptrBuf - updateData);
		}
	}
}

/******************************************************************************/
/* Function Name  : InsertDataTable                                            */
/* Description    : �豸״̬���                                            */
/* Input          : int   vType          ��ֵ����                             */
/*                  char* value          ����ֵ                               */
/*                  char* standard       ����ֵ��Χ                           */
/* Output         : none                                                      */
/* Return         : true       ������������Χ��                               */
/*                  false      ������������Χ��                               */
/******************************************************************************/
void CAnalyser::InsertDataTable(MSG_ANALYSE_HASH hash_node, const char* szTableName, char* time, char* szValue , int status, bool bRedo)
{
	char sqlData[512] = {0};
	char szDeviceId[DEVICE_ID_LEN_TEMP] = {0};
	char szAttrSn[DEVICE_SN_LEN_TEMP] = {0};
	char szStatus[4] = {0};

	memset(szDeviceId, 0, sizeof(szDeviceId));
	memset(szAttrSn, 0, sizeof(szAttrSn));

	memcpy(szDeviceId, hash_node.key, DEVICE_ID_LEN);
	memcpy(szAttrSn,   hash_node.key + DEVICE_ID_LEN, DEVICE_SN_LEN);

	sprintf(szStatus, "%d", status);

	strcat(sqlData, "null, ");
	strcat(sqlData, "'");
	strcat(sqlData, szDeviceId);        //�豸���
	strcat(sqlData, "','");
	strcat(sqlData, szAttrSn);
	strcat(sqlData, "', datetime('");
	strcat(sqlData, time);         //�ɼ�ʱ��
	strcat(sqlData, "'), '");
	strcat(sqlData, szValue);        //�ɼ�����
	strcat(sqlData, "', '");
	strcat(sqlData, szStatus);		//����״̬
	strcat(sqlData, "', '0', '', ''");         //����״̬
	g_SqlCtl.insert_into_table( szTableName, sqlData, bRedo);               //��ȡ�����ݲ������ݿ�

}


/******************************************************************************/
void CAnalyser::f_DelData()
{
	char sqlData[512] = {0};
    //strcat(sqlData, "where julianday(strftime(\'%Y-%m-%d\',\'now\'))-julianday(strftime(\'%Y-%m-%d\', ctime))>60");
    strcat(sqlData, "where sn not in (select sn from data desc limit 0,1000000)" ); 
	g_SqlCtl.delete_record("data", sqlData, false);
}


/******************************************************************************/
/* Function Name  : f_valueChangedCheck                                       */
/* Description    : ���ݱ仯�ж�                                            */
/* Input          : int   vType          ��ֵ����                             */
/*                  char* value          ����ֵ                               */
/*                  char* lastValue      ��������ֵ                           */
/* Output         : none                                                      */
/* Return         : true       ���ݷ����仯                               */
/*                  false      ����δ�����仯                               */
/******************************************************************************/
bool CAnalyser::f_valueChangedCheck(int vType, char* value, UnionLastValue unionLastValue)
{
	bool ret = false;
	int cmpLen = 0;
	switch(vType)
	{
	case SHORT:
		cmpLen = sizeof(short);
		break;
	case INTEGER:
		cmpLen = sizeof(int);
		break;
	case FLOAT:
		cmpLen = sizeof(float);
		break;
	case DOUBLE:
		cmpLen = sizeof(double);
		break;
	case CHAR:
		cmpLen = sizeof(char);
		break;
	case CHARS:
	case BYTES:
	case BCD:
		cmpLen = 128;
		break;
	default:
		break;
	}
	if (cmpLen > 0)
	{
		if(0 != memcmp(value, &unionLastValue, cmpLen))//ֵ�б仯
		{
			ret = true;
		}
	}
	return ret;
}

/******************************************************************************/
/* Function Name  : DecodeAct                                                 */
/* Description    : ������������                                              */
/* Input          : char* deviceId       ������Դ�豸���                     */
/*                  char* ctype          ��������                             */
/*                  char* vType          ��ֵ����                             */
/*                  char* value          ����ֵ                               */
/*                  char* a_format       ����Э��                             */
/* Output         : none                                                      */
/* Return         : none                                                      */
/******************************************************************************/
void CAnalyser::DecodeAct(char* deviceId, const char* sn,int vType, char* value, int* pSeq, int actCnt, char* strTime)
{
    char act_msg[MAXMSGSIZE] = {0};
    char inCondition[129] = {0};                         //a_format����
    char timeFormat[30] = {0};                          //��Чʱ�䷶Χ
    time_t tm_NowTime;

    memset(act_msg, 0, sizeof(act_msg));
    ACTION_MSG *pSendMsg = (ACTION_MSG *)act_msg;
	ACT_FORMAT a_format;
    //---------------------------------����������������-------------------------------
    for( int j=0; j<actCnt; j++ )
    {
        memset(inCondition, 0, sizeof(inCondition));
        memset(timeFormat, 0, sizeof(timeFormat));
		memset((unsigned char*)&a_format, 0, sizeof(ACT_FORMAT));
		//lhy����������ŵ��������ñ��ȡ��������lhylhy
		HashedAgentInfo::iterator it;
		CTblLock.lock();
		a_format.iSeq = pSeq[j];
		it = m_pAgentHashTable->find((int)a_format.iSeq);	
		if (it == m_pAgentHashTable->end())
		{
			CTblLock.unlock();
			continue;
		}

		memcpy((unsigned char*)&a_format, (unsigned char*)&it->second.iSeq, sizeof(ACT_FORMAT));
		CTblLock.unlock();

        memcpy(inCondition, a_format.szCondition, sizeof(a_format.szCondition));
        DBG_ANA(("condition:[%s]\n", inCondition));
		char attrId[DEVICE_ATTR_ID_LEN_TEMP] = {0};
		strncpy(attrId, deviceId, DEVICE_ID_LEN);
		strncpy(attrId + DEVICE_ID_LEN, sn, DEVICE_SN_LEN);
		if(!ConditionCheck(attrId, vType, value, inCondition))//���������㣬������������
		{
			//�������ָ����������¼��� lsd 2016.1.4
			if(1 != a_format.times_lastTime)
			{
				a_format.times_lastTime = 1;
				CTblLock.lock();
				it = m_pAgentHashTable->find((int)a_format.iSeq);	
				if (it == m_pAgentHashTable->end())
				{
					CTblLock.unlock();
					continue;
				}
				((PACT_FORMAT)&it->second.iSeq)->times_lastTime = a_format.times_lastTime;	//lsd 2016.1.4
				CTblLock.unlock();
			}
			continue;
		}

		memcpy(timeFormat, a_format.szValibtime, sizeof(a_format.szValibtime));
		time(&tm_NowTime);                 //��ȡ��ǰʱ��
		if( tm_NowTime - a_format.time_lastTime < a_format.iInterval )
		{
			DBG_ANA(("������������ڣ��������� NowTime[%ld] oldTime[%ld] �������[%d]\n", tm_NowTime, a_format.time_lastTime, a_format.iInterval));
			continue;
		}  
		DBG_ANA(("������ǰ[%d] һ��[%d]\n", a_format.times_lastTime, a_format.iTimes));
		if( a_format.times_lastTime > a_format.iTimes && 0 != a_format.iTimes)	//iTimes=0��ʾ��������������
		{
			DBG_ANA(("���������������������� ������ǰ[%d] һ��[%d]\n", a_format.times_lastTime, a_format.iTimes));
			continue;
		}
		if( f_timeCheck( tm_NowTime, timeFormat ) == false )
		{
			DBG_ANA(("���ڲ���ʱ���ڣ���������[%s]\n", timeFormat));
			continue;
		}   //ʱ���ⲻͨ������������

		DBG_ANA(("DecodeAct Begin--deviceId[%s]- sn[%s]---actCnt:[%d] szCondition[%s] szActValue[%s]\n",deviceId, sn,  actCnt, a_format.szCondition, a_format.szActValue));
	    memcpy(pSendMsg->DstId, a_format.szActDevId, sizeof(a_format.szActDevId));    //Ŀ���豸ID
		pSendMsg->DstId[sizeof(a_format.szActDevId)] = '\0';
        

		memcpy(pSendMsg->ActionSn, a_format.szActActionId, sizeof(a_format.szActActionId));		
		memcpy(pSendMsg->ActionValue, a_format.szActValue, sizeof(a_format.szActValue));
		pSendMsg->ActionValue[sizeof(a_format.szActValue)] = '\0';
		char szData[257] = {0};
		memcpy(szData, pSendMsg->ActionValue, sizeof(pSendMsg->ActionValue));
		char* t_posi = strstr(szData, "[V]");
		if( t_posi != NULL )
		{
			char temp[257] = {0};
			memcpy(temp, szData, sizeof(szData));
			memset(szData, 0, sizeof(szData));
			t_posi = strstr(temp, "[V]");
			memcpy(szData, temp, t_posi - temp);
			strcat(szData, value);
			strcat(szData, t_posi+3);
			szData[256] = '\0';
			memset(pSendMsg->ActionValue, 0, sizeof(pSendMsg->ActionValue));
			strcpy(pSendMsg->ActionValue, szData);
		}

        a_format.time_lastTime = tm_NowTime;      //����ʱ�����
		a_format.times_lastTime ++;				  //������������

        memcpy(pSendMsg->SrcId, deviceId, DEVICE_ID_LEN);                       //Դ�豸ID
        pSendMsg->SrcId[DEVICE_ID_LEN] = '\0';

	    strncpy(pSendMsg->AttrSn, sn, DEVICE_SN_LEN);		//��������
		strncpy(pSendMsg->SrcValue, value, 128);//����ֵ

        memcpy(pSendMsg->Operator, "AUTO", 4);  //������
        pSendMsg->Operator[4] = '\0';
        pSendMsg->ActionSource = ACTION_SOURCE_SYSTEM;

	//	Time2Chars(&tm_NowTime, pSendMsg->szActionTimeStamp);
		strcpy(pSendMsg->szActionTimeStamp, strTime);
        DBG_ANA(("pSendMsg    ID:[%s]   Value:[%s]\n", pSendMsg->DstId, pSendMsg->ActionValue));
        

		//�澯���ݴ���,lhy
		if (0 == strncmp(pSendMsg->DstId, CPM_GATEWAY_ID, DEVICE_ID_LEN))
		{
			char level[5] = {0};
			char szDescribe[257] = {0};
			char* valuePos = strstr(pSendMsg->ActionValue, ":");
			if (NULL == valuePos || 4 < (valuePos - pSendMsg->ActionValue) || valuePos == pSendMsg->ActionValue)
			{
				strcpy(szDescribe, pSendMsg->ActionValue);
			}
			else//���ɼ���
			{
				strncpy(level, pSendMsg->ActionValue, valuePos - pSendMsg->ActionValue);
				strcpy(szDescribe, valuePos + 1);
			}

			CSqlCtl::InsertAlertInfoTable(pSendMsg->SrcId, pSendMsg->AttrSn, pSendMsg->szActionTimeStamp,ALARM_GENERAL, szDescribe,'0',level);
			CSqlCtl::DeleteFromAlertNowTable(pSendMsg->SrcId, pSendMsg->AttrSn, 1);

			char deviceAttrId[DEVICE_ATTR_ID_LEN_TEMP] = {0};
			memcpy(deviceAttrId, deviceId, DEVICE_ID_LEN);
			memcpy(deviceAttrId + DEVICE_ID_LEN, sn, DEVICE_SN_LEN);
			DEVICE_INFO_POLL deviceNode;
			if(SYS_STATUS_SUCCESS == g_DevInfoContainer.GetAttrNode(deviceAttrId, deviceNode))
			{
				memcpy(deviceNode.stLastValueInfo.szLevel, level, 4);
				memcpy(deviceNode.stLastValueInfo.szDescribe, szDescribe, 128);
				g_DevInfoContainer.UpdateDeviceAlarmValue(deviceNode);
			}
			//�������ݱ��¼
			const char* sqlFormat = "update %s "
				"set level = '%s', desc = '%s' "
				"where id = '%s' and ctype = '%s' and ctime = '%s';";
			char sql[512] = {0};
			sprintf(sql, sqlFormat, 
				"data",
				level, 
				szDescribe, 
				pSendMsg->SrcId, 
				pSendMsg->AttrSn, 
				pSendMsg->szActionTimeStamp);
			g_SqlCtl.ExecSql(sql, NULL, NULL, true, 0);

			//���µ�ǰ�����������״̬
			MSG_ANALYSE_HASH hash_node;
			memset((unsigned char*)&hash_node, 0, sizeof(MSG_ANALYSE_HASH));
			memcpy(hash_node.key, deviceId, DEVICE_ID_LEN);
			memcpy(hash_node.key + DEVICE_ID_LEN, sn, DEVICE_SN_LEN);
			if( GetHashData(hash_node))
			{
				hash_node.dataStatus = 1;
				updateNodePara(hash_node, 4);
			}
		}
		
		CTblLock.lock();
		it = m_pAgentHashTable->find((int)a_format.iSeq);	
		if (it == m_pAgentHashTable->end())
		{
			CTblLock.unlock();
			continue;
		}
		((PACT_FORMAT)&it->second.iSeq)->time_lastTime = a_format.time_lastTime;
		((PACT_FORMAT)&it->second.iSeq)->times_lastTime = a_format.times_lastTime;	//lsd 2016.1.4
		CTblLock.unlock();

		int patchRet = CDevInfoContainer::DisPatchAction(*pSendMsg);
		if (SYS_STATUS_SUBMIT_SUCCESS_NEED_RESP != patchRet && SYS_STATUS_SUBMIT_SUCCESS_IN_LIST != patchRet)
		{
			CDevInfoContainer::SendToActionSource(*pSendMsg, patchRet);
		}
    }

	//DBG_ANA(("DecodeAct End--deviceId[%s]- sn[%s]---actCnt:[%d] szCondition[%s] szActValue[%s]\n",deviceId, sn,  actCnt, a_format->szCondition, a_format->szActValue));
}


/******************************************************************************/
/* Function Name  : DoOffLineAct                                                 */
/* Description    : ������������                                              */
/* Input          : char* deviceId       ������Դ�豸���                     */
/*                  char* ctype          ��������                             */
/*                  char* vType          ��ֵ����                             */
/*                  char* value          ����ֵ                               */
/*                  char* a_format       ����Э��                             */
/* Output         : none                                                      */
/* Return         : none                                                      */
/******************************************************************************/
void CAnalyser::DoOffLineAct(char* deviceId, const char* sn, int* pSeq, int actCnt, bool bIsReNotice)
{
	char act_msg[MAXMSGSIZE] = {0};

	char timeFormat[30] = {0};                          //��Чʱ�䷶Χ
	time_t tm_NowTime;

	ACTION_MSG *pSendMsg = (ACTION_MSG *)act_msg;


	ACT_FORMAT a_format;
	//---------------------------------����������������-------------------------------
	for( int j=0; j<actCnt; j++ )
	{
		memset(act_msg, 0, sizeof(act_msg));
		memset(timeFormat, 0, sizeof(timeFormat));		
		memset((unsigned char*)&a_format, 0, sizeof(ACT_FORMAT));

		HashedAgentInfo::iterator it;
		CTblLock.lock();
		a_format.iSeq = pSeq[j];
		it = m_pAgentHashTable->find((int)a_format.iSeq);	
		if (it == m_pAgentHashTable->end())
		{
			CTblLock.unlock();
			continue;
		}
		
		memcpy((unsigned char*)&a_format, (unsigned char*)&it->second.iSeq, sizeof(ACT_FORMAT));
		CTblLock.unlock();

		memcpy(timeFormat, a_format.szValibtime, sizeof(a_format.szValibtime));
		time(&tm_NowTime);                 //��ȡ��ǰʱ��
		
		if (bIsReNotice)//������ظ�֪ͨ
		{
			if (0 == a_format.iInterval//���û���������
				|| ( tm_NowTime - a_format.time_lastTime < a_format.iInterval ))//������������ڣ���������
			{
				DBG_ANA(("������������ڻ�ֻ��Ҫһ���������������� NowTime[%ld] oldTime[%ld] �������[%d]\n", tm_NowTime, a_format.time_lastTime, a_format.iInterval));
				continue;
			}
			DBG_ANA(("�ظ�֪ͨ NowTime[%ld] oldTime[%ld] �������[%d]\n", tm_NowTime, a_format.time_lastTime, a_format.iInterval));
		}

		if( false == f_timeCheck( tm_NowTime, timeFormat ))
		{
			DBG_ANA(("���ڲ���ʱ���ڣ���������[%s]\n", timeFormat));
			continue;
		}   //ʱ���ⲻͨ������������

		memcpy(pSendMsg->DstId, a_format.szActDevId, sizeof(a_format.szActDevId));    //Ŀ���豸ID
		pSendMsg->DstId[sizeof(a_format.szActDevId)] = '\0';

		memcpy(pSendMsg->ActionSn, a_format.szActActionId, sizeof(a_format.szActActionId));

		memcpy(pSendMsg->ActionValue, a_format.szActValue, sizeof(a_format.szActValue));
		pSendMsg->ActionValue[sizeof(a_format.szActValue)] = '\0';

		a_format.time_lastTime = tm_NowTime;      //����ʱ�����

		memcpy(pSendMsg->SrcId, deviceId, DEVICE_ID_LEN);                       //Դ�豸ID
		pSendMsg->SrcId[DEVICE_ID_LEN] = '\0';

		strncpy(pSendMsg->AttrSn, sn, DEVICE_SN_LEN);		//��������

		memcpy(pSendMsg->Operator, "AUTO", 4);  //������
		pSendMsg->Operator[4] = '\0';
		pSendMsg->ActionSource = ACTION_SOURCE_SYSTEM;
		DBG_ANA(("DoOffLineAct pSendMsg    ID:[%s]   Value:[%s]\n", pSendMsg->DstId, pSendMsg->ActionValue));


		CTblLock.lock();
		it = m_pAgentHashTable->find((int)a_format.iSeq);	
		if (it == m_pAgentHashTable->end())
		{
			CTblLock.unlock();
			continue;
		}
		((PACT_FORMAT)&it->second.iSeq)->time_lastTime = a_format.time_lastTime;
	
		CTblLock.unlock();

		int patchRet = CDevInfoContainer::DisPatchAction(*pSendMsg);

		if (SYS_STATUS_SUBMIT_SUCCESS_NEED_RESP != patchRet && SYS_STATUS_SUBMIT_SUCCESS_IN_LIST != patchRet)
		{
			CDevInfoContainer::SendToActionSource(*pSendMsg, patchRet);
		}
	}
}



/******************************************************************************/
bool CAnalyser::f_timeCheck(time_t tm_NowTime, char *timeFormat)
{
    bool ret = false;
	char timeTemp[2][15] = {{0}};
    char *split_result = NULL;
    struct tm st_startTime;
    struct tm st_endTime;
    struct tm *st_nowTime;
    char timeTrans[3] = {0};
	char* savePtr = NULL;
    if( 0>= strlen(timeFormat))
    {
        return true;
    }
    split_result = strtok_r(timeFormat, ",", &savePtr);
    if( split_result != NULL )
    {
        strcat(timeTemp[0], split_result);
        split_result = strtok_r(NULL, ",", &savePtr);
        if( split_result != NULL )
        {
            strcat(timeTemp[1], split_result);
        }
        else
        {
            return ret;
        }
    }
    else
    {
        return ret;
    }

    if( timeTemp[0][0] == '*' )            //��ѭ��
    {
        st_nowTime = localtime(&tm_NowTime);
        memset(timeTrans, 0, sizeof(timeTrans));
        timeTrans[0] = timeTemp[0][1];
        timeTrans[1] = timeTemp[0][2];
        st_startTime.tm_hour = atoi(timeTrans);
        memset(timeTrans, 0, sizeof(timeTrans));
        timeTrans[0] = timeTemp[0][3];
        timeTrans[1] = timeTemp[0][4];
        st_startTime.tm_min = atoi(timeTrans);
        memset(timeTrans, 0, sizeof(timeTrans));
        timeTrans[0] = timeTemp[1][1];
        timeTrans[1] = timeTemp[1][2];
        st_endTime.tm_hour = atoi(timeTrans);
        memset(timeTrans, 0, sizeof(timeTrans));
        timeTrans[0] = timeTemp[1][3];
        timeTrans[1] = timeTemp[1][4];
        st_endTime.tm_min = atoi(timeTrans);

        //DBG_ANA(("start time:[%d:%d]\n", st_startTime.tm_hour, st_startTime.tm_min));
        //DBG_ANA(("end time:[%d:%d]\n", st_endTime.tm_hour, st_endTime.tm_min));
        //DBG_ANA(("now time:[%d:%d]\n", st_nowTime->tm_hour, st_nowTime->tm_min));
        if( (st_startTime.tm_hour < st_endTime.tm_hour)         //��ʼ������ʱ����ͬһ��
        || ( (st_startTime.tm_hour==st_endTime.tm_hour) && (st_startTime.tm_min<st_endTime.tm_min) ) )
        {
            if( st_nowTime->tm_hour > st_startTime.tm_hour )            //
            {
                if( st_nowTime->tm_hour < st_endTime.tm_hour )
                {
                    ret = true;
                }
                else if( (st_nowTime->tm_hour == st_endTime.tm_hour) && (st_nowTime->tm_min <= st_endTime.tm_hour))
                {
                    ret =true;
                }
            }
            else if( st_nowTime->tm_hour == st_startTime.tm_hour )
            {
                if( st_nowTime->tm_min >= st_startTime.tm_min )
                {
                    if( st_nowTime->tm_hour < st_endTime.tm_hour )
                    {
                        ret = true;
                    }
                    else if( (st_nowTime->tm_hour == st_endTime.tm_hour) && (st_nowTime->tm_min <= st_endTime.tm_min) )
                    {
                        ret = true;
                    }
                }
            }
        }
        else                      //��ʼ������ʱ�����
        {
            if( (st_nowTime->tm_hour > st_startTime.tm_hour) || (st_nowTime->tm_hour < st_endTime.tm_hour) )
            {
                ret = true;
            }
            else if( ( st_nowTime->tm_hour == st_startTime.tm_hour) && ( st_nowTime->tm_min >= st_startTime.tm_min) )
            {
                ret = true;
            }
            else if( ( st_nowTime->tm_hour == st_endTime.tm_hour) && ( st_nowTime->tm_min <= st_endTime.tm_min) )
            {
                ret = true;
            }
        }
    }
    else                                   //ʱ���
    {
    }

    return ret;
}

/******************************************************************************/
/* Function Name  : conditionCheck                                            */
/* Description    : ����ƥ��                                                  */
/* Input          : char* value               ����ֵ                          */
/*                  char* split_result_pos    ����                            */
/* Output         : none                                                      */
/* Return         : true   ����ƥ��ɹ�                                       */
/*                  false  ����ƥ��ʧ��                                       */
/******************************************************************************/
bool CAnalyser::ConditionCheck(char* attrId, int vType, const char* curValue, char* strConditions)//ֻ֧�֡��롱
{
	bool ret = true; 
	char* savePtr = NULL;
	char* split_result = NULL;
	split_result = strtok_r(strConditions, "@", &savePtr);
	DBG_ANA(("ConditionCheck   attrId[%s] vType[%d] curValue[%s] condition[%s]\n", attrId, vType, curValue, strConditions));
	while(split_result != NULL)
	{
		if(!IsTrue(attrId, vType, curValue, split_result))
		{
			ret = false;
			break;
		}
		split_result = strtok_r(NULL, "@", &savePtr);
	}
	return ret;
}

/******************************************************************************/
bool CAnalyser::IsTrue(char* pAttrId, int vType, const char* curValue, char* expression)
{
	bool ret = false;
	char compareFlag = 0;
	char* pPperator = NULL;
	char szPre[128] = {0};
	char szNext[128] = {0};

	DBG_ANA(("IsTrue   attrId[%s] vType[%d] curValue[%s] expression[%s]\n", pAttrId, vType, curValue, expression));
	if(NULL != (pPperator = strstr(expression, "<")))
	{
		memcpy(szPre, expression, pPperator - expression);
		strcpy(szNext,pPperator + 1);
		compareFlag = 1;
	}
	else if (NULL != (pPperator = strstr(expression, ">")))
	{
		memcpy(szPre, expression, pPperator - expression);
		strcpy(szNext,pPperator + 1);
		compareFlag = 2;
	}
	else if (NULL != (pPperator = strstr(expression, "=")))
	{
		memcpy(szPre, expression, pPperator - expression);
		strcpy(szNext,pPperator + 1);
		compareFlag = 3;
	}
	else if (NULL != (pPperator = strstr(expression, "!=")))
	{
		memcpy(szPre, expression, pPperator - expression);
		strcpy(szNext,pPperator + 2);
		compareFlag = 4;
	}
	else
	{
		return ret;
	}

	//�����������ʽ�ȽϷ������ֵ
	DBG_ANA(("IsTrue   szPre[%s] szNext[%s] compareFlag[%d]\n", szPre, szNext,compareFlag));
	char tempCalFunc[256] = {0};
	float fRetPre = 0.0;
	transPolish1(szPre, tempCalFunc);//ת��Ϊ�沨��ʽ
	DBG_ANA(("IsTrue after transPolish1 11 tempCalFunc[%s] curValue[%s]\n", tempCalFunc, curValue));
	int cacuRetPre = compvalue1(pAttrId, vType, curValue, tempCalFunc, fRetPre);
	DBG_ANA(("IsTrue after compvalue1 Pre cacuRetPre[%d] fRetPre[%f]\n", cacuRetPre, fRetPre));

	//�����������ʽ�ȽϷ��ұ���ֵ
	float fRetNext = 0.0;
	memset(tempCalFunc, 0, sizeof(tempCalFunc));
	transPolish1(szNext, tempCalFunc);
	DBG_ANA(("IsTrue after transPolish1 22 tempCalFunc[%s] \n", tempCalFunc));
	int cacuRetNext = compvalue1(pAttrId, vType, curValue, tempCalFunc, fRetNext);
	DBG_ANA(("IsTrue after compvalue1 Next cacuRetNext[%d] fRetNext[%f]\n", cacuRetNext, fRetNext));

	if (SYS_STATUS_SUCCESS == cacuRetPre && SYS_STATUS_SUCCESS == cacuRetNext)//������ֵ����
	{
		DBG_ANA(("IsTrue after ������ֵ���� pre[%f] next[%f]\n", fRetPre, fRetNext));
		switch(compareFlag)
		{
		case 1:
			if (fRetPre < fRetNext)
			{
				ret = true;
			}
			break;
		case 2:
			if (fRetPre > fRetNext)
			{
				ret = true;
			}
			break;
		case 3:
			if (fRetPre == fRetNext)
			{
				ret = true;
			}
			break;
		case 4:
			if (fRetPre != fRetNext)
			{
				ret = true;
			}
			break;
		}
	}
	else if(SYS_STATUS_CACULATE_MODE_ERROR == cacuRetPre || SYS_STATUS_CACULATE_MODE_ERROR == cacuRetNext)//�ַ�������
	{
		char szPreValue[512] = {0};
		char szNextValue[512] = {0};

		DBG_ANA(("IsTrue after ������ֵ���㣬Ĭ��ʹ���ַ����Ƚ� pre[%s] next[%s]\n", szPre, szNext));
		if(IsAttrId(szPre))
		{
			char* ptrAttrId = szPre + 1;
			if (0 == strncmp(ptrAttrId, pAttrId, DEVICE_ATTR_ID_LEN))//��������
			{
				if (CHARS == vType)
				{
					memset(szPreValue, 0, sizeof(szPreValue));
					strcpy(szPreValue, curValue);
				}
			}
			else
			{
				LAST_VALUE_INFO stLastValue;
				if(SYS_STATUS_SUCCESS == g_DevInfoContainer.GetAttrValue(ptrAttrId, stLastValue))
				{
					memset(szPreValue, 0, sizeof(szPreValue));
					strcpy(szPreValue, stLastValue.Value);
				}
				else
				{
					DBG_ANA(("IsTrue Pre�޷��ҵ�[%s]��Ӧ�Ľ��\n",ptrAttrId));
				}
			}
			DBG_ANA(("IsTrue Pre�����Ա�ţ�����ֵΪ[%s]\n",szPreValue));
			if (0 == strncmp(szPreValue, "NULL", 4))
			{
				compareFlag = 3;
			}
		}
		else
		{
			strcpy(szPreValue, szPre);
		}

		if(IsAttrId(szNext))
		{
			char* ptrAttrId = szNext + 1;
			if (0 == strncmp(ptrAttrId, pAttrId, DEVICE_ATTR_ID_LEN))//��������
			{
				if (CHARS == vType)
				{
					memset(szNextValue, 0, sizeof(szNextValue));
					strcpy(szNextValue, curValue);
				}
			}
			else
			{
				LAST_VALUE_INFO stLastValue;
				if(SYS_STATUS_SUCCESS == g_DevInfoContainer.GetAttrValue(ptrAttrId, stLastValue))
				{
					memset(szNextValue, 0, sizeof(szNextValue));
					strcpy(szNextValue, stLastValue.Value);
				}
				else
				{
					DBG_ANA(("IsTrue Next�޷��ҵ�[%s]��Ӧ�Ľ��\n",ptrAttrId));
				}
			}
			DBG_ANA(("IsTrue Next�����Ա�ţ�����ֵΪ[%s]\n",szNextValue));

			if (0 == strncmp(szPreValue, "NULL", 4))
			{
				compareFlag = 3;
			}
		}
		else
		{
			strcpy(szNextValue, szNext);
		}

		DBG_ANA(("IsTrue ת����LastPre[%s] LastNext[%s]\n",szPreValue, szNextValue));
		switch(compareFlag)
		{
		case 1:
			if (0 > strcmp(szPreValue, szNextValue))
			{
				ret = true;
			}
			break;
		case 2:
			if (0 < strcmp(szPreValue, szNextValue))
			{
				ret = true;
			}
			break;
		case 3:
			if (0 == strcmp(szPreValue, szNextValue))
			{
				ret = true;
			}
			break;
		case 4:
			if (0 != strcmp(szPreValue, szNextValue))
			{
				ret = true;
			}
			break;
		}
	}
	return ret;
}

/******************************************************************************/
/* Function Name  : transPolish                                               */
/* Description    : ����ʽת�����沨��ʽ                                      */
/* Input          : szRdCalFunc     ����ʽ                                    */
/* Output         : tempCalFunc     �沨��ʽ                                  */
/* Return         :                                                           */
/******************************************************************************/
void CAnalyser::transPolish1(char* szRdCalFunc, char* tempCalFunc)
{
	char stack[128] = {0};
	char ch;
	int i = 0,t = 0,top = 0;

	ch = szRdCalFunc[i];
	i++;

	while( ch != 0)          //����ʽ����
	{
		switch( ch )
		{
		case '+':
		case '-': 
			while(top != 0 && stack[top] != '(')
			{
				tempCalFunc[t] = stack[top];
				top--;
				t++;
			}
			top++;
			stack[top] = ch;
			break;
		case '*':
		case '/':
			while(stack[top] == '*'|| stack[top] == '/')
			{
				tempCalFunc[t] = stack[top];
				top--;
				t++;
			}
			top++;
			stack[top] = ch;
			break;
		case '(':
			top++;
			stack[top] = ch;
			break;
		case ')':
			while( stack[top]!= '(' )
			{
				tempCalFunc[t] = stack[top];
				top--;
				t++;
			}
			top--;
			break;
		case ' ':break;
		default:

			while( isdigit(ch) || ch == '.' || IsAlp(ch))
			{
				tempCalFunc[t] = ch;
				t++;
				ch = szRdCalFunc[i];
				i++;
			}
			i--;
			tempCalFunc[t] = ' ';	//����֮��Ҫ�ӿո��Ա���ͺ��������������һ���޷���ȷʶ��������λ��
			t++;
		}
		ch = szRdCalFunc[i];
		i++;

		DBG_ANA(("tempCalFunc[%s]\n", tempCalFunc));
	}
	while(top!= 0)
	{
		tempCalFunc[t] = stack[top];
		t++;
		top--;
	}
	tempCalFunc[t] = ' ';
}
bool CAnalyser::IsAlp(char c)
{
	return ((c <= 'z' && c >= 'a') || (c >= 'A' && c <= 'Z'));
}

bool CAnalyser::IsDigit(char* ch)
{
	bool ret = true;
	for(int i=0; i<(int)strlen(ch); i++)
	{
		if(!isdigit(ch[i]) && ch[i] != '.')
		{
			ret =false;
			break;
		}
	}
	return ret;
}
bool CAnalyser::IsAttrId(char* ch)
{
	return 'P' == ch[0] && (DEVICE_ATTR_ID_LEN + 1) == strlen(ch) && IsDigit(ch + 1);
}


/******************************************************************************/
/* Function Name  : compvalue                                                 */
/* Description    : �沨��ʽ������                                          */
/* Input          : tempCalFunc     �沨��ʽ                                  */
/*                  midResult       ��������ֵ                                */
/* Output         :                                                           */
/* Return         : ������                                                  */
/******************************************************************************/
int CAnalyser::compvalue1(char* pAttrId, int vType, const char* curValue, char* tempCalFunc, float& fRsult)
{
	float stack[20];
	char ch = 0;
	char str[100] = {0};
	int i = 0, t = 0,top = 0;
	ch = tempCalFunc[t];
	t++;

	while(ch!= ' ')
	{
		switch(ch)
		{
		case '+':
			stack[top-1] = stack[top-1] + stack[top];
			top--;
			break;
		case '-':
			stack[top-1] = stack[top-1] - stack[top];
			top--;
			break;
		case '*':
			stack[top-1] = stack[top-1] * stack[top];
			top--;
			break; 
		case '/':
			if(stack[top]!= 0)
				stack[top-1] = stack[top-1]/stack[top];
			else
			{
				DBG_ANA(("�������\n"));
				return SYS_STATUS_FORMAT_ERROR;
			}
			top--;
			break;
		default:
			i = 0;
			while( isdigit(ch) || ch == '.' || IsAlp(ch))
			{
				str[i] = ch;
				i++;
				ch = tempCalFunc[t];
				t++;
			}
			str[i] = '\0';
			if (IsAttrId(str))//�ҵ����Խ��
			{
				DBG_ANA(("compvalue1 �����Խ��[%s] ʵ������AttrId[%s]\n", str, pAttrId));
				if (0 == strcmp(str+1, pAttrId))//��������
				{
					if (CHARS == vType)
					{
						return SYS_STATUS_CACULATE_MODE_ERROR;
					}
					top++;
					stack[top] = atof(curValue);
				}
				else//��������
				{
					DEVICE_INFO_POLL devicePoll;
					if (SYS_STATUS_SUCCESS != g_DevInfoContainer.GetAttrNode(str+1, devicePoll))
					{
						return SYS_STATUS_DEVICE_NOT_EXIST;
					}
					if (IsDigit(devicePoll.stLastValueInfo.Value) && FLOAT == devicePoll.stLastValueInfo.VType)//��������ֵ����������Ϊ������
					{
						top++;
						stack[top] = atof(devicePoll.stLastValueInfo.Value);
					}
					else
					{
						return SYS_STATUS_CACULATE_MODE_ERROR;
					}
				}
			}
			else if (IsDigit(str))//��ʵ����ֵ
			{
				DBG_ANA(("compvalue1 �ǳ���[%s]\n", str));
				top++;
				stack[top] = atof(str);
			}
			else
			{
				DBG_ANA(("compvalue1 ����ʧ��[%s]\n", str));
				return SYS_STATUS_CACULATE_MODE_ERROR;
			}

		}
		ch = tempCalFunc[t];
		t++;
	}
	fRsult = stack[top];
	return SYS_STATUS_SUCCESS;
}

/******************************************************************************/
/* Function Name  : conditionCheck                                            */
/* Description    : ����ƥ��                                                  */
/* Input          : char* value               ����ֵ                          */
/*                  char* split_result_pos    ����                            */
/* Output         : none                                                      */
/* Return         : true   ����ƥ��ɹ�                                       */
/*                  false  ����ƥ��ʧ��                                       */
/******************************************************************************/
bool CAnalyser::conditionCheck(int vType, const char* value, char* split_result_pos)
{
    bool ret = true;
    const char* split = ",";
    char* split_result = NULL;
    int recValue_interg = 0;           //��ѯȡ�õ�ֵ
    double recValue_doub = 0;
    int conValue_interg = 0;           //��׼ֵ
    double conValue_doub = 0;

    if( vType == INTEGER )
    {
        recValue_interg = atoi(value);
        //DBG_ANA(("RecValue:[%d]", recValue_interg));
    }
    else if( vType == FLOAT )
    {
        recValue_doub = atof(value);
        //DBG_ANA(("RecValue:[%f]", recValue_doub));
    }

	char* savePtr = NULL;
    split_result = strtok_r(split_result_pos, ",", &savePtr);
    while(split_result != NULL)
    {
        //DBG_ANA(("Condition:[%s]\n", split_result));
        if( *split_result == '*' )        //���ֽ�Ϊ *,��ʾ�κ�ֵ��ʹ��������
        {
            //DBG_ANA(("No Condition\n"));
            ret = true;
            return ret;
        }
        else if( *split_result == '<' )
        {
            if( vType == INTEGER )
            {
                conValue_interg = atoi(split_result+1);
                //DBG_ANA(("convalue:[%d]\n", conValue_interg));
                if( recValue_interg < conValue_interg ){    ret = true;  }
                else{    return false;  }    //�κ����������㣬��Ϊ����������
            }
            else if( vType == FLOAT )
            {
                conValue_doub = atof(split_result+1);
                //DBG_ANA(("convalue <:[%f]\n", conValue_doub));
                if( recValue_doub < conValue_doub ){    ret = true;  }
                else{    return false;  }
            }
        }
        else if( *split_result == '>' )
        {
            if( vType == INTEGER )
            {
                conValue_interg = atoi(split_result+1);
                DBG_ANA(("convalue:[%d]\n", conValue_interg));
                if( recValue_interg > conValue_interg ){    ret = true;  }
                else{    return false;  }
            }
            else if( vType == FLOAT )
            {
                conValue_doub = atof(split_result+1);
                DBG_ANA(("convalue >:[%f]\n", conValue_doub));
                if( recValue_doub > conValue_doub ){    ret = true;  }
                else{    return false;  }
            }
        }
        else if( *split_result == '=' )
        {
            if( vType == INTEGER )
            {
                conValue_interg = atoi(split_result+1);
                //DBG_ANA(("convalue:[%d]\n", conValue_interg));
                if( recValue_interg == conValue_interg ){    ret = true;  }
                else{    return false;  }
            }
            else if( vType == FLOAT )
            {
                conValue_doub = atof(split_result+1);
                //DBG_ANA(("convalue =:[%f]\n", conValue_doub));
                if( recValue_doub == conValue_doub ){    ret = true;  }
                else{    return false;  }
            }
        }

        split_result = strtok_r(NULL, split, &savePtr);
    }
    return ret;
}


/******************************************************************************/
/* Function Name  : addNode                                                   */
/* Description    : ���豸���������ڵ�                                      */
/* Input          : MSG_ACT_HASH hash_node      �豸�ڵ�                      */
/* Output         : none                                                      */
/* Return         : true   �ڵ��Ѳ���                                         */
/*                  false  �ڵ��Ѹ���                                         */
/******************************************************************************/
bool CAnalyser::addNode(MSG_ANALYSE_HASH hash_node)
{
    int ret = false;
    MSG_ANALYSE_HASH msg_node;
    HashedInfo::iterator it;

    memset((BYTE*)&msg_node, 0, sizeof(msg_node));
    memcpy((BYTE*)&msg_node, (BYTE*)&hash_node, sizeof(MSG_ANALYSE_HASH));

    CTblLock.lock();
    it = m_pActHashTable->find(msg_node.key);
    if (it == m_pActHashTable->end())
    {		
  		m_pActHashTable->insert(HashedInfo::value_type(msg_node.key,msg_node));
		ret = true;
	}
    CTblLock.unlock();
    return ret;
}


/******************************************************************************/
/* Function Name  : delNode                                                   */
/* Description    : ���豸��������ɾ���ڵ�                                    */
/* Input          : MSG_ACT_HASH hash_node      �豸�ڵ�                      */
/* Output         : none                                                      */
/* Return         : true   �ڵ�ɾ���ɹ�                                       */
/*                  false  �ڵ�ɾ��ʧ��                                       */
/******************************************************************************/
bool CAnalyser::delNode(MSG_ANALYSE_HASH hash_node)
{
    HashedInfo::iterator it;
	CTblLock.lock();
    it = m_pActHashTable->find(hash_node.key);	
	if (it != m_pActHashTable->end())
	{
		m_pActHashTable->erase(it);
	}
	CTblLock.unlock();
	return true;
}


/******************************************************************************/
/* Function Name  : updateNodePara                                            */
/* Description    : �����豸������ڵ�                                        */
/* Input          : MSG_ACT_HASH hash_node      �豸�ڵ�                      */
/* Output         : none                                                      */
/* Return         : true   �ڵ����                                           */
/*                  false  �ڵ�����                                           */
/******************************************************************************/
bool CAnalyser::updateNodePara(MSG_ANALYSE_HASH hash_node, int paraId)
{
	bool ret = false;
	HashedInfo::iterator it;
	CTblLock.lock();
	it = m_pActHashTable->find(hash_node.key);
	if (it != m_pActHashTable->end())
	{
		PMSG_ANALYSE_HASH pNode = (PMSG_ANALYSE_HASH)(&it->second.key);
		switch (paraId)
		{
		case 1://UnionLastValue
			memset((unsigned char*)&pNode->unionLastValue, 0, sizeof(pNode->unionLastValue));
			memcpy((unsigned char*)&pNode->unionLastValue, (unsigned char*)&hash_node.unionLastValue, sizeof( hash_node.unionLastValue));
			break;
		case 2://LastTime
			memset((unsigned char*)&pNode->LastTime, 0, sizeof(pNode->LastTime));
			memcpy((unsigned char*)&pNode->LastTime, (unsigned char*)&hash_node.LastTime, sizeof( hash_node.LastTime));
			break;
		case 3://ActFormat,����ʱ�����
			ACT_FORMAT act_format;
			for(int i=0; i<MAX_CNT_ACTFORMAT; i++)
			{				
				memset((unsigned char*)&act_format, 0, sizeof(ACT_FORMAT));
				act_format.iSeq = pNode->agentSeq[i];
				HashedAgentInfo::iterator agentIt = m_pAgentHashTable->find(act_format.iSeq);
				if (agentIt != m_pAgentHashTable->end())
				{
					PACT_FORMAT pNode = (PACT_FORMAT)(&agentIt->second.iSeq);
					pNode->time_lastTime = pNode->time_lastTime;
				}
			}
			break;
		case 4://��������״̬
			pNode->dataStatus = hash_node.dataStatus;
			break;
		}
		ret = true;
	}
	CTblLock.unlock();
	return ret;
}

/******************************************************************************/
/* Function Name  : searchNode                                                */
/* Description    : �ڹ�ϣ������ҽڵ�                                        */
/* Input          : MSG_ACT_HASH hash_node      �豸�ڵ�                      */
/* Output         : none                                                      */
/* Return         : true   ���ҵ��ڵ�                                         */
/*                  false  δ�鵽�ڵ�                                         */
/******************************************************************************/
bool CAnalyser::searchNode(MSG_ANALYSE_HASH hash_node)
{
    bool ret = false;
    HashedInfo::iterator it;
	CTblLock.lock();
    //DBG_ANA(("hashnode.key:[%s]\n", hash_node.key));
    it = m_pActHashTable->find(hash_node.key);	
	if (it != m_pActHashTable->end())
	{
		ret = true;
	}
	CTblLock.unlock();
	return ret;
}

/******************************************************************************/
/* Function Name  : GetHashData                                               */
/* Description    : �ӹ�ϣ����Ѱ�ҽڵ㲢ȡ������                              */
/* Input          : char* deviceId       �豸���                             */
/*                  char* ctype          ��������                             */
/*                  char* value          ����ֵ                               */
/*                  char* a_format       ����Э��                             */
/* Output         : none                                                      */
/* Return         : true                                                      */
/*                  false                                                     */
/******************************************************************************/
bool CAnalyser::GetHashData(MSG_ANALYSE_HASH& hash_node)
{
    bool ret = false;
    HashedInfo::iterator it;
	CTblLock.lock();
    it = m_pActHashTable->find(hash_node.key);
    if (it != m_pActHashTable->end())
	{
        memcpy((BYTE*)&hash_node, (BYTE*)&it->second, sizeof(MSG_ANALYSE_HASH));
        ret = true;
    }
	CTblLock.unlock();
    return ret;
}


//�豸״̬�����仯ʱ��������sn����仯
bool CAnalyser::deviceStatusUpdate(char* devId, int status)
{
	bool ret = false;
    CTblLock.lock();
    HashedInfo::iterator it;
    for (it = m_pActHashTable->begin(); it != m_pActHashTable->end(); it++)
	{
        if( 0 == memcmp(it->second.key, devId, DEVICE_ID_LEN))
        {
            ((PMSG_ANALYSE_HASH)(&it->second))->deviceStatus = status;
			ret = true;
        }
    }
    CTblLock.unlock();
    return ret;
}

/******************************************************************************/
/* Function Name  : AnalyseHashAgentCheck                                   */
/* Description    : �����豸��������������                                    */
/* Input          : device_attr_id �豸���					                      */
/*		          : actions ��������					                      */
/* Output         : none                                                      */
/* Return         : true   �ڵ����                                           */
/*                  false  �ڵ�����                                           */
/******************************************************************************/
int CAnalyser::AnalyseHashAgentCheck( AgentInfo stAgentInfo)
{
	int ret = SYS_STATUS_FAILED;
	HashedInfo::iterator it;
	MSG_ANALYSE_HASH hash_node;
	memset((unsigned char*)&hash_node, 0, sizeof(MSG_ANALYSE_HASH));
	CTblLock.lock();
	
	DBG_ANA(("����豸�������[%s] Seq[%d]\n", stAgentInfo.szDevId,stAgentInfo.iSeq));
	strcpy(hash_node.key, stAgentInfo.szDevId);
	if ('1' == stAgentInfo.cType)//��������
	{		
		ret = SYS_STATUS_SUCCESS;
	}
	else//		
	{
		strncpy(hash_node.key + DEVICE_ID_LEN, stAgentInfo.szAttrId, DEVICE_SN_LEN);
		it = m_pActHashTable->find(hash_node.key);
		if (it != m_pActHashTable->end())
		{
			PMSG_ANALYSE_HASH curr_node = (PMSG_ANALYSE_HASH)(&it->second.key);
			if (curr_node->actCnt < MAX_CNT_ACTFORMAT)
			{
				ret = SYS_STATUS_SUCCESS;
				DBG_ANA(("��������ɹ� ��ǰ[%s]��������[%d]\n", curr_node->key, curr_node->actCnt));
			}
			else
			{
				ret = SYS_STATUS_DEVICE_ANALYSE_ACTION_FULL;
				DBG_ANA(("AnalyseHashAgentAdd  ������������ ��ǰ[%s]��������[%d]\n", curr_node->key, curr_node->actCnt));
			}
		}	
	}
	CTblLock.unlock();
	return ret;
}

/******************************************************************************/
/* Function Name  : AnalyseHashAgentAdd                                    */
/* Description    : �����豸��������������                                        */
/* Input          : device_attr_id �豸���					                      */
/*		          : actions ��������					                      */
/* Output         : none                                                      */
/* Return         : true   �ڵ����                                           */
/*                  false  �ڵ�����                                           */
/******************************************************************************/
int CAnalyser::AnalyseHashAgentAdd( AgentInfo stAgentInfo)
{
	int ret = SYS_STATUS_FAILED;
	HashedInfo::iterator it;
	MSG_ANALYSE_HASH hash_node;
	memset((unsigned char*)&hash_node, 0, sizeof(MSG_ANALYSE_HASH));
	CTblLock.lock();
	DBG_ANA(("����豸�������[%s] Seq[%d]\n", stAgentInfo.szDevId,stAgentInfo.iSeq));
	strcpy(hash_node.key, stAgentInfo.szDevId);
	if ('1' == stAgentInfo.cType)//��������
	{
		strncpy(hash_node.key + DEVICE_ID_LEN, "0000", DEVICE_SN_LEN);
		it = m_pActHashTable->find(hash_node.key);
		if (it == m_pActHashTable->end())
		{
			memset((unsigned char*)&hash_node.agentSeq[0], 0, sizeof(int) * MAX_CNT_ACTFORMAT);
			hash_node.actCnt = 0;
			m_pActHashTable->insert(HashedInfo::value_type(hash_node.key,hash_node));
			DBG_ANA(("����豸�����������[%s]\n", hash_node.key));
		}
	}
	else//
		strncpy(hash_node.key + DEVICE_ID_LEN, stAgentInfo.szAttrId, DEVICE_SN_LEN);

	
	it = m_pActHashTable->find(hash_node.key);
	if (it != m_pActHashTable->end())
	{
		PMSG_ANALYSE_HASH curr_node = (PMSG_ANALYSE_HASH)(&it->second.key);
		if (curr_node->actCnt < MAX_CNT_ACTFORMAT)
		{
			getAgentMsg(&curr_node->agentSeq[curr_node->actCnt], stAgentInfo);
			curr_node->actCnt++;
			ret = SYS_STATUS_SUCCESS;
			DBG_ANA(("��������ɹ� ��ǰ[%s]SEQ[%d]��������[%d]\n", curr_node->key, curr_node->agentSeq[curr_node->actCnt-1],curr_node->actCnt));
		}
		else
		{
			ret = SYS_STATUS_DEVICE_ANALYSE_ACTION_FULL;
			DBG_ANA(("AnalyseHashAgentAdd  ������������ ��ǰ[%s]��������[%d]\n", curr_node->key, curr_node->actCnt));
		}
	}	
	else
	{
		DBG_ANA(("δ�ҵ���Ӧ���豸���[%s]\n", hash_node.key));
		ret = SYS_STATUS_DEVICE_ANALYSE_NOT_EXIST;
	}
	CTblLock.unlock();
	return ret;
}


/******************************************************************************/
/* Function Name  : AnalyseHashAgentUpdate                                    */
/* Description    : �����豸��������������                                        */
/* Input          : device_attr_id �豸���					                      */
/*		          : actions ��������					                      */
/* Output         : none                                                      */
/* Return         : true   �ڵ����                                           */
/*                  false  �ڵ�����                                           */
/******************************************************************************/
int CAnalyser::AnalyseHashAgentUpdate( AgentInfo stAgentInfo)
{
	int ret = SYS_STATUS_FAILED;
	HashedInfo::iterator it;
	MSG_ANALYSE_HASH hash_node;
	memset((unsigned char*)&hash_node, 0, sizeof(MSG_ANALYSE_HASH));
	strncpy(hash_node.key, stAgentInfo.szDevId, DEVICE_ID_LEN);

	strncpy(hash_node.key + DEVICE_ID_LEN, "0000", DEVICE_SN_LEN);
	if (0 < strlen(stAgentInfo.szAttrId) && 0 != strncmp(stAgentInfo.szAttrId, "    ", DEVICE_SN_LEN))
	{
		strncpy(hash_node.key + DEVICE_ID_LEN, stAgentInfo.szAttrId, DEVICE_SN_LEN);
	}

	CTblLock.lock();
	it = m_pActHashTable->find(hash_node.key);
	if (it != m_pActHashTable->end())
	{
		PMSG_ANALYSE_HASH curr_node = (PMSG_ANALYSE_HASH)(&it->second.key);
		for(int i=0; i<curr_node->actCnt; i++)
		{
			if (stAgentInfo.iSeq == curr_node->agentSeq[i])
			{
				getAgentMsg(&curr_node->agentSeq[i], stAgentInfo);
				ret = SYS_STATUS_SUCCESS;
			}
		}
	}	
	CTblLock.unlock();
	return ret;
}

/******************************************************************************/
/* Function Name  : AnalyseHashAgentDelete                                    */
/* Description    : �����豸��������������                                     */
/* Input          : device_attr_id �豸���					                      */
/*		          : actions ��������					                      */
/* Output         : none                                                      */
/* Return         : true   �ڵ����                                           */
/*                  false  �ڵ�����                                           */
/******************************************************************************/
bool CAnalyser::AnalyseHashAgentDelete( AgentInfo stAgentInfo)
{
	bool ret = false;
	HashedInfo::iterator it;
	MSG_ANALYSE_HASH hash_node;
	memset((unsigned char*)&hash_node, 0, sizeof(MSG_ANALYSE_HASH));
	strncpy(hash_node.key, stAgentInfo.szDevId, DEVICE_ID_LEN);
	strncpy(hash_node.key + DEVICE_ID_LEN, "0000", DEVICE_SN_LEN);
	if (0 < strlen(stAgentInfo.szAttrId) && 0 != strncmp(stAgentInfo.szAttrId, "    ", DEVICE_SN_LEN))
	{
		strncpy(hash_node.key + DEVICE_ID_LEN, stAgentInfo.szAttrId, DEVICE_SN_LEN);
	}

	DBG_ANA(("AnalyseHashAgentDelete ����������ɾ��[%s]\n", hash_node.key));
	CTblLock.lock();
	
	DBG_ANA(("AnalyseHashAgentDelete ...........111111\n"));
	it = m_pActHashTable->find(hash_node.key);
	if (it != m_pActHashTable->end())
	{
		int index = -1;
		PMSG_ANALYSE_HASH curr_node = (PMSG_ANALYSE_HASH)(&it->second.key);
		DBG_ANA(("AnalyseHashAgentDelete ...........2222222\n"));
		for(int i=0; i<curr_node->actCnt; i++)
		{
			DBG_ANA(("AnalyseHashAgentDelete iSeq[%d] oldSeq[%d]\n", stAgentInfo.iSeq, curr_node->agentSeq[i]));
			if (stAgentInfo.iSeq == curr_node->agentSeq[i])
			{
				index = i;
				break;
			}
		}

		DBG_ANA(("AnalyseHashAgentDelete ...........333333\n"));
		if (0 <= index)//lhy ���
		{
			curr_node->agentSeq[index] = 0;
			memcpy((unsigned char*)&curr_node->agentSeq[index], (unsigned char*)&curr_node->agentSeq[index + 1], (curr_node->actCnt - index) * sizeof(int));
			curr_node->actCnt--;
			DBG_ANA(("ɾ�������ɹ� ��ǰ[%s]��������[%d]\n", curr_node->key, curr_node->actCnt));
			
			//ɾ��agentInfo���м�¼lhy����
			HashedAgentInfo::iterator agentIt;
			ACT_FORMAT act_format;
			act_format.iSeq = curr_node->agentSeq[index];
			agentIt = m_pAgentHashTable->find((int)act_format.iSeq);	
			if (agentIt != m_pAgentHashTable->end())
			{
				m_pAgentHashTable->erase(agentIt);
			}
			ret = true;
		}
	}	
	else
	{
		DBG_ANA(("ɾ���������[%s]δ�ҵ�\n", hash_node.key));
	}
	
	DBG_ANA(("AnalyseHashAgentDelete ...........444444\n"));
	CTblLock.unlock();
	return ret;
}

/******************************************************************************/
/* Function Name  : AnalyseHashDeleteByDeviceAttr                                    */
/* Description    : �����豸������ڵ�                                        */
/* Input          : device_attr_id �豸���					                      */
/*		          : standard ��׼��Χ					                      */
/* Output         : none                                                      */
/* Return         : true   �ڵ����                                           */
/*                  false  �ڵ�����                                           */
/******************************************************************************/
bool CAnalyser::AnalyseHashDeleteByDeviceAttr( char* device_attr_id)
{
	HashedInfo::iterator it;
	MSG_ANALYSE_HASH hash_node;
	memset((unsigned char*)&hash_node, 0, sizeof(MSG_ANALYSE_HASH));
	strncpy(hash_node.key, device_attr_id, DEVICE_ATTR_ID_LEN);
	CTblLock.lock();
	it = m_pActHashTable->find(hash_node.key);	
	if (it != m_pActHashTable->end())
	{
		m_pActHashTable->erase(it);
	}
	CTblLock.unlock();
	return true;
}


/******************************************************************************/
/* Function Name  : AnalyseHashDeleteByNet                                    */
/* Description    : ɾ���豸������ڵ�                                        */
/* Input          : device_id �豸���					                      */
/*		          : apptype Ӧ������					                      */
/* Output         : none                                                      */
/* Return         : true   �ڵ����                                           */
/*                  false  �ڵ�����                                           */
/******************************************************************************/
bool CAnalyser::AnalyseHashDeleteByNet( char* device_id)
{

	bool ret = false;
	HashedInfo::iterator it;
	PMSG_ANALYSE_HASH device_node;	
	char keys[256][16];
	memset(&keys[0][0], 0, 256*16);
	int i = 0;
	CTblLock.lock();

	for (it = m_pActHashTable->begin(); it != m_pActHashTable->end(); it++)
	{
		device_node = (PMSG_ANALYSE_HASH)(&it->second.key);

		if (0 != strncmp(device_node->key, device_id, DEVICE_ID_LEN))
		{
			continue;
		}
		strcpy(&(keys[i][0]), device_node->key);
		i++;
	}
	MSG_ANALYSE_HASH hashnode;
	for(int j=0; j<=i; j++)
	{
		memset((unsigned char*)&hashnode, 0, sizeof(DEVICE_INFO_POLL));
		strcpy(hashnode.key, &(keys[i][0]));
		it = m_pActHashTable->find(hashnode.key);	
		if (it != m_pActHashTable->end())
		{
			m_pActHashTable->erase(it);
		}
	}
	CTblLock.unlock();
	ret = true;
	return ret;
}

/******************************************************************************/
/* Function Name  : InsertATSTable                                            */
/* Description    : ���뵽���ڼ�¼��                                          */
/* Input          : int   vType          ��ֵ����                             */
/*                  char* value          ����ֵ                               */
/*                  char* standard       ����ֵ��Χ                           */
/* Output         : none                                                      */
/* Return         : true       ������������Χ��                               */
/*                  false      ������������Χ��                               */
/******************************************************************************/
void CAnalyser::InsertATSTable(MSG_ANALYSE_HASH hash_node, char* szTableName, char* time, char* szValue , char* pUserId)
{
	char sqlData[512] = {0};
	char szDeviceId[DEVICE_ID_LEN_TEMP] = {0};
	char szAttrSn[DEVICE_SN_LEN_TEMP] = {0};

	memset(szDeviceId, 0, sizeof(szDeviceId));
	memset(szAttrSn, 0, sizeof(szAttrSn));

	memcpy(szDeviceId, hash_node.key, DEVICE_ID_LEN);
	memcpy(szAttrSn,   hash_node.key + DEVICE_ID_LEN, DEVICE_SN_LEN);

	strcat(sqlData, "null, ");
	strcat(sqlData, "'");
	strcat(sqlData, szDeviceId);        //�豸���
	strcat(sqlData, "','");
	strcat(sqlData, szAttrSn);
	strcat(sqlData, "', datetime('");
	strcat(sqlData, time);         //�ɼ�ʱ��
	strcat(sqlData, "'), '");
	strcat(sqlData, szValue);        //�ɼ�����
	strcat(sqlData, "', '");
	strcat(sqlData, pUserId);		//����״̬
	strcat(sqlData, "'");         //����״̬
	g_SqlCtl.insert_into_table( szTableName, sqlData, false);               //��ȡ�����ݲ������ݿ�

}
/*******************************END OF FILE************************************/


