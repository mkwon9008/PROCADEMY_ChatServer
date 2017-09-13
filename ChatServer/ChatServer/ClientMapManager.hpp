/* single ton class */
/* �ش� ���ø� Ŭ������ ���ø� ���ڸ� 2���� ���� �� �ִ�.
���ø� ���ڸ� �������ڷ� �ؼ� �������� map�� Ŭ���� ���� ���� �� ������?
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