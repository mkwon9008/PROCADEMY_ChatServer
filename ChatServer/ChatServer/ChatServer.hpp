#pragma once

#ifndef __CHAT_SERVER_H__
#define __CHAT_SERVER_H__

#pragma warning(disable : 4996)
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <cstdlib>
#include <WinSock2.h>
#include <vector>
#include <map>
#include <list>
#include "Protocol.hpp"
#include "includes\SerializationBuffer_v01\SerializationBuffer.hpp"
#include "includes\StreamingBuffer_v01\StreamingBuffer.hpp"

#define BUFSIZE 256
#define dfRECV_BUFF 512

typedef struct st_CLIENT
{
	SOCKET			Socket;
	SOCKADDR_IN		ClientAddr;

	CStreamQueue SendQueue;
	CStreamQueue RecvQueue;

	DWORD dwUserNO;
	DWORD dwEnterRoomNO;

	WCHAR szNickname[dfNICK_MAX_LEN];
	bool bIsRobby;

}CLIENT;

typedef struct st_CHAT_ROOM
{
	DWORD	dwRoomNO;
	WCHAR	szTitle[dfCHAT_ROOM_TITLE_MAX_LEN];

	std::list<DWORD> UserList;
}CHATROOM;

//PIXME : global variable map LINK ERROR.
extern std::map<DWORD, st_CLIENT*>		g_ClientMap;	//Ŭ�� ������ ��.
extern std::map<DWORD, st_CHAT_ROOM*>	g_RoomMap;		//�� ������ ��.

extern SOCKET g_ListenSocket;

/* ���� ���� �� �̷��� ����ϸ� ���� �ȵ�. */
//���� ����Ű, �� ���� Ű�� ���� ���� ����, �Ҵ� �� ���� +1 �ؼ� ���.
extern DWORD g_dwKey_UserNO;
extern DWORD g_dwKey_RoomNO;
/*********************************************/


/*Function pre-declaration********************/
bool NetworkInit(void);
void NetworkProcess(void);
void netProc_Accept(void);
void netProc_Send(DWORD dwUserNO);
void SelectSocket(DWORD* dwpTableNO, SOCKET* pTableSocket, FD_SET* pReadSet, FD_SET* pWriteSet);


st_CLIENT* FindClient(DWORD dwUserNO);
void DisconnetClient(DWORD dwUserNO);
void ErrHandling(char *message);

/*********************************************/

#endif

