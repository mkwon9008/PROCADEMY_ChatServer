/* single ton class */
/* 해당 템플릿 클래스는 템플릿 인자를 2개만 받을 수 있다.
템플릿 인자를 가변인자로 해서 여러개의 map을 클래스 내에 만들 수 없을까?
*/
/*
#pragma once
#ifndef __CLIENTMAP_MANAGER_H__
#define __CLIENTMAP_MANAGER_H__

#include <map>

using namespace std;

class CClientMapManager
{
private :
	static bool instanceFlag; //is created instance.
	static CClientMapManager* instance;
	CClientMapManager()	{ init(); }
	virtual ~CClientMapManager() { instanceFlag = false; }
	
	std::map<DWORD, st_CLIENT> m_Map;

public:
	void init(void)
	{
		m_Map.clear();
		return;
	}

	static CClientMapManager* getInstance(void)
	{
		if (!instance)
		{
			instance = new CClientMapManager();
			instanceFlag = true;
		}
		return instance;
	}

	bool SetData(DWORD key_data, st_CLIENT val_data)
	{
		if (key_data <= 0)
		{
			return true;
		}

		m_Map.insert(std::pair<DWORD, st_CLIENT>(key_data, val_data));
		return false;
	}

	st_CLIENT* findClient(DWORD key_data)
	{
		map<DWORD, st_CLIENT>::iterator iter = m_Map.find(key_data);
		if (iter == m_Map.end()) //not found.
		{
			return &iter->second;
		}
		else //found.
		{
			return &iter->second;
		}
	}

	map<DWORD, st_CLIENT>::iterator GetIter(void)
	{
		map<DWORD, st_CLIENT>::iterator iter;
		return iter;
	}

	void ClearData(void)
	{
		m_Map.clear();
		return;
	}

};



#endif */ /* __CLIENTMAP_MANAGER_H__ */