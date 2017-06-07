#ifndef _DEFINE_APP_H_
#define _DEFINE_APP_H_

#include "Shared.h"

enum _CPM_DATA_TYPE
{
	SHORT = 1,
	INTEGER ,
	FLOAT,
	DOUBLE,
	BYTES = 5,
	CHAR,
	CHARS,
	BCD,
	BCD_2 = 9,
	UCHAR,
	BIT
};

#define DEVICE_DETAIL_MAX		256
#define DEVICE_MODE_MAX		    1024
#define DEVICE_ATTR_MAX			2048//V1.0.1.2


#define	 DEVICE_TYPE_LEN		(6)
#define	 DEVICE_TYPE_LEN_TEMP   (7)

#define	 DEVICE_INDEX_LEN		(4)
#define	 DEVICE_INDEX_LEN_TEMP	(5)

#define  DEVICE_ID_LEN			(10)	//�豸��ų���
#define  DEVICE_ID_LEN_TEMP		(11)	//�豸��ų���

#define  DEVICE_SN_LEN          (4)	//���Ա�ų���
#define  DEVICE_SN_LEN_TEMP     (5)	//���Ա�ų���


#define  DEVICE_CUR_ACTION_LEN          (4)	//��ǰ����ֵ����
#define  DEVICE_CUR_ACTION_LEN_TEMP     (5)	//���Ա�ų���

#define  DEVICE_ACTION_SN_LEN          (8)	//���Ա�ų���
#define  DEVICE_ACTION_SN_LEN_TEMP     (9)	//���Ա�ų���

#define  DEVICE_ATTR_ID_LEN     (14)	//���Ա�ų���
#define  DEVICE_ATTR_ID_LEN_TEMP  (15)	//���Ա�ų���

#define  DEVICE_ACT_ID_LEN		(18)	//�������Ա�ų���
#define  DEVICE_ACT_ID_LEN_TEMP	(19)	//�������Ա�ų���

#define  DEVICE_MODE_LEN		(4)	//�豸����ģʽ��ų���
#define  DEVICE_MODE_LEN_TEMP	(5)	//�豸����ģʽ��ų���
#define  DEVICE_MODEID_LEN		(14)	//�豸����ģʽ��ų���
#define  DEVICE_MODEID_LEN_TEMP	(15)	//�豸����ģʽ��ų���

#define  CPM_CPM_ID				"001102"
#define  CPM_GATEWAY_ID			"0011010001"
#define  GSM_SMS_DEV			"001103"
#define  GSM_CALL_DEV			"001104"


#define  DEV_TYPE_CNOOC_DEAL		"032"
#define  DEV_TYPE_CNOOC_DEAL_ATTID	"0001"

#define  DEV_TYPE_READER		"029101"
#define  DEV_TYPE_READER_ATTID	"0001"

#define	 ACTION_DEFENCE			"0001"		//����/����ģʽ
#define	 ACTION_SET_DEFENCE		"00010001"		//��������
#define	 ACTION_REMOVE_DEFENCE	"00010002"	//��������

#define ONE_DAY_SECONDS (86400)    //һ�������

#define DB_TODAY_PATH		"/home/CPM/tmp.db"//��ʱ���ݿ�
#define DB_HISTORY_PATH		"/home/CPM/cpm.db"//��ʷ���ݿ�
#define DB_SAMPLE_PATH		"/home/CPM/sample.db"//"�걾���ݿ�"

//���� �ڴ涨��
typedef enum{	// ��ȡͨ���Ŷ���
	MUCB_IF_NET_CLI_RCV=0,
	MUCB_IF_NET_CLI_SEND,

	MUCB_IF_NET_SVR_RCV_BOA,
	MUCB_IF_NET_SVR_SEND_BOA,

	MUCB_IF_NET_SVR_RCV_TMN,
	MUCB_IF_NET_SVR_SEND_TMN,
		
	MUCB_DEV_485_1,		//
	MUCB_DEV_485_2,		//
	MUCB_DEV_485_3,		//
	MUCB_DEV_485_4,		//
	MUCB_DEV_485_5,		//
	MUCB_DEV_485_6,		//
	MUCB_DEV_485_7,		//
	MUCB_DEV_485_8,		//
	MUCB_DEV_ADS,		//
	MUCB_DEV_GSM,		//
	MUCB_DEV_NET,		//

	MUCB_DEV_ANALYSE,	//	
    MUCB_DEV_MSGCTRL_BOA,
	MUCB_DEV_MSGCTRL_TMN,

	MUCB_DEV_SMS_RECV,
	MUCB_DEV_SMS_SEND,

	//
	MUCB_DEV_NET_ACTION,
	MUCB_DEV_TRANSPARENT,
	MUCB_MAX
}TBLMUCBEnum;

typedef enum{
	MOD_CTRL,

	MOD_IF_CLI_NET,                 //
	MOD_IF_CLI_CTRL,
	
	MOD_IF_SVR_NET_BOA,
	MOD_IF_SVR_CTRL_BOA,

	MOD_IF_SVR_NET_TMN,
	MOD_IF_SVR_CTRL_TMN,
	//
	MOD_ACTION_485_1,
	MOD_ACTION_485_2,
	MOD_ACTION_485_3,
	MOD_ACTION_485_4,
	MOD_ACTION_485_5,
	MOD_ACTION_485_6,
	MOD_ACTION_485_7,
	MOD_ACTION_485_8,
	MOD_ACTION_ADS,
	MOD_ACTION_GSM,
	MOD_ACTION_NET,

	MOD_DEV_ANALYSE,
    MOD_DEV_MSGCTRL_BOA,
	MOD_DEV_MSGCTRL_TMN,

	MOD_DEV_SMS_RECV,
	MOD_DEV_SMS_SEND,
	
	//
	MOD_DEV_NET_ACTION,
	
	MOD_DEV_TRANSPARENT,		//͸������ҵ�������,

	SYS_MOD_MAX
}SysModType;

//hash�ڵ㣬�Ƚ�ָ��
typedef enum{
	COMPARE_TYPE		= 0x01,
	COMPARE_MODE		= 0x02,
	COMPARE_ID			= 0x03,
	COMPARE_tid			= 0x04,
	COMPARE_TID			= 0x05,
    COMPARE_POSITION    = 0x06
}HASH_COMPARE_CMD;

//CPM��ƽ̨
#define CMD_DEVICE_STATUS_UPDATE      (1000)         //�豸״̬�仯�ϴ�
#define CMD_ENVIRON_DATA_UPDATE       (1001)         //���������ϴ�
#define CMD_YKT_DATA_UPDATE           (1002)         //YKT�����ϴ�
#define CMD_ACT_UPLOAD                (1003)         //�����ϴ�
#define CMD_ALARM_UPLOAD              (1004)         //GSM�澯�����ϴ�
#define CMD_DEVICE_ATTR_STATUS_UP     (1005)         //�豸�ɼ�����״̬�仯�ϴ�
#define CMD_DEVICE_INFO_UP			  (1006)         //�豸��Ϣ�ϴ�
#define CMD_DEVICE_ATTR_UP			  (1007)         //�豸�ɼ�������Ϣ�ϴ�
#define CMD_DEVICE_ACT_UP			  (1008)         //�豸����������Ϣ�ϴ�
#define CMD_TRANSPARENT_DATA_UP	      (1011)         //�к������������ϴ�

//web/ƽ̨��CPM
#define CMD_DEVICE_QUERY			(2000)         //�豸״̬��ѯ
#define CMD_DEVICE_STATUS_QUERY		(2001)		   //�豸ʵʱ״̬��ѯ
#define CMD_DEVICE_ADD				(2002)         //�豸���
#define CMD_DEVICE_UPDATE			(2003)         //�豸�޸�
#define CMD_DEVICE_DEL				(2004)         //�豸ɾ��

#define CMD_DEVICE_PRIATTR_UPDATE	(2005)         //�豸�ɼ������޸�
#define CMD_DEVICE_ACTION_UPDATE	(2006)         //�豸�ɼ�����ɾ��

#define CMD_ONTIME_TASK_ADD         (2008)         //��ʱ��������
#define CMD_ONTIME_TASK_UPDATE      (2009)         //��ʱ�����޸�
#define CMD_ONTIME_TASK_DEL         (2010)         //��ʱ����ɾ��


#define CMD_DEVICE_AGENT_ADD        (2011)         //�豸�������
#define CMD_DEVICE_AGENT_UPDATE     (2012)         //�豸�����޸�
#define CMD_DEVICE_AGENT_DEL        (2013)         //�豸����ɾ��


#define CMD_DEVICES_STATUS_QUERY	(2020)         //�豸״̬��ѯ
#define CMD_DEVICES_PARAS_QUERY		(2021)         //�������ݲ�ѯ
#define CMD_DEVICES_MODES_QUERY		(2022)         //ģʽ���ݲ�ѯ

//Զ�̿���
#define CMD_DEVICE_DEFENCE_UPDATE	(3001)         //������������
#define CMD_ACT_SEND				(3002)         //�����·�

#define CMD_DEVICE_SYN				(3003)         //�豸ͬ��
#define CMD_DEVICE_ATTR_SYN			(3004)         //�豸����ͬ��
#define CMD_DEVICE_ACT_SYN			(3005)         //�豸����ͬ��


#define CMD_SYS_CONFIG_SCH			(3006)         //SCHϵͳ��������

#define CMD_GSM_READ_IMSI			(3007)         //GSM��ȡ��IMSI��


//�������ն˵�CPM
#define CMD_TERMINAL_DATA_SUBMIT			(4001)	//���ݲ���Ҫ��Ӧ
#define CMD_TERMINAL_DATA_SUBMIT_NEEDRESP	(4011)	//������Ҫ��Ӧ

//CPM���������ն�
#define CMD_TERMINAL_CTRL			(4002)			//����

#pragma pack(1)

/************************************************************************/

#define MAX_PACKAGE_LEN_TERMINAL		512
#define MAX_PACKAGE_LEN_NET				2048

//����ģʽѡ��
#define DECODE_TYPE_RDFORMAT     (1)      //�Զ�ȡֵ������ʽ����
#define DECODE_TYPE_WDFORMAT     (2)      //��д��ֵ������ʽ����

enum _INTERFACE_TYPE
{
	TYPE_INTERFACE_RS485 = 1,
	TYPE_INTERFACE_AD_A,
	TYPE_INTERFACE_AD_V,
	TYPE_INTERFACE_DI,
	TYPE_INTERFACE_DO,
	TYPE_INTERFACE_AV_OUT,
	TYPE_INTERFACE_GSM,
	TYPE_INTERFACE_RF,
	TYPE_INTERFACE_NET
};

//CPM �豸�ӿڶ���
enum _INTERFACE_CPM_
{
	//232���ڽӿ�
	INTERFACE_RS232 = 0,

	//485�ӿ�
	INTERFACE_RS485_1 = 1,    //485 1�ſ�        *****************************************
	INTERFACE_RS485_2,        //485 2�ſ�        *  1�ſ�  *  3�ſ�  *  5�ſ�  *  7�ſ�  *
							  //                 *****************************************
	INTERFACE_RS485_3,       //485 3�ſ�        *  2�ſ�  *  4�ſ�  *  6�ſ�  *  8�ſ�  *
	INTERFACE_RS485_4,       //485 4�ſ�        *****************************************
	INTERFACE_RS485_5,       //485 5�ſ�     485(1) ->ttyS3    485(8) ->SPI1.1 (serial0)   485(7) ->SPI1.2 (serial0)
	INTERFACE_RS485_6,       //485 6�ſ�     485(2) ->ttyS2    485(4) ->SPI1.1 (serial1)   485(3) ->SPI1.2 (serial1)
	INTERFACE_RS485_7,       //485 7�ſ�                       485(6) ->SPI1.1 (serial2)   485(5) ->SPI1.2 (serial2)
	INTERFACE_RS485_8,       //485 8�ſ�

	//ģ��������A��
	INTERFACE_AD_A0 = 9,
	INTERFACE_AD_A1,
	INTERFACE_AD_A2,
	INTERFACE_AD_A3,

	//ģ��������B��
	INTERFACE_AD_B0,
	INTERFACE_AD_B1,
	INTERFACE_AD_B2,
	INTERFACE_AD_B3,

	//����������A��
	INTERFACE_DIN_A0 = 17,
	INTERFACE_DIN_A1,
	INTERFACE_DIN_A2,
	INTERFACE_DIN_A3,         //20

	//����������B��
	INTERFACE_DIN_B0,
	INTERFACE_DIN_B1,
	INTERFACE_DIN_B2,
	INTERFACE_DIN_B3,

	//�������������
	INTERFACE_DOUT_A2 = 25,
	INTERFACE_DOUT_A4,
	INTERFACE_DOUT_B2,
	INTERFACE_DOUT_B4,

	//�������������
	INTERFACE_DOUT_A1,
	INTERFACE_DOUT_A3,
	INTERFACE_DOUT_B1,
	INTERFACE_DOUT_B3,

	//�̵�����Դ���
	INTERFACE_AV_OUT_1 = 33,

	INTERFACE_GSM = 34,
	//RF-433�ӿ�
	INTERFACE_RF433 = 35,

	//��Уͨ
	INTERFACE_SHC = 40,

	//��������
	INTERFACE_NET_INPUT = 50,

	MAX_INTERFACE_COUNT

};

enum DEVICE_STATUS
{
	DEVICE_NOT_USE,		//�豸����
	DEVICE_OFFLINE,		//�豸����
	DEVICE_EXCEPTION,	//�豸�쳣
	DEVICE_USE,			//�豸����
	DEVICE_ONLINE		//�豸����
};
enum ACTION_SOURCE
{
	ACTION_SOURCE_SYSTEM,		//ϵͳ����
	ACTION_SOURCE_NET_CPM,		//CPMǶ��ʽ����ָ��
	ACTION_SOURCE_NET_PLA,		//ƽ̨����ָ��
	ACTION_SOURCE_GSM,			//��GSM�������
    ACTION_SOURCE_ONTIME,       //��ʱ����
	ACTION_TO_GSM,				//����GSM�Ķ���
	ACTION_RESUME				//ģʽ��������
};
enum ALARM_LEVEL
{
	ALARM_SERIOUS = 1,				//���ظ澯
	ALARM_GENERAL,				//��ͨ�澯
	ALARM_ATTENTION,			//һ����ʾ
	ALARM_UNKNOW				//δ֪�澯
};

enum SYS_ACTION
{
	SYS_ACTION_SET_DEFENCE = 1,		//��������
	SYS_ACTION_REMOVE_DEFENCE = 2		//��������
};

enum SYS_STATUS
{
	SYS_STATUS_SUCCESS						= 0,	//	�ɹ�
	SYS_STATUS_ILLEGAL_CHANNELID			= 1001,	//	ͨ�����Ϸ�
	SYS_STATUS_DEVICE_TYPE_NOT_SURPORT		= 2001,	//	�����豸��֧��
	SYS_STATUS_DEVICE_NOT_EXIST				= 2002,	//	�豸������
	SYS_STATUS_DEVICE_COUNT_MAX				= 2003,	//	�豸������������
	SYS_STATUS_DEVICE_ATTR_OFFLINE			= 2004,	//	�豸��������
	SYS_STATUS_DEVICE_ATTR_ONLINE			= 2005,	//	�豸��������
	SYS_STATUS_DEVICE_DEV_OFFLINE			= 2006,	//	�豸����
	SYS_STATUS_DEVICE_DEV_ONLINE			= 2007,	//	�豸����
	SYS_STATUS_DEVICE_ATTR_MAX				= 2008,	//	�豸����������������
	SYS_STATUS_DEVICE_ALREADY_EXIST			= 2010,	//	�豸�����Ѵ���
	SYS_STATUS_DEVICE_ACTION_NOT_EXIST		= 2011,	//	�豸�������Բ�����
	SYS_STATUS_DEVICE_ANALYSE_NOT_EXIST		= 2012,	//	�����豸������
	SYS_STATUS_DEVICE_ANALYSE_ACTION_FULL	= 2013,	//	�豸����������
	SYS_STATUS_DEVICE_ACTION_NOT_USE		= 2014,	//	�豸����δ����
	SYS_STATUS_DEVICE_SINGLE_AGENT_MAX		= 2015,	//	�豸����������������������
	SYS_STATUS_DEVICE_AGENT_MAX				= 2016,	//	�豸������������������
	SYS_STATUS_DEVICE_ACTION_MAX			= 2017,	//	�豸������������������

	SYS_STATUS_SUBMIT_SUCCESS				= 3000,	//	�ύ�ɹ�
	SYS_STATUS_SUBMIT_SUCCESS_NEED_RESP		= 3001,	//	�ύ�ɹ�����ȴ���Ӧ
	SYS_STATUS_SUBMIT_SUCCESS_IN_LIST		= 3002,	//	�Ѳ��뵽ִ�ж���
	SYS_STATUS_CREATE_THREAD_FAILED			= 3003,	//	�߳�����ʧ��
	SYS_STATUS_FAILED						= 3006,	//	ʧ��
	SYS_STATUS_FAILED_NO_RESP				= 3007,	//	ʧ��,�����Ӧ
	SYS_STATUS_DEVICE_REMOVE_DEFENCE		= 3008,	//	ʧ��,�豸�ѳ���
	SYS_STATUS_DEVICE_COMMUNICATION_ERROR	= 3009,	//	ͨ���쳣

	SYS_STATUS_TASK_INFO_NOT_EMPTY			= 3010, //  ������иü�¼
	SYS_STATUS_AGENT_INFO_NOT_EMPTY			= 3011, //  �������иü�¼
	SYS_STATUS_ATTR_USED_BY_OTHER			= 3012, //  �������иü�¼
	SYS_STATUS_ROLE_NOT_EMPTY				= 3013, //  �������иü�¼

	SYS_STATUS_ACTION_ENCODE_ERROR			= 3020,	//	д�����������
	SYS_STATUS_ACTION_DECODE_ERROR		    = 3021, //	д���ݽ������
	SYS_STATUS_ACTION_DECODE_FORMAT_ERROR	= 3022,	//	д���ݻ�Ӧ�����ʽ���ô���
	SYS_STATUS_ACTION_COMPARE_FORMAT_ERROR	= 3023,	//	д���ݻ�Ӧ�Ƚϱ��ʽ���ô���
	SYS_STATUS_ACTION_RESP_FAILED			= 3024,	//	д���������ն˻�Ӧʧ��
	SYS_STATUS_ACTION_RESP_FORMAT_ERROR		= 3025,	//	д���������ն˻�Ӧ���ݸ�ʽ����
	SYS_STATUS_ACTION_WFORMAT_ERROR			= 3026,	//	д��ʽ����

	SYS_STATUS_ACION_RESUME_CONFIG_ERROR	= 3030, //  �����������ô���
	SYS_STATUS_ACION_RESUME_ERROR			= 3031, //  ִ�к�������ʧ��

	SYS_STATUS_DATA_DEOCDE_FAIL				= 3040,	//	����ʧ��
	SYS_STATUS_DATA_DECODE_NORMAL			= 3041,	//	�����ɹ�����������
	SYS_STATUS_DATA_DECODE_UNNOMAL_THROW	= 3042,	//	�����ɹ����쳣���ݣ�����
	SYS_STATUS_DATA_DECODE_UNNOMAL_AGENT	= 3043,	//	�����ɹ����쳣���ݣ��ύAgent

	SYS_STATUS_COMM_PARA_ERROR				= 3100,	//	���ڲ�������
	SYS_STATUS_COMM_BAUDRATE_ERROR			= 3101,	//	���ڲ����ʴ���
	SYS_STATUS_OPEN_COMM_FAILED				= 3102,	//	�򿪴���ʧ��
	SYS_STATUS_SEND_COMM_FAILED				= 3103,	//	���ʹ�������ʧ��
	SYS_STATUS_RECV_COMM_FAILED				= 3104,	//	���մ�������ʧ��

	SYS_STATUS_GSM_BUSY						= 4001,	//  GSM��æ
	SYS_STATUS_GSM_PLAY_SOUND_ERROR			= 4002,	//  GSM����ʧ��
	SYS_STATUS_GSM_CALL_NO_CARRIER			= 4003,	//  GSM���ź�
	SYS_STATUS_GSM_CALL_BUSY				= 4004,	//  �Է���æ
	SYS_STATUS_GSM_CALL_NO_DIALTONE			= 4005,	//  �޲�����
	SYS_STATUS_GSM_CALL_BARRED				= 4006,	//  ���ű���
	SYS_STATUS_GSM_SENDLIST_EXCEPTION		= 4007,	//  GSM���Ͷ����쳣
	
	SYS_STATUS_TASK_TIME_FORMAT_ERROR		= 9980,	//	��ʱ����ʱ���ʽ����
	SYS_STATUS_TASK_TIME_MAX				= 9981,	//	��ʱ������������������
	SYS_STATUS_INTERFACE_NOT_USED			= 9990, //  �ӿ�δ����
	SYS_STATUS_DB_LOCKED_ERROR				= 9991,	//	���ݿⱻ��
	SYS_STATUS_CACULATE_MODE_ERROR			= 9992,	//	���㷽ʽ����
	SYS_STATUS_INSERT_DB_FAILED				= 9993,	//	���뵽���ݿ�ʧ��
	SYS_STATUS_SEQUENCE_ERROR				= 9994,	//	��ˮ���쳣
	SYS_STATUS_FORMAT_ERROR					= 9995,	//	��ʽ����
	SYS_STATUS_ILLEGAL_REQUEST				= 9996,	//	�Ƿ�����	
	SYS_STATUS_TIMEOUT						= 9997,	//	����ʱ
	SYS_STATUS_SYS_BUSY						= 9998,	//	ϵͳ��æ	
	SYS_STATUS_UNKONW_ERROR					= 9999	//	δ֪����
};
/************************************************************************/
//--------------------------------------------------------------------------------����ͨ����Ϣ��

typedef struct _MsgLogin{      //�����¼��
	BYTE szLoginId[10];             
	BYTE szTime[14];
	BYTE szMd5[32];
}MsgLogin, *PMsgLogin;

typedef struct _MsgPlDevAdd{      //2001 �豸���
    char szDevId[DEVICE_ID_LEN];              //�豸���
    char szCName[30];						  //�豸����
    char szUpper[DEVICE_ID_LEN];              //�ϼ��豸
}MsgPlDevAdd, *PMsgPlDevAdd;

typedef struct _MsgPlDevDel{      //2003 �豸ɾ��
    char szDevId[DEVICE_ID_LEN];              //�豸���
}MsgPlDevDel, *PMsgPlDevDel;

typedef struct _MsgPlActUpdate{   //2004 �������ø���
    char szDevId[DEVICE_ID_LEN];
    char szActUpdate[1024];
}MsgPlActUpdate, *PMsgPlActUpdate;

typedef struct _MsgPlPriAttr{     //2005 �豸˽�����Ը���
    char szDevId[DEVICE_ID_LEN];  //�豸���
    char szCtype[2];              //Ӧ������
    char szUpChannel[8];          //ͨ����
    char szSelfId[20];            //��ͨ����
    char szSecureTime[20];        //ʱ��
    char szStandardValue[60];     //����ֵ��Χ
    char szPrivateAttr[255];      //˽�в���
}MsgPlPriAttr, *PMsgPlAttr;

typedef struct _MsgPlDefen{       //3001 �豸������������
    char szStatus;
    char devIds[256];
}MsgPlDefen, *PMsgPlDefen;

typedef struct _MsgPlActSend{     //3002 �����·�
    char szOprator[10];           //�����û�
    char szDevId[DEVICE_ID_LEN];  //�豸��
    char szActId[60];             //����ָ��
    char szActValue[128];         //����ֵ
}MsgPlActSend, *PMsgPlActSend;

typedef struct _MsgRecvFromPl{
    char szReserve[20];
    char szStatus[4];
    char szDeal[4];
    union {
        MsgPlDevAdd      stPlDevAdd;
        MsgPlDevDel      stPlDevDel;
        MsgPlActUpdate   stPlActUpdate;
        MsgPlPriAttr     stPlPriAttr;
        MsgPlDefen       stPlDefen;
        MsgPlActSend     stPlActSend;
    };
}MsgRecvFromPl, *PMsgRecvFromPl;

typedef struct _MsgRespToPl{
    char szReserve[20];
    char szStatus[4];
    char szDeal[4];
}MsgRespToPl, *PMsgRespToPl;


typedef struct{
	char szApmId[11]; 
	char szName[6];
	char szPwd[6];
	char szIP[15];
	BYTE btStatus;
	int nClientId;	
}ApmInfo;


typedef struct{
	char szIniFileName[128];	//�����ļ������ݿ�·��
	char szModName[30];			//ģ����
	BYTE pid;               //ͨ����
	BYTE self;				//ģ���ʶ
	BYTE ctrl;				//����ģ��
	BYTE remote;			//Զ��ģ��
	TKU32 unMemHdl;				//�����߳������ڴ�����ľ��
	TKU32 unMemHdlSend;			//�����߳������ڴ�����ľ��
}ModInfo;

//--------------------------------------------------------------------------------Э��ͨ�Ž����ṹ��
typedef struct _NET_DEVICE_INFO_UPDATE
{
	char Id[DEVICE_ID_LEN_TEMP];		//�豸���(8λ)
	char Upper_Channel[3];              //�豸�ϼ�ͨ����
	char Self_Id[21];                   //ͨ��ID
	char PrivateAttr[256];				//�豸˽������
} NET_DEVICE_INFO_UPDATE, *PNET_DEVICE_INFO_UPDATE;


typedef struct _NET_DEVICE_ATTR_INFO_UPDATE
{
	char Id[DEVICE_ID_LEN_TEMP];		//�豸���(6λ)
	char AttrId[DEVICE_SN_LEN_TEMP];						//�豸�ɼ����Ա��
	char SecureTime[21];                //����ʱ��
	char StandardValue[61];             //��׼ֵ
	char PrivateAttr[256];				//˽�в���
	char szV_Id[DEVICE_ID_LEN_TEMP];
	char szV_AttrId[DEVICE_SN_LEN_TEMP];
	char cStaShow;
	char szS_define[129];
	char szAttr_Name[21];
	char cStatus;
	char szValueLimit[65];

} NET_DEVICE_ATTR_INFO_UPDATE, *PNET_DEVICE_ATTR_INFO_UPDATE;

typedef struct _DEVICE_DETAIL_STATUS
{
	char Id[DEVICE_ID_LEN_TEMP];                        //�豸���(8λ)
	char szCName[128];					//�豸��
	char Status;                        //��ǰ״̬
	char Show[56];						//״̬��ʾ
} DEVICE_DETAIL_STATUS, *PDEVICE_DETAIL_STATUS;

typedef struct _DEVICE_DETAIL_PARAS
{
	char Id[DEVICE_ID_LEN_TEMP];                        //�豸���(10λ)
	char ParaValues[MAXMSGSIZE];
} DEVICE_DETAIL_PARAS, *PDEVICE_DETAIL_PARAS;

typedef struct _DEVICE_DETAIL_MODES
{
	char Id[DEVICE_ID_LEN_TEMP];                        //�豸���(10λ)
	char ModeId[DEVICE_ACTION_SN_LEN_TEMP];			  //ģʽ���+ֵ���
	char ParaValues[MAXMSGSIZE];
} DEVICE_DETAIL_MODES, *PDEVICE_DETAIL_MODES;

typedef struct _DEVICE_DETAIL
{
	char Id[DEVICE_ID_LEN_TEMP];                        //�豸���(6λ)
	char szCName[128];					//�豸��
	char Status;                        //��ǰ״̬
	char Show[56];						//״̬��ʾ
	char ParaValues[1024];
} DEVICE_DETAIL, *PDEVICE_DETAIL;

typedef struct _PARA_INFO	//��������
{
	char szId[3];			//ϵͳ���
	char szCname[13];		//���ƣ����¶ȣ�ʪ�ȵ�
	char szUnit[13];		//��λ������
	char szMemo[129];		//��ע
}PARA_INFO, *PPARA_INFO;

//G, E, A ͨ�Žӿ�
typedef struct _AgentInfo{  
	int	 iSeq;					//�������
	char szDevId[DEVICE_ID_LEN_TEMP];				//�������
	char szAttrId[DEVICE_SN_LEN_TEMP];				//�豸�ɼ����Ա��
	char szCondition[129];			//������������
	char szValibtime[31];			//������Чʱ��
	int  iInterval;					//�������
	int  iTimes;					//��������
	char szActDevId[DEVICE_ID_LEN_TEMP];				//�����豸���
	char szActActionId[DEVICE_ACTION_SN_LEN_TEMP];			//�����豸�������
	char szActValue[257];			//����ʱ��������
	char cType;						//�������ͣ�0:�ɼ�������1:��������
}AgentInfo, *PAgentInfo;

//--------------------------------------------------------------------------------�豸��ṹ��
typedef struct _LAST_VALUE_INFO          //
{
	char DevAttrId[DEVICE_ATTR_ID_LEN_TEMP];         //�豸���
	char Type[5];           //��������
	struct timeval LastTime;            //�ϴβ���ʱ��
	int	 VType;             //��ֵ���࣬�縡���ͣ��ַ����͵�
	char Value[MAX_PACKAGE_LEN_TERMINAL];        //�ɼ�����
	int  DataStatus;		//����״̬
	char szLevel[5];
	char szDescribe[257];
	char szValueUseable[MAX_PACKAGE_LEN_TERMINAL];
} LAST_VALUE_INFO, *PLAST_VALUE_INFO;

//�ɼ����Խṹ
typedef struct _DEVICE_INFO_POLL
{
	char Id[DEVICE_ATTR_ID_LEN_TEMP];   //�豸���(10λ) + ��ѯSN(4λ)
	char Upper_Channel[9];              //�豸�ϼ�ͨ����
	char Self_Id[22];                   //ͨ��ID
	char R_Format[128];                 //����ʽ,lhy
	char Rck_Format[128];               //��У��,lhy
	char RD_Format[128];                //������,lhy
	char RD_Comparison_table[512];      //�����,lhy
	char szAttrName[21];				//��������
	char cIsNeedIdChange;				//��Ҫ��ֵת��Ϊ�û����

	char cBeOn;							//��������״̬V10.0.0.8

	unsigned int BaudRate;				//������
	unsigned int databyte;              //����λ
	unsigned int stopbyte;              //ֹͣλ
	unsigned int timeOut;				//�����ڳ�ʱʱ��
	char checkMode[10];                 //��������У�鷽ʽ

	char V_Id[DEVICE_ATTR_ID_LEN_TEMP];	//�������Ա��
	char cStaShow;
	char szS_Define[128];				//״̬ת������
	char szStandard[60];				//��׼��Χ
	char cLedger;						//ͳ�Ʊ��

	char szValueLimit[65];				//lsdֵ��Χ
	int Range_Type;						//ֵ������
	float Range_1;						//ֵ��߽�1
	float Range_2;						//ֵ��߽�2
	int Range_Agent;					//����ֵ����취��0����¼��1��¼-����agent��
	int offLineTotle;					//lsdһ����������

	int isSOIF;							//lsd�Ƿ�ʹ�ö��ƴ���� 0,��ʹ��;1,ʹ��
	int isTransparent;					//lsd�Ƿ�����͸������	0,��ʹ��;1,ʹ��

	char PreId[DEVICE_ATTR_ID_LEN_TEMP];					//��ȡ��ʽ��ͬ����һ�豸����
	char NextId[DEVICE_ATTR_ID_LEN_TEMP];					//��ȡ��ʽ��ͬ����һ�豸����

	char ChannelPreId[DEVICE_ATTR_ID_LEN_TEMP];				//ͨ����ͬ����һ�豸����
	char ChannelNextId[DEVICE_ATTR_ID_LEN_TEMP];			//ͨ����ͬ����һ�豸����

	unsigned long Frequence;			//��Ѳ���
	unsigned char isUpload;				//�Ƿ��ϴ���ƽ̨
	int offLineCnt;                    //���߼���
	struct tm startTime;                //������ʼʱ��
	struct tm endTime;                  //��������ʱ��

	char szAttrValueLimit[65];			//lsd��Ԫ״̬����
	int	retryCount;
	
	LAST_VALUE_INFO stLastValueInfo;		//��������
} DEVICE_INFO_POLL, *PDEVICE_INFO_POLL;

//�������Խṹ
typedef struct _DEVICE_INFO_ACT
{
	char Id[DEVICE_ACT_ID_LEN_TEMP];    //�豸���(10λ) + ����ID(8λ)
	char Upper[11];						//�豸�ϼ���ţ�ID��
	char Upper_Channel[9];              //�豸�ϼ�ͨ����
	char Self_Id[22];                   //ͨ��ID
	char cBeOn;							//����װ��
	char W_Format[128];                 //д��ʽ,lhy
	char Wck_Format[128];               //дУ��,lhy
	char WD_Format[128];                //д����,lhy
	char WD_Comparison_table[512];     //д���,lhy
	unsigned int BaudRate;				//������
	unsigned int databyte;              //����λ
	unsigned int stopbyte;              //ֹͣλ
	unsigned int timeOut;				//�����ڳ�ʱʱ��
	char checkMode[10];                 //��������У�鷽ʽ
	time_t LastTime;					//�ϴζ���ʱ��
	char szCmdName[21];		
	char ctype2[3];
	char szVActionId[DEVICE_ACT_ID_LEN_TEMP];				//���⶯�����
	char flag2;							//�Ƿ�����Ϊ��ǰģʽ
	char flag3;							//�Ƿ�ִ��ǰһģʽ
} DEVICE_INFO_ACT, *PDEVICE_INFO_ACT;

//--------------------------------------------------------------------------------���س������������ṹ��
//���ݲɼ��������
typedef struct _QUERY_INFO_CPM          //����������ģ������ݸ�ʽ
{
	char Cmd;               //��Ϊ1 �����趨�豸����״̬��
	char DevId[DEVICE_ATTR_ID_LEN_TEMP];         //�豸���
	char Type[5];           //��������
	char Time[21];          //���ݲɼ�ʱ��
	int	 VType;             //��ֵ���࣬�縡���ͣ��ַ����͵�
	char Value[MAX_PACKAGE_LEN_TERMINAL];        //�ɼ�����
	int  ValueStatus;		//lsdֵ��״̬��0Ϊ����,1Ϊ�쳣������,2Ϊ�쳣�ҽ���agent����
	int  DataStatus;		//����״̬,1Ϊ�쳣
} QUERY_INFO_CPM, *PQUERY_INFO_CPM;

typedef struct _QUERY_INFO_CPM_EX          //����������ģ������ݸ�ʽ
{
	char Cmd;               //��Ϊ1 �����趨�豸����״̬��
	char DevId[DEVICE_ATTR_ID_LEN_TEMP];         //�豸���
	char Type[5];           //��������
	char Time[21];          //���ݲɼ�ʱ��
	int	 VType;             //��ֵ���࣬�縡���ͣ��ַ����͵�
	char Value[512];        //�ɼ�����
	int  DataStatus;		//����״̬��1Ϊ�쳣
} QUERY_INFO_CPM_EX, *PQUERY_INFO_CPM_EX;

typedef struct _ACTION_MSG
{
	int	 ActionSource;				//��Դ,0,����ָ�1,Ƕ��ʽWebָ�2,ƽָ̨��;3,����ָ��
	char DstId[DEVICE_ID_LEN_TEMP];				//Ŀ���豸
	char ActionSn[DEVICE_ACTION_SN_LEN_TEMP];			//�������Ա��
	char Operator[11];			//����Ա���(��������ΪAUTO)
	char ActionValue[257];		//��������ֵ

	char SrcId[DEVICE_ID_LEN_TEMP];				//����Դ�豸
	char AttrSn[DEVICE_SN_LEN_TEMP];				//��������ָ��
	char SrcValue[128];
	char szActionTimeStamp[21];			//ʱ���
	unsigned int	 Seq;		//ִ����Ϻ����ڲ��һظ�·��
}ACTION_MSG, *PACTION_MSG;

typedef struct _ACTION_MSG_RESP_SMS
{
	int	 ActionSource;			//��Դ,0,����ָ�1,Ƕ��ʽWebָ�2,ƽָ̨��;3,����ָ��
	char DstId[DEVICE_ID_LEN_TEMP];				//Ŀ���豸
	char ActionSn[DEVICE_ACTION_SN_LEN_TEMP];			//�������Ա��
	char Operator[11];			//����Ա���(��������ΪAUTO)
	char ActionValue[257];		//��������ֵ

	char SrcId[DEVICE_ID_LEN_TEMP];				//����Դ�豸
	char AttrSn[DEVICE_SN_LEN_TEMP];				//��������ָ��
	char SrcValue[128];
	char szActionTimeStamp[21];			//ʱ���
	unsigned int	 Seq;		//ִ����Ϻ����ڲ��һظ�·��
	int Status;
}ACTION_MSG_RESP_SMS, *PCTION_MSG_RESP_SMSG;


typedef struct _ACTION_MSG_WAIT_RESP_HASH
{
	unsigned int Id;			//
	ACTION_MSG   stActionMsg;
}ACTION_MSG_WAIT_RESP_HASH, *PACTION_MSG_WAIT_RESP_HASH;

typedef struct _ACTION_RESP_MSG
{
	TKU32 Seq;        //��ϣ����
	int Status;       //ִ��״̬
}ACTION_RESP_MSG, *PACTION_RESP_MSG;

typedef int (*DeviceInit_CallBack)(void*, DEVICE_INFO_POLL*);

/************************************************************************/
/* ���紫���豸                                                         */
/************************************************************************/
typedef struct _NetDispatch{
	char			szReserve[20];				//Reserve
	char			szDeal[4];					//
	char			szStatus[4];
	char 			szPid[10];					//�����ն˱��
	BYTE			szMd5[32];					//MD5
	char			szTid[10];				
	char			szSubId[8];					
	char			szDataLen[4];
	char			szDispatchValue[128];		//ActId[8]/AttrId[4] + DataLen[4] + Data[DataLen]
}NetDispatch, *PNetDispatch;
typedef struct _MsgNetDataSubmit{				//
	char			szReserve[20];				//Reserve
	char			szDeal[4];					//
	char			szStatus[4];
	char 			szPid[10];					//�����ն˱��
	BYTE			szMd5[32];					//MD5
	char			szTid[10];					//�����ն˱��
	char			szAttrId[DEVICE_SN_LEN];	//���Ա�ţ�0000��ʾȫ���ԣ�������ʾָ������
	unsigned int	iDataLen;					//���ݳ���
	unsigned char	szValue[256];				//����
}MsgNetDataSubmit, *PMsgNetDataSubmit;
typedef struct _MsgNetDataDeliver{				//
	char			szReserve[20];				//Reserve
	char			szDeal[4];					//
	char			szStatus[4];
	char 			szPid[10];					//�����ն˱��
	BYTE			szMd5[32];					//MD5
}MsgNetDataDeliver, *PMsgNetDataDeliver;

typedef struct _MsgNetDataAction{				//
	char			szReserve[20];				//Reserve
	char			szDeal[4];					//
	char			szStatus[4];
	char 			szPid[10];					//�����ն˱��
	BYTE			szMd5[32];					//MD5
	char			szTid[10];					//�����ն˱��
	char			szAttrId[DEVICE_ACTION_SN_LEN];
	unsigned int	iDataLen;					//���ݳ���
	unsigned char	szValue[256];				//����
}MsgNetDataAction, *PMsgNetDataAction;
typedef struct _MsgNetDataActionResp{			//
	char			szReserve[20];				//Reserve
	char			szDeal[4];					//
	char			szStatus[4];
	char 			szPid[10];					//�����ն˱��
	BYTE			szMd5[32];					//MD5
}MsgNetDataActionResp, *PMsgNetDataActionResp;

enum _NET_TERMINAL_CMD
{
	NET_TERMINAL_CMD_ACTIVE_TEST		= 1000,		//��·����
	NET_TERMINAL_CMD_DATA_SUBMIT		= 1001,		//�����ϴ�
	NET_TERMINAL_CMD_ACTION_DELEIVE		= 2001,		//����ָ��
	NET_TERMINAL_CMD_DATA_DELEIVE		= 2002		//�ɼ�ָ��
};

typedef struct _DB_OPRATE_FAIL
{
	char szSql[1024];
	time_t tTime;
}DB_OPRATE_FAIL, *PDB_OPRATE_FAIL;

typedef enum _MOD_TYPE
{
	MOD_RS485_1 = 1,
	MOD_RS485_2,
	MOD_RS485_3,
	MOD_RS485_4,
	MOD_RS485_5,
	MOD_RS485_6,
	MOD_RS485_7,
	MOD_RS485_8,
	MOD_AD = 9,
	MOD_GSM = 34,
	MOD_TRANSPARENT = 41,
	MOD_NetTerminal = 50,
	MOD_DB_Insert = 99
}MOD_TYPE;

typedef struct _MODE_CFG
{
	unsigned char Ad;
	unsigned char GSM;
	unsigned char NetTerminal;
	unsigned char Transparent;
	unsigned char RS485_1;
	unsigned char RS485_2;
	unsigned char RS485_3;
	unsigned char RS485_4;
	unsigned char RS485_5;
	unsigned char RS485_6;
	unsigned char RS485_7;
	unsigned char RS485_8;
	unsigned char DB_Insert;
}MODE_CFG, *PMODE_CFG;

typedef struct _RECORD_COUNT_CFG
{
	long User_Max;				//���û�����				01
	long Dev_Max;				//���豸����
	long Attr_Max;				//����������
	long Agent_Max;				//����������
	long TimeCtrl_Max;			//�ܶ�ʱ��������			05
	long Single_Agent_Max;		//��������������
	long NetSend_Max;			//���緢�Ͷ�������
	long NetRecv_Max;			//������ն�������
	long NetClientSend_Max;		//�ͻ������緢�Ͷ�������
	long NetClientRecv_Max;		//�ͻ���������ն�������	10
	long RS485_1_Max;			//485-1������Ϣ����
	long RS485_2_Max;			//485-2������Ϣ����
	long RS485_3_Max;			//485-3������Ϣ����
	long RS485_4_Max;			//485-4������Ϣ����
	long RS485_5_Max;			//485-5������Ϣ����			15
	long RS485_6_Max;			//485-6������Ϣ����
	long RS485_7_Max;			//485-7������Ϣ����
	long RS485_8_Max;			//485-8������Ϣ����
	long ADS_Max;				//ADS������Ϣ����
	long GSM_Max;				//GSM������Ϣ����			20
	long Action_Max;			//�ܶ�������				21
}RECORD_COUNT_CFG, *PRECORD_COUNT_CFG;


typedef struct _STR_DEVICE_AD
{
	int iChannel;
	float fValue;
	float fTempValue[512];
	int iTempCount;
	int iErrCount;
}STR_DEVICE_AD, *PSTR_DEVICE_AD;


//GSM

typedef struct _SMS
{
	int	 sn;
	char tel[20];			//�ֻ���
	char content[1025];		//����
}SMS, *PSMS;



typedef struct _SMS_INFO
{
	int Seq;				//����Դ��ˮ��
	int	 sn;				//ϵͳ������ˮ��
	int  ctype;				//���ͣ�0��������1������
	char srcDev[DEVICE_ID_LEN_TEMP];        //�澯�豸���
	char srcAttr[DEVICE_SN_LEN_TEMP];		//�澯����
	char dstDev[DEVICE_ID_LEN_TEMP];			//Ŀ���豸
	char dstAction[DEVICE_ACTION_SN_LEN_TEMP];		//Ŀ�궯��
	char seprator[11];      //������
	char tel[256];			//�绰����
	char szTimeStamp[21];		//ʱ���
	int  index;				//��Ϣģ�������
	char content[512];		//����
}SMS_INFO, *PSMS_INFO;

typedef struct _GSM_MSG
{
	int iMsgType;//0,GSM���գ�1,���͵�GSM����Ҫ��Ӧ��2,���͵�GSM,����Ҫ��Ӧ��3�������ظ���Ϣ
	union
	{
		SMS_INFO stSmsInfo;
		ACTION_RESP_MSG stActionRespMsg;
		ACTION_MSG	stAction;
		ACTION_MSG_RESP_SMS stRespSMS;
	};
}GSM_MSG, *PGSM_MSG;

#pragma pack()
#endif
