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

/* 실제 서비스 시 이렇게 사용하면 절대 안됨. */
//유저 고유키, 방 고유 키를 만들 전역 변수, 할당 시 마다 +1 해서 사용.
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

