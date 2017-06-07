// ThreadPool.cpp the implementation of thread management class

#include "ThreadPool.h"
#include <algorithm>
#include <unistd.h>
#include <sstream>
#include <stdio.h>
using namespace std;
//�����̳߳�
ThreadPool* ThreadPool::Instance()
{
	static ThreadPool pool;
	return &pool;
}

/****************************************************************************
*	��������:	StartMonitor()												*
*	��������:   ���������߳�												*
*	�������:	��												*
*	�������:	��                                                          *
*	����ֵ:     ��															*
*   ������Ϣ:																*
*****************************************************************************/
bool ThreadPool::StartMonitor()
{
	if (monitorStarted_)
		return false;
	pthread_t tid;
	pthread_create(&tid, 0, &ThreadPool::StaticMonitorThread, 0);
	return true;
}

void* ThreadPool::StaticMonitorThread(void* arg)
{
	Instance()->MonitorThread();
	return 0;
}

//�̼߳���
/****************************************************************************
*	��������:	void MonitorThread()										*
*	��������:   �̼߳���													*
*	�������:	��															*
*	�������:	��                                                          *
*	����ֵ:     ��															*
*   ������Ϣ:																*
*****************************************************************************/
void ThreadPool::MonitorThread()
{
	std::vector<ThreadInfo>::iterator it;
	for (;;)
	{
		bool create = false;
		ThreadCreateInfo info;
		std::string name;
		pthread_mutex_lock(&mutex_);
		std::stringstream strm;
		for (it = threads_.begin(); it != threads_.end(); it++)
		{
			strm.str("");
			strm.clear();
			if (!it->pid)
				continue;
			strm << "/proc/" << it->pid << "/cmdline";
			FILE *file = fopen(strm.str().c_str(), "r");
			if (!file)
			{
				if (it->system)
				{
					create = true;
					info = it->createInfo;
					name = it->name;
				}
				threads_.erase(it);
				break;
			} else
				fclose(file);
		}
		pthread_mutex_unlock(&mutex_);
		if (create)
			CreateThread(name, info.func, true,
						 info.arg, info.attr);
		sleep(1);
	}
}

//�����߳�
/****************************************************************************
*	��������:	bool CreateThread(std::string name, ThreadFunc func, bool system = false,
				  void* arg = 0, pthread_attr_t* attr = 0)					*
*	��������:   �����߳�													*
*	�������:	name:	�߳���												*
*				func:	�̹߳���											*
				system:	ϵͳ��Ϣ											*
				arg:	����												*
				attr:	����												*
*	�������:	��                                                          *
*	����ֵ:     ��															*
*   ������Ϣ:																*
*****************************************************************************/

bool ThreadPool::CreateThread(std::string name, ThreadFunc func, bool system ,
							  void* arg, pthread_attr_t* attr)
{

	pthread_mutex_lock(&mutex_);
	if (threads_.size() >= THREAD_MAX)
	{
		pthread_mutex_unlock(&mutex_);
		return false;
	}
	ThreadInfo info;
	info.system = system;
	info.name = name;
	info.createInfo.func = func;
	info.createInfo.arg = arg;
	info.createInfo.attr = attr;
	info.pid = 0;

	pthread_create(&(info.tid), attr, &ThreadPool::StaticThreadFunction, 0);

	threads_.push_back(info);
	pthread_mutex_unlock(&mutex_);
	return true;
}

void* ThreadPool::StaticThreadFunction(void* arg)
{
	Instance()->ThreadFunction();
	return 0;
}


/****************************************************************************
*	��������:	void ThreadFunction()										*
*	��������:																*
*	�������:	��															*
*	�������:	��                                                          *
*	����ֵ:     ��															*
*   ������Ϣ:																*
*****************************************************************************/
void ThreadPool::ThreadFunction()
{
	pthread_mutex_lock(&mutex_);
	pthread_t tid = pthread_self();
	std::vector<ThreadInfo>::iterator it;
	it = find(threads_.begin(), threads_.end(), ThreadInfo(tid));
	if (it == threads_.end())
	{
		pthread_mutex_unlock(&mutex_);
		return;
	}
	it->pid = getpid();
	ThreadFunc func = it->createInfo.func;
	void *arg = it->createInfo.arg;
	pthread_mutex_unlock(&mutex_);
	pthread_detach(tid);
	func(arg);
	return;
}

bool ThreadPool::SetThreadMode(pthread_t tid, bool IsSystem)
{
	pthread_mutex_lock(&mutex_);
	std::vector<ThreadInfo>::iterator it;
	it = find(threads_.begin(), threads_.end(), ThreadInfo(tid));
	if (it == threads_.end())
	{
		pthread_mutex_unlock(&mutex_);
		return false;
	}
	it->system = IsSystem;
	pthread_mutex_unlock(&mutex_);
	return true;
}

/****************************************************************************
*	��������:	void CancelThread(pthread_t tid)							*
*	��������:   ȡ���߳� 													*
*	�������:	tid: �̺߳�													*
*	�������:	��                                                          *
*	����ֵ:     ��															*
*   ������Ϣ:																*
*****************************************************************************/
void ThreadPool::CancelThread(pthread_t tid)
{
	if (tid == pthread_self())
		pthread_exit(0);
	else
		pthread_cancel(tid);
}

/****************************************************************************
*	��������:	void GetThreadsInfo(std::vector<ThreadInfo>* info)			*
*	��������:   ����߳���Ϣ												*
*	�������:	info: �߳���Ϣ												*
*	�������:	��                                                          *
*	����ֵ:     ��															*
*   ������Ϣ:																*
*****************************************************************************/
void ThreadPool::GetThreadsInfo(std::vector<ThreadInfo>* info)
{
	pthread_mutex_lock(&mutex_);
	*info = threads_;
	pthread_mutex_unlock(&mutex_);
}

ThreadPool::ThreadPool() : monitorStarted_(false)
{
	threads_.reserve(THREAD_MAX);
	pthread_mutex_init(&mutex_, 0);
}

