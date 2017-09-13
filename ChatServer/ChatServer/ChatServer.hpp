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
int CompleteRecvPacket(st_CLIENT* Client);

st_CLIENT* FindClient(DWORD dwUserNO);
void DisconnetClient(DWORD dwUserNO);
void ErrHandling(char *message);

/*********************************************/

#endif

