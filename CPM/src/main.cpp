/*****************************************************************************
*	程序名称: main.cpp			                                             *
*	设 计 人: 吕丽民                                                         *
*	设计单位: 杭州百事达应用软件研究所                                       *
*   建立日期: 2003-06-22                                                     *
*	程序功能: 								                                 *
*	修 改 人: 吕丽民                                                         *   
*	修改日期: 2002-05-11                                                     *
*	修改内容:                                                                *
******************************************************************************/
#include "Init.h"


CInit g_clsInit;
int main()
{	 
	if(!g_clsInit.Initialize())
	{
		exit(0);	
	}
	
	g_clsInit.WaitForCmd();

	return 1;
}
