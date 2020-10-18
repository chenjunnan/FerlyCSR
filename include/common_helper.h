#pragma once
#include "xCommon.h"

#ifndef SAFE_DELETE
#define  SAFE_DELETE(p) { if(p) { delete(p);   (p)=NULL; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif


class AutoLock_helper
{
public:
	explicit AutoLock_helper(CHANDLE  lk) :_lock(lk){ 
		common_mutex_threadlock_lock(_lock); 
	}
	~AutoLock_helper(){
		common_mutex_threadlock_unlock(_lock); 
	}
protected:
	CHANDLE  _lock;
};

struct Message
{
	int msgType;
	void *msgObject;
	std::string message;
	Message()
	{
		msgType = -1;// -2 exit thread  
		msgObject = NULL;
	}

};

class Thread_helper:public thread_interface
{
public:
	Thread_helper() :m_handle(NULL){ if (!common_thread_create_wait(this, &m_handle))LOGE("Thread create error"); }
	virtual~Thread_helper(){ close(); }
	Thread_helper(const Thread_helper& p) = delete;
	Thread_helper& operator=(const Thread_helper& p) = delete;

	virtual Message getq(int t = -1)
	{
		Message msg;
		if (!m_handle)return msg;
		common_thread_get_message(m_handle, &msg.msgType, &msg.message, &msg.msgObject, t);
		return msg;
	}
	virtual void pushq(int t, std::string s = "", void*p = NULL)
	{
		if (!m_handle)return;
		Message msg;
		msg.msgType = t; msg.message = s; msg.msgObject = p;
		common_thread_post_message(m_handle, t, s.c_str(), p);
	}
	virtual void pushq(Message msg)
	{
		if (!m_handle)return;
		common_thread_post_message(m_handle, msg.msgType, msg.message.c_str(), msg.msgObject);
	}
	virtual void rpushq(int t, std::string s = "", void*p = NULL)
	{
		if (!m_handle)return;
		common_thread_rpost_message(m_handle, t, s.c_str(), p);
	}

	virtual void rpushq(Message msg)
	{
		if (!m_handle)return;
		common_thread_rpost_message(m_handle, msg.msgType, msg.message.c_str(), msg.msgObject);
	}
	virtual void clean_msgqueue()
	{
		if (!m_handle)return;
		common_thread_clean_message(m_handle);
	}

	virtual bool start()
	{
		if (m_handle)
		{
			if (common_thread_state(m_handle))
				return true;
			else
				return common_thread_active_wait(m_handle);
		}
		return common_thread_create(this, &m_handle);
	}
	virtual void quit()
	{
		if (!m_handle)return;
		 common_thread_wait_quit(m_handle);

	}
	virtual bool wait(int tim=-1)
	{
		if (!m_handle)return true;
		return common_thread_wait(m_handle, tim);
	}
	virtual bool state()
	{
		if (!m_handle)return false;
		return common_thread_state(m_handle);
	}

protected:
	virtual void close()
	{
		if (m_handle)
		{
			common_thread_wait_quit(m_handle);
			common_thread_release(m_handle);
			m_handle = NULL;
		}
	}

	CHANDLE m_handle;

};

class Timer_helper
{
public:
	Timer_helper() {
		m_handle = common_timer_create();
	}
	void update()
	{
		common_timer_update_time(m_handle);
	}

	bool isreach(unsigned long time_v)
	{
		return common_timer_isreach_time_ms(m_handle, time_v);
	}
	Timer_helper(const Timer_helper& p) = delete;

	Timer_helper& operator=(const Timer_helper& p) = delete;

	virtual~Timer_helper(){ common_timer_release(m_handle); }
protected:
	CHANDLE m_handle;
};


#define LOCK_HELPER_AUTO_HANDLER_IMP(x,name) AutoLock_helper alock_##name(x);
#define LOCK_HELPER_AUTO_HANDLER_CON(x,y) LOCK_HELPER_AUTO_HANDLER_IMP(x,y)
#define LOCK_HELPER_AUTO_HANDLER(x) LOCK_HELPER_AUTO_HANDLER_CON(x,__COUNTER__)