#pragma once
#define CRLF	"\013\010"
#include "Afxsock.h"

extern BOOL get_ip(BYTE *IP);

/////////////////////////////////////////////////////////////////////////////

class CServerSocket : public CAsyncSocket
{
// Attributes
public:

	class CGameEngine *model;
// Operations
public:
	CServerSocket();
	virtual ~CServerSocket();

// Overrides
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CServerSocket)
	public:
	virtual void OnAccept(int nErrorCode);
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CServerSocket)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

// Implementation
protected:
};

struct grid_packet
{
	char cmd;			// codigo de comando
	
	//tiempo
	float time;
	float elapsed_time;

	// parametros de la moto
	float pos_x;
	float pos_y;
	float pos_z;
	float dir_x;
	float dir_y;
	float dir_z;
	float vel;
	float vel_v;
	float alfa;
	float beta;

	char data[32];		// 32 bytes no usados por ahora

};


/////////////////////////////////////////////////////////////////////////////
#define BUFFER_LEN	65536

class CClientSocket : public CAsyncSocket
{
// Attributes
public:	
	class CGameEngine *model;
	class player *player;
	BOOL activo;
	char buff_send[BUFFER_LEN];
	int pbuff_send;

	// buffer para recibir
	char buff[BUFFER_LEN];
	int pbuff;


	// Operations
public:
	CClientSocket();
	virtual ~CClientSocket();

	virtual void Response();
	virtual BOOL SendData(char *buff,int len);
	virtual void Flush();

// Overrides
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClientSocket)
	public:
	virtual void OnConnect(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode);
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CClientSocket)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

// Implementation
protected:
};


