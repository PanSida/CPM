#ifndef _SQLCTL_TEST_H
#define _SQLCTL_TEST_H

#include "sqlite3.h"


class CSqlCtlTest
{
public:
    CSqlCtlTest(void);
    ~CSqlCtlTest(void);

	bool OpenDB(const char* DBName);
    void error(const char* error);                      //��ӡ������Ϣ
    void create_database(char *name);                   //�������ݿ�
    void optimize_database(char *name);                 //���ݿ��Ż�
    void create_table(const char *table);               //�������ݱ�
    bool insert_into_table(char *table, char *record);  //�����¼
    bool update_into_table(char *table, char *record); 

    bool select_from_table(char *table, char *column, char *condition, char *result);
	bool select_from_tableEx(char *sql, sqlite3_callback FuncSelectCallBack, void *result);

    void print_table(char *table);                      //��ӡ������
    bool delete_record(char *table,char *expression);   //ɾ����¼
    void close_database(char *tablename);               //�ر����ݿ�
    void begin_transaction(void);                       //��ʼ����
    void commit_transaction(void);                      //�ύ����
    static int loadData(void *data, int n_column, char **column_value, char **column_name);

	bool ExecSql(char *sql, sqlite3_callback FuncSelectCallBack, void * userData);
private:
    sqlite3* db;
    char* errmsg;
    char **strTable;
};


#endif
