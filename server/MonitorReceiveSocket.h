// MonitorReceiveSocket.h: interface for the CMonitorReceiveSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MONITORRECEIVESOCKET_H__39910150_1AA7_47F4_9A99_4679C0BFB1AF__INCLUDED_)
#define AFX_MONITORRECEIVESOCKET_H__39910150_1AA7_47F4_9A99_4679C0BFB1AF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseSocket.h"
#include "ACI_H.H"
class CListenSocket;

class CMonitorReceiveSocket : public CBaseSocket
{
public:
	void* m_pDlgPointer;
	void ParsePackage();
	CMonitorReceiveSocket();
	virtual ~CMonitorReceiveSocket();
	CListenSocket* m_pListenSocket;	// classǰ���class CListenSocket;������CListenSocket��CMonitorReceiveSocket�໥ʹ��ʱ�������

// Override
	void OnConnect(int nErrorCode);
	void OnReceive(int nErrorCode);
	void OnClose(int nErrorCode);
	union UStuff
	{
		float   f;
		unsigned char   c[0];
		int i;
	};

//private:
//	void parseBaseToken(BaseType* res,int *st,int length,char* data);
};

#endif // !defined(AFX_MONITORRECEIVESOCKET_H__39910150_1AA7_47F4_9A99_4679C0BFB1AF__INCLUDED_)
