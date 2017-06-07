#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termio.h>
#include <unistd.h>
#include <locale.h>
#include <stdio.h>
#include "Init.h"
#include "SqlCtl.h"
#include "GSMCtrl.h"

#include "HardIF.h"

#define SMS_CONTENT_MAXLEN   (1024)
#define ONEDAY_SECONDS       (24*60*60)

#ifdef  DEBUG
#define DEBUG_GSMCTRL
#endif

#ifdef DEBUG_GSMCTRL
#define DBG_GSMCTRL(a)		printf a;
#else
#define DBG_GSMCTRL(a)	
#endif


CGSMCtrl::CGSMCtrl()
{
	m_SMSSendThrd = 0xFFFF;
	m_SeqCPM = 0;
}

CGSMCtrl::~CGSMCtrl(void)
{				
	if(0xFFFF != m_SMSSendThrd)
	{
		ThreadPool::Instance()->SetThreadMode(m_SMSSendThrd, false);
	}
}

/******************************************************************************/
// Function Name  : Initialize                                               
// Description    : ��ʼ��Siemens GSMģ��                                          
// Input          : none
// Output         : none                                                     
// Return         : true:�ɹ�;false:ʧ��                                                     
/******************************************************************************/
bool CGSMCtrl::Initialize(TKU32 unHandleSend)
{   
    m_ptrActInfoTable = new HashedActInfo();

	//����GSMģ��״̬����߳�
	char buf[256] = {0};
	sprintf(buf, "%s %s", "CGSMCtrl", "SMSCtrlThrd");
	if (!ThreadPool::Instance()->CreateThread(buf, &CGSMCtrl::SMSCtrlThrd, true,this))
	{
		DBG_GSMCTRL(("CGSMCtrlThrd Create failed!\n"));
		return false;
	}
	m_unHandleGSM  = unHandleSend;
	return true;
}

/******************************************************************************/
//Function Name  : SMSSendThrd                                               
//Description    :                                         
//Input          : void* this
//Output         : none                                                     
//Return         : none                                                   
/******************************************************************************/
void * CGSMCtrl::SMSCtrlThrd(void *arg)
{
	CGSMCtrl *_this = (CGSMCtrl *)arg;
	_this->SMSCtrlThrdProc();
	return NULL;
}

/******************************************************************************/
//Function Name  : CheckThrdProc                                               
//Description    :                                        
//Input          : none
//Output         : none                                                     
//Return         : none                                                   
/******************************************************************************/
void CGSMCtrl::SMSCtrlThrdProc(void)
{ 
	GSM_ACTION_HASH hash_node;

	m_SMSSendThrd = pthread_self();

	printf("CGSMCtrl::GSMCtrlThrdProc Start ... Pid[%d]\n", getpid());
	QUEUEELEMENT pMsg = NULL;
	while(1)
	{
		pthread_testcancel();
        time_t tm_NowTime;
		

		//------������ִ�г�ʱ���
		TKU32 seqBuf[256] = {0};
		int cnt = 0;
		HashedActInfo::iterator it;
		CTblLockGsmAction.lock();
		for (it = m_ptrActInfoTable->begin(); it != m_ptrActInfoTable->end(); it++)
		{
			time(&tm_NowTime);
			if( tm_NowTime - it->second.act_time >= 120000 )
			{
				seqBuf[cnt] = it->second.Seq;
				cnt++;
			}
		}
		CTblLockGsmAction.unlock();

		for( int i=0; i < cnt; i++ )
		{
			memset(&hash_node, 0, sizeof(hash_node));
			hash_node.Seq = seqBuf[i];

			//��ʱ��ֻ��ɾ��
			if( !GetAndDeleteHashData(hash_node))
			{
				continue;
			}
		}

		//------������Ϣ����
		do 
		{
			//�ȴ�GSMģ����ȡ�����������ݣ����ͽ�� & ���ն��ţ�
			
			GSM_MSG stGsmMsg;
			if(SYS_STATUS_SUCCESS == HardIF_GetGSMMsg(stGsmMsg))
			{
				//����У����봦�����
				if(!SendMsg_Ex(m_unHandleGSM, MUCB_DEV_GSM, (char*)(&stGsmMsg), sizeof(GSM_MSG)))
				{
					DBG_GSMCTRL(("DecodeRecvSMS SendMsg_Ex failed, handle[%d]\n", m_unHandleGSM));
				}

			}

			//GSM�������
			if( MMR_OK != g_MsgMgr.GetFirstMsg(m_unHandleGSM, pMsg, 100))
			{
                //δȡ����������
                HardIF_PLALightOFF();
				break;
			}

			if (NULL == pMsg)
			{
				break;
			}

            HardIF_PLALightON();
			
			PGSM_MSG pGsmMsg = (PGSM_MSG)pMsg;
			switch(pGsmMsg->iMsgType)
			{
			case 0://��GSM����
				{
					DBG(("GSM ���յ�����\n"));
					GsmRequestProc(&pGsmMsg->stSmsInfo);
				}
				break;
			case 1://���͵�GSM,��Ҫ��Ӧ
				{
					DBG_GSMCTRL(("CPM ���͵�GSMģ�飬��Ҫ�ظ�ִ��״̬ SrcId[%s] SrcSn[%s]\n", pGsmMsg->stAction.SrcId, pGsmMsg->stAction.AttrSn));		
					int actionRet = SYS_STATUS_FAILED;
					if( SYS_STATUS_SUCCESS != (actionRet = HardIF_DoGSMAction(pGsmMsg)))
					{
						ACTION_MSG stAction;
						memcpy((unsigned char*)&stAction, (unsigned char*)&pGsmMsg->stAction, sizeof(ACTION_MSG));
						CDevInfoContainer::SendToActionSource(stAction, actionRet);
					}
				}
				break;
			case 2://���͵�GSM,����Ҫ��Ӧ				
				HardIF_DoGSMAction(pGsmMsg);	//HardIF����
				break;
			case 3://����ִ�н���ظ�
				{
					DBG_GSMCTRL(("GSM���յ�����ִ�н��\n"));
					ActionResponseProc(&pGsmMsg->stActionRespMsg);
				}
				break;
			case 4:	//GSMBase���ͽ����������
				{
					ACTION_MSG stAction;
					memcpy((unsigned char*)&stAction, (unsigned char*)&pGsmMsg->stRespSMS, sizeof(ACTION_MSG));
					CDevInfoContainer::SendToActionSource(stAction, pGsmMsg->stRespSMS.Status);
				}
				break;
			default:
				DBG_GSMCTRL(("SMSCtrlThrdProc δ֪����\n"));
				break;
			}
		} while (false);
		if (NULL != pMsg)
		{
            MM_FREE(pMsg);
			pMsg = NULL;
		}		
		
		usleep( 1000 * 1000 );
	}

}

//���ݲ�����ŷ��Ͷ���
int CGSMCtrl::SendToSmsList(char* Tel, char* content)
{
	char newValue[1024] = {0};
	sprintf(newValue, "%s//%s", Tel, content);

	GSM_MSG stGsmMsg;
	memset((unsigned char*)&stGsmMsg, 0, sizeof(GSM_MSG));
	stGsmMsg.iMsgType = 2;//�����Ӧ
	stGsmMsg.stAction.ActionSource = ACTION_SOURCE_GSM;
	strncpy(stGsmMsg.stAction.DstId, GSM_SMS_DEV, DEVICE_TYPE_LEN);
	memset(stGsmMsg.stAction.ActionValue, 0, sizeof(stGsmMsg.stAction.ActionValue));
	strcpy(stGsmMsg.stAction.ActionValue, newValue);
	HardIF_DoGSMAction(&stGsmMsg);
	return 0;
}	

TKU32 CGSMCtrl::GetSeq()
{
	if(0xffffffff == m_SeqCPM)
		m_SeqCPM = 0;
	return m_SeqCPM++;
}


/******************************************************************************/
//Function Name  : GsmRequestProc                                               
//Description    : GSM��������                                          
//Input          : none
//Output         : none                                                     
//Return         : ���ھ���� -1Ϊʧ��                                                     
/******************************************************************************/
/*
		���ݲ�ѯ���塰1#�豸��š�
		������ʽ���塰2#�豸���#�������#����ֵ���������豸���࣬����ֵ��Ϊ��
		��ݶ���:
			��0�����г�Ȩ�޷�Χ�ڵ��豸���������ݸ�ʽ���豸���,�豸����:�������Ա��,��������;...��
			��1����ȫ��Ȩ�޷�Χ�ڵ��豸������
			��2����ȫ��Ȩ�޷�Χ�ڵ��豸������
*/
int CGSMCtrl::GsmRequestProc(SMS_INFO* pSms)
{   
	int ret = 3006;
	char* split_result = NULL;
	char* savePtr = NULL;

	do 
	{
		DBG_GSMCTRL(("Tel[%s] Content[%s] Index[%d]\n", pSms->tel, pSms->content, pSms->index));
		HardIF_DeleteSms(pSms->index);

		char tel[20] = {0};
		if ( '+' == pSms->tel[0])
		{
			memcpy(tel, &pSms->tel[3], strlen(pSms->tel) - 3);
		}
		else if('8' == pSms->tel[0] && '6' == pSms->tel[1])
		{
			memcpy(tel, &pSms->tel[2], strlen(pSms->tel) - 2);
		}
		else
			strcpy(tel, pSms->tel);

		//������֤lhy
		int checkRet = CheckTel(tel);
		if (0 != checkRet)
		{
			ret = 9996;
			break;
		}

		//��ʽ��֤		
		char content[256] = {0};
		strcpy(content, pSms->content);
		if (8 <= strlen(content))//��ʽָ��
		{
			char cmd[3] = {0};
			savePtr = NULL;
			split_result = strtok_r(content, "#", &savePtr);
			if (split_result != NULL)
			{
				strncpy(cmd, split_result, 2);
				switch(atoi(cmd))
				{
				case 1://���ò�Ʒ����lsd
					{
						char strPwd[7] = {0};
						split_result = strtok_r( NULL, "#", &savePtr);
						if (NULL == split_result)
							break;
						strncpy(strPwd, split_result, 6);
						//��֤�豸���Ȩ�޺Ϸ���
						if( 0 != CheckSmsPwd(strPwd))
						{
							DBG_GSMCTRL(("CheckSmsPwd Failed\n"));
							break;
						}

						char proDate[9] = {0};
						split_result = strtok_r( NULL, "#", &savePtr);
						if (NULL == split_result)
							break;
						strncpy(proDate, split_result, 8);
						//���ò�Ʒ����
						if( 0 != SetPro_Date(proDate))
						{
							DBG_GSMCTRL(("SetPro_Date Failed\n"));
							break;
						}
						//���ز�Ʒ��Ϣ
						if( 0 != SendProInfoToSms(tel))
						{
							DBG_GSMCTRL(("SendProInfoToSms Failed\n"));
							break;
						}
					}
					break;
				case 2://���ò�Ʒ����lsd
					{
						char strPwd[7] = {0};
						split_result = strtok_r( NULL, "#", &savePtr);
						if (NULL == split_result)
							break;
						strncpy(strPwd, split_result, 6);
						//��֤�豸���Ȩ�޺Ϸ���
						if( 0 != CheckSmsPwd(strPwd))
						{
							DBG_GSMCTRL(("CheckSmsPwd Failed\n"));
							break;
						}

						char proTel[12] = {0};
						split_result = strtok_r( NULL, "#", &savePtr);
						if (NULL == split_result)
							break;
						strncpy(proTel, split_result, 11);
						//���ò�Ʒ����
						if( 0 != SetPro_Tel(proTel))
						{
							DBG_GSMCTRL(("SetPro_Tel Failed\n"));
							break;
						}
						//���ز�Ʒ��Ϣ
						if( 0 != SendProInfoToSms(tel))
						{
							DBG_GSMCTRL(("SendProInfoToSms Failed\n"));
							break;
						}
					}
					break;
				default:
					//SendToSmsList(tel, "0:�豸��,1:ȫ����,2:ȫ���� [��ѯ:1#�豸�ţ�����:2#�豸��#������#ֵ]");
					return 9996;
				}
			}
		}
	} while (false);
	return ret;
}

/******************************************************************************/
//Function Name  : CheckSmsPwd                                               
//Description    : ��ָ֤������                                          
//Input          : pwd
//Output         : None                                                     
//Return         : 0��-1                                                     
/******************************************************************************/
int CGSMCtrl::CheckSmsPwd(char* pwd)
{   
	int ret = -1;
	char sqlResult[7] = {0};
	char condition[32] = {0};
	char* split_result = NULL;
	char* savePtr = NULL;

	sprintf(condition, " where id = 'system'");
	ret = g_SqlCtl.select_from_table(DB_TODAY_PATH, (char*)"user_info", (char*)"pwd", condition, sqlResult);
	if(0 != ret)
		return -1;

	split_result = strtok_r(sqlResult, "#", &savePtr);
	ret = -1;
	DBG_GSMCTRL(("check[%s]\n", split_result));
	if( split_result != NULL )
	{
		if (0 == strcmp(split_result, pwd))
		{
			ret = 0;
		}
	}

	return ret;
}
/******************************************************************************/
//Function Name  : SetPro_Date                                               
//Description    : ���ò�Ʒ����                                         
//Input          : Date
//Output         : None                                                     
//Return         : 0��-1                                                     
/******************************************************************************/
int CGSMCtrl::SetPro_Date(char* proDate)
{   
	int ret = -1;
	char sqlData[128] = {0};
	char newDate[11] = {0};

	int year = 2013;
	int month = 1;
	int day = 1;
	if(0 == sscanf(proDate, "%04d%02d%02d", &year, &month, &day))
	{
		DBG(("SetPro_Date FormatErr!"));
		return ret;
	}
	sprintf(newDate, "%04d-%02d-%02d", year, month, day);
	//�������ݿ��豸״̬
	strcat(sqlData, "PRO_DATE='");
	strcat(sqlData, newDate);
	strcat(sqlData, "', PRO_SIM='");
	strcat(sqlData, HardIF_GetGSMIMSI());
	strcat(sqlData, "'");
	if (g_SqlCtl.update_into_table("PRO_CTRL", sqlData, true))
	{
		ret = 0;
	}
	return ret;
}
int CGSMCtrl::SetPro_Tel(char* proTel)
{   
	int ret = -1;
	char sqlData[128] = {0};

	//�������ݿ��豸״̬
	strcat(sqlData, "PRO_TEL='");
	strcat(sqlData, proTel);
	strcat(sqlData, "', PRO_SIM='");
	strcat(sqlData, HardIF_GetGSMIMSI());
	strcat(sqlData, "'");
	if (g_SqlCtl.update_into_table("PRO_CTRL", sqlData, true))
	{
		ret = 0;
	}
	return ret;
}

int CGSMCtrl::GsmCtrlProc( char* Tel, char* deviceId, char* actionId, char* actionValue)
{
	int ret = SYS_STATUS_FAILED;
	GSM_ACTION_HASH gsmAction;
	memset(&gsmAction, 0, sizeof(GSM_ACTION_HASH));
	time(&gsmAction.act_time);
	gsmAction.Seq = GetSeq();
	strcpy(gsmAction.Tel, Tel);
	strcpy(gsmAction.Value, actionValue);
	char actionSn[DEVICE_ACTION_SN_LEN_TEMP] = {0};
	memcpy(actionSn, actionId, DEVICE_ACTION_SN_LEN);
	if(addNode(gsmAction))//���뵽��ִ�б�
	{
		ACTION_MSG act_msg;
		memset((unsigned char*)&act_msg, 0, sizeof(ACTION_MSG));
		act_msg.ActionSource = ACTION_SOURCE_GSM;//lhy
		strcpy(act_msg.DstId, deviceId);
		strcpy(act_msg.ActionSn, actionSn);

		strcpy(act_msg.Operator, Tel);
		strcpy(act_msg.ActionValue, actionValue);
		act_msg.Seq = gsmAction.Seq;

		ret = CDevInfoContainer::DisPatchAction(act_msg);
		if (SYS_STATUS_SUBMIT_SUCCESS_NEED_RESP != ret &&  SYS_STATUS_SUBMIT_SUCCESS_IN_LIST != ret)
		{
			//�ӻ�Ӧ����ɾ��
			GetAndDeleteHashData(gsmAction);
		}
	}
	return ret;
}	
/******************************************************************************/
//Function Name  : SendDeviceValuesToSms                                               
//Description    : ��֤�豸��źϷ���                                          
//Input          : Tel
//Output         : None                                                     
//Return         : true��false                                                     
/******************************************************************************/
int CGSMCtrl::SendProInfoToSms(char*  Tel)
{   
	char szContent[1024] = {0};
	int ret = g_SqlCtl.select_from_table(DB_TODAY_PATH, (char*)"PRO_CTRL", (char*)"PRO_SEQ, PRO_DATE, PRO_SIM, PRO_TEL", (char*)"", szContent);
	if(0 != ret)
		return -1;
	SendToSmsList(Tel, szContent);
	return SYS_STATUS_SUCCESS;
}
/******************************************************************************/
//Function Name  : SendDeviceValuesToSms                                               
//Description    : ��֤�豸��źϷ���                                          
//Input          : Tel
//Output         : None                                                     
//Return         : true��false                                                     
/******************************************************************************/
int CGSMCtrl::SendProInfoToSmsMonthly()
{   
	int ret = -1;
	char sqlResult[32] = {0};
	char strTel[32] = {0};
	char* split_result = NULL;
	char* savePtr = NULL;

	ret = g_SqlCtl.select_from_table(DB_TODAY_PATH, (char*)"PRO_CTRL", (char*)"IS_GSM, PRO_TEL", (char*)"", sqlResult);
	if(0 != ret)
		return -1;

	split_result = strtok_r(sqlResult, "#", &savePtr);
	if( split_result != NULL )
	{
		if (0 == atoi(split_result))
		{
			return 1;
		}
	}
	split_result = strtok_r(NULL, "#", &savePtr);
	if( split_result != NULL )
	{
		strcpy(strTel, split_result);
		printf("strTel[%s]\n",strTel);
	}
	//���ز�Ʒ��Ϣ
	if( 0 != SendProInfoToSms(strTel))
	{
		DBG_GSMCTRL(("SendProInfoToSms Failed\n"));
		return -2;
	}
	return 0;
}

/******************************************************************************/
//Function Name  : SendDeviceValuesToSms                                               
//Description    : ��֤�豸��źϷ���                                          
//Input          : Tel
//Output         : None                                                     
//Return         : true��false                                                     
/******************************************************************************/
int CGSMCtrl::SendDeviceValuesToSms(char* Tel, char* deviceId)
{   
	DEVICE_DETAIL stDeviceInfo;
	memset((unsigned char*)&stDeviceInfo, 0, sizeof(DEVICE_DETAIL));
	if(SYS_STATUS_SUCCESS != g_DevInfoContainer.GetDeviceInfoNode(deviceId, stDeviceInfo))
		return SYS_STATUS_FAILED;

	char szContent[1512] = {0};
	char szShow[128] = {0};
	strcat(szContent, stDeviceInfo.szCName);
	strcat(szContent, " [");
	memcpy(szShow, stDeviceInfo.Show, sizeof(stDeviceInfo.Show));
	strcat(szContent, trim(szShow, strlen(szShow)));
	strcat(szContent, "] ");
	strcat(szContent, stDeviceInfo.ParaValues);
	SendToSmsList(Tel, szContent);
	return SYS_STATUS_SUCCESS;

	/*
	int ret = -1;
	char sqlResult[128] = {0};
	char condition[128] = {0};

	char* split_result = NULL;
	char* savePtr = NULL;

	char deviceName[20] = {0};
	char status[5] = {0};
	//��ȡ�豸��
	char* hjFormat = " where a.id = '%s'";
	sprintf(condition, hjFormat, deviceId);
	ret = g_SqlCtl.select_from_table(DB_TODAY_PATH, "device_detail a", "a.cname, a.status", condition, sqlResult);
	if(0 != ret)
		return -1;

	split_result = strtok_r(sqlResult, "#", &savePtr);
	
	strcpy(deviceName, split_result);
	split_result = strtok_r(NULL, "#", &savePtr);
	if (NULL == split_result)
	{
		return -1;
	}
	switch(atoi(split_result))
	{
	case 0:
		strcpy(status, "����,");
		break;
	case 1:
		strcpy(status, "����,");
		break;
	case 2:
		strcpy(status, "����,");
		break;
	case 3:
		strcpy(status, "�쳣,");
		break;
	default:
		return -1;
	}
	
	SMS smsInfo;
	memset(&smsInfo, 0, sizeof(SMS));
	smsInfo.sn = 0;
	strcat(smsInfo.content, deviceName);
	strcat(smsInfo.content, status);
	strcat(smsInfo.tel, Tel);

	char* devValueFormat = "select c.attr_name, a.ctime, a.value, b.unit from data_now a, data_type b, device_attr c "
        "where a.id = '%s' and substr(a.id,1,4) = c.id and a.ctype = c.sn and b.id = c.attr_id "
        "group by a.id, a.ctype order by a.ctime desc;";
        //"select data_now.ctime, data_now.value, data_type.unit  from data_now, data_type where data_now.id = '%s' and data_now.ctype = data_type.id order by data_now.ctime;";
	char devValueSql[256] = {0};
	sprintf(devValueSql, devValueFormat, deviceId);
    g_SqlCtl.select_from_tableEx(devValueSql, CGSMCtrl::SelectCallBack_DevDataInfo, (void *)&smsInfo);

	if (strlen(smsInfo.content) > 0)
	{
		SendToSmsList(Tel, smsInfo.content);
	}

	return ret;
	*/
}


int CGSMCtrl::SelectCallBack_DevDataInfo(void *data, int n_column, char **column_value, char **column_name)    //��ѯʱ�Ļص�����
{
	SMS* smsBuf = (SMS*)data;

	char tempData[128] = {0};

	sprintf(tempData, "%s: %s,%s%s|",column_value[0],column_value[1],column_value[2],column_value[3]);

        DBG_GSMCTRL(("tempData[%s] content[%s] contentLen[%zu]\n", tempData, smsBuf->content, strlen(smsBuf->content)));
	if (strlen(smsBuf->content) + strlen(tempData) > SMS_CONTENT_MAXLEN)
	{
        DBG_GSMCTRL(("Tel[%s] content[%s]\n", smsBuf->tel, smsBuf->content));
		SendToSmsList(smsBuf->tel, smsBuf->content);
		memset(smsBuf->content, 0, sizeof(smsBuf->content));
		strcpy(smsBuf->content, tempData);
	}
	else
	{
		strcat(smsBuf->content, tempData);
	}

	return 0;
}

/******************************************************************************/
//Function Name  : GetRoleArea                                               
//Description    : ��ȡ�û�����                                          
//Input          : Tel
//Output         : hj_role                                                     
//Return         : true��false                                                     
/******************************************************************************/
int CGSMCtrl::GetRoleArea(const char*  Tel, char* hj_role)//lhy ��ѯ����
{   
	int ret = -1;
	char condition[256] = {0};

	//��������
	const char* hjFormat 
		= "where length(b.id) = 8 and substr(b.id, 1, 4) = a.ykt_role and a.tel = '%s' group by b.id";
	sprintf(condition, hjFormat, Tel);
	ret = g_SqlCtl.select_from_table(DB_TODAY_PATH, (char*)"user_info a, role b", (char*)"b.id", condition, hj_role);
	return ret;
}


/******************************************************************************/
//Function Name  : CheckTel                                               
//Description    : ��֤�ֻ��Ϸ���                                          
//Input          : Tel
//Output         : None                                                     
//Return         : true��false                                                     
/******************************************************************************/
int CGSMCtrl::CheckTel(const char*  Tel)
{   
	int ret = -1;
	char sqlResult[56] = {0};
	char condition[128] = {0};
	char* split_result = NULL;
	char* savePtr = NULL;
	sprintf(condition, "where pro_tel = '%s'", Tel);

	//if (0 == strcmp(Tel, "18657190588")
	//	|| 0 == strcmp(Tel, "18668208766")
	//	|| 0 == strcmp(Tel, "15658881077"))
	//{
	//	return 0;
	//}
	DBG_GSMCTRL(("CheckTel.....\n"));
	ret = g_SqlCtl.select_from_table(DB_TODAY_PATH, (char*)"PRO_CTRL", (char*)"count(*)", condition, sqlResult);
	if(0 != ret)
		return -1;

	split_result = strtok_r(sqlResult, "#", &savePtr);
	ret = -1;
	DBG_GSMCTRL(("checke[%s]\n", split_result));
	if( split_result != NULL )
	{
		if (1 == atoi(split_result))
		{
			ret = 0;
		}
	}

	return ret;
}


/******************************************************************************/
//Function Name  : CheckDeviceAllow                                               
//Description    : ��֤�豸��źϷ���                                          
//Input          : Tel
//Output         : None                                                     
//Return         : true��false                                                     
/******************************************************************************/
int CGSMCtrl::CheckDeviceAllow(const char*  Tel, char* deviceId)
{   
	int ret = -1;
	char sqlResult[56] = {0};
	char condition[512] = {0};
	char* split_result = NULL;
	char* savePtr = NULL;

	//select count(*) from user_info a, role b where length(b.id) = 6 and substr(b.id, 1, 4) = a.hj_role and a.tel = '13588200884' and b.point like '%030101%';
	//��������

	const char* hjFormat 
		= " where length(b.id) = 8 and substr(b.id, 1, 4) = a.ykt_role and a.tel = '%s' and b.point like '%%%s%%'";
	sprintf(condition, hjFormat, Tel, deviceId);
	ret = g_SqlCtl.select_from_table(DB_TODAY_PATH, (char*)"user_info a, role b", (char*)"count(*)", condition, sqlResult);
	if(0 != ret)
		return -1;

	split_result = strtok_r(sqlResult, "#", &savePtr);
	ret = -1;
	DBG_GSMCTRL(("check[%s]\n", split_result));
	if( split_result != NULL )
	{
		if (1 <= atoi(split_result))
		{
			ret = 0;
		}
	}

	return ret;
}

int CGSMCtrl::GetDevList(char*  roleid, char* deviceList)
{   
	int ret = -1;
	char condition[256] = {0};
	//
	const char* hjFormat = "where id like '%s%%' and length(id) = 8";
	sprintf(condition, hjFormat, roleid);
	ret = g_SqlCtl.select_from_table(DB_TODAY_PATH, (char*)"role", (char*)"point", condition, deviceList);
	return ret;
}


int CGSMCtrl::CheckActionId(char* tel, char* deviceId, char* actionSn)
{
	int ret = -1;
	char sqlResult[56] = {0};
	char condition[512] = {0};
	//char roleId[24] = {0};

	char* split_result = NULL;
	char* savePtr = NULL;

	char devType[DEVICE_TYPE_LEN_TEMP] = {0};

	memcpy(devType, deviceId, DEVICE_TYPE_LEN);

	//��ȡ�������
	const char* roleFormat = "where tel = '%s'";
	sprintf(condition, roleFormat, tel);
	ret = g_SqlCtl.select_from_table(DB_TODAY_PATH, (char*)"user_info", (char*)"ctrl_role", condition, sqlResult);
	if (0 != ret)
	{
		return -1;
	}
	split_result = strtok_r(sqlResult, "#", &savePtr);
	if (NULL == split_result)
	{
		return -1;
	}
	

	//��������

	memset(sqlResult, 0, sizeof(sqlResult));
	memset(condition, 0, sizeof(condition));
	const char* hjFormat = " where b.id like '%%%s%%' and b.point like '%%%s%%'";
	sprintf(condition, hjFormat, split_result, deviceId);

	ret = g_SqlCtl.select_from_table(DB_TODAY_PATH, (char*)"user_info a, role b", (char*)"count(*)", condition, sqlResult);
	if(0 != ret)
		return -1;

	split_result = strtok_r(sqlResult, "#", &savePtr);
	ret = -1;
	if( split_result != NULL )
	{
		if (1 <= atoi(split_result))
		{
			ret = 0;
		}
	}

	return ret;
}
int CGSMCtrl::ActionResponseProc( PACTION_RESP_MSG pActionRespMsg)
{
    DBG_GSMCTRL(("ActionResponseProc--> seq:[%d] \n", pActionRespMsg->Seq));
	GSM_ACTION_HASH hashnode;
	memset(&hashnode, 0, sizeof(GSM_ACTION_HASH));
	hashnode.Seq = pActionRespMsg->Seq;

	if(!GetAndDeleteHashData(hashnode))
    {
        DBG_GSMCTRL(("Delete HashData Failed!\n"));
		return -1;
    }

	//��������ص��û��ֻ�
	char status[56] = {0};
	switch(pActionRespMsg->Status)
	{
	case SYS_STATUS_SUCCESS:
			strcpy(status, "�����ɹ�");
			break;
		case SYS_STATUS_ILLEGAL_CHANNELID:	
			strcpy(status, "ͨ�����Ϸ�");
			break;
		case SYS_STATUS_DEVICE_TYPE_NOT_SURPORT:
			strcpy(status, "�����豸��֧��");
			break;
		case SYS_STATUS_DEVICE_NOT_EXIST:
			strcpy(status, "�豸������");
			break;

		case SYS_STATUS_SUBMIT_SUCCESS:
			strcpy(status, "�ύ�ɹ�");
			break;
		case SYS_STATUS_FAILED:
			strcpy(status, "����ʧ��");
			break;		
		case SYS_STATUS_SEQUENCE_ERROR:
			strcpy(status, "��ˮ���쳣");
			break;
		case SYS_STATUS_FORMAT_ERROR:
			strcpy(status, "��ʽ����");
			break;
		case SYS_STATUS_ILLEGAL_REQUEST:
			strcpy(status, "�Ƿ�����");
			break;
		case SYS_STATUS_TIMEOUT:
			strcpy(status, "����ʱ");
			break;
		case SYS_STATUS_SYS_BUSY:
			strcpy(status, "ϵͳ��æ�����Ժ�����");
			break;
		default:
			strcpy(status, "�����쳣");
			break;
	}
	SendToSmsList(hashnode.Tel, status);
	return 0;
}	

//hashtables

bool CGSMCtrl::addNode(GSM_ACTION_HASH hash_node)
{
	bool ret = false;
	GSM_ACTION_HASH msg_node;
	HashedActInfo::iterator it;

	memset((BYTE*)&msg_node, 0, sizeof(msg_node));
	memcpy((BYTE*)&msg_node, (BYTE*)&hash_node, sizeof(GSM_ACTION_HASH));

	CTblLockGsmAction.lock();
	it = m_ptrActInfoTable->find(msg_node.Seq);
	if (it == m_ptrActInfoTable->end())
	{
		m_ptrActInfoTable->insert(HashedActInfo::value_type(hash_node.Seq,hash_node));
		ret = true;
	}
	CTblLockGsmAction.unlock();
	return ret;
}

bool CGSMCtrl::GetAndDeleteHashData(GSM_ACTION_HASH& hash_node)
{
	bool ret = false;
	HashedActInfo::iterator it;
	CTblLockGsmAction.lock();

	it = m_ptrActInfoTable->find(hash_node.Seq);
	if (it != m_ptrActInfoTable->end())
	{
		memcpy((BYTE*)&hash_node, (BYTE*)&it->second.Seq, sizeof(GSM_ACTION_HASH));
		m_ptrActInfoTable->erase(it);
		ret = true;
	}
	CTblLockGsmAction.unlock();
	return ret;
}

bool CGSMCtrl::GetHashData(GSM_ACTION_HASH& hash_node)
{
	bool ret = false;
	HashedActInfo::iterator it;
	CTblLockGsmAction.lock();

	it = m_ptrActInfoTable->find(hash_node.Seq);
	if (it != m_ptrActInfoTable->end())
	{
		memcpy((BYTE*)&hash_node, (BYTE*)&it->second.Seq, sizeof(GSM_ACTION_HASH));
		ret = true;
	}
	CTblLockGsmAction.unlock();
	return ret;
}

/*******************************End of file*********************************************/
