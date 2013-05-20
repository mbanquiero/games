#include "stdafx.h"
#include <stdio.h>
#include <io.h>
#include <time.h>
#include "MySocket.h"
#include "ChildView.h"
#include "GameEngine.h"
#include <afxinet.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CServerSocket::CServerSocket()
{
}

CServerSocket::~CServerSocket()
{
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CServerSocket, CAsyncSocket)
	//{{AFX_MSG_MAP(CServerSocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CServerSocket member functions

void CServerSocket::OnAccept(int nErrorCode) 
{
	CAsyncSocket::OnAccept(nErrorCode);

	if(model->cant_players>7)
		return;			// no acepto la conexion

	// acepto la conexion
	CClientSocket *socket = &model->P[model->cant_players++].socket;
	if(!Accept(*socket))
	{
		int error = GetLastError();
		char buffer[255];
		sprintf(buffer,"Error : %d",error);
		//AfxMessageBox(buffer);
		return;

	}
	

	CString address;
	UINT port;
	if(socket->GetPeerName(address,port))
	{
		char buffer[255];
		sprintf(buffer,"%s (%d)",(LPCSTR)address,port);
	}

}


/////////////////////////////////////////////////////////////////////////////
// CClientSocket

CClientSocket::CClientSocket()
{
	pbuff_send = 0;
	pbuff = 0;
	activo = TRUE;
}

CClientSocket::~CClientSocket()
{
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CClientSocket, CAsyncSocket)
	//{{AFX_MSG_MAP(CClientSocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CClientSocket member functions
void CClientSocket::OnConnect(int nErrorCode) 
{
	CAsyncSocket::OnConnect(nErrorCode);
}

void CClientSocket::OnReceive(int nErrorCode) 
{

	char buffer[255];
	CAsyncSocket::OnReceive(nErrorCode);
	int bytes_rec = Receive	(buff+pbuff,sizeof(buff)-1-pbuff);

	while(bytes_rec!=0)
	{
		if(bytes_rec==SOCKET_ERROR)
		{
			int error = GetLastError();
			if(error!=WSAEWOULDBLOCK)
			{
				// mando el error
				sprintf(buffer,"Error %d",error);
				//AfxMessageBox(buffer);
				return;
			}
			else
			{
				// cuando el cliente mande mas datos me va a volver a avisar
				bytes_rec = 0;
			}	
		}
		else
		{
			pbuff+=bytes_rec;
			bytes_rec = Receive(buff+pbuff,sizeof(buff)-1-pbuff);
		}
	}	

	// Proceso el mensaje
	int cant_paquetes = pbuff/sizeof(grid_packet);
	int resto = pbuff%sizeof(grid_packet);
	TVector3d ant_Pos = TVector3d(0,0,0);
	TVector3d Pos = TVector3d(0,0,0);
	for(int i=0;i<cant_paquetes;++i)
	{
		grid_packet *packet = (grid_packet *)(buff+i*sizeof(grid_packet));

		switch(packet->cmd)
		{
			case 0:
				// hello
				// el servidor primero mando un msg de hello al remoto con su nombre, y demas datos, 
				// el remoto le contesta con otro msg de hello, con mismos datos y comienza el juego

				// ambos (cliente y servidor) actualizan los datos del otro jugador: 
				// Recibe los datos del jugador remoto
				strcpy(model->P[1].nombre,packet->data);
				model->P[1].nro_avatar = packet->alfa;
				model->P[1].SetColor((int)packet->beta);

				if(!model->es_servidor)
				{
					// en el caso del cliente, al recibir los datos del servidor, 
					// le tiene que contestar con sus propios datos para que termine la conversacion: 
					grid_packet packet_hello;
					memset(&packet_hello,0,sizeof(packet_hello));
					strncpy(packet_hello.data , model->player_one->nombre,31);
					packet_hello.data[32] = '\0'; 
					packet_hello.cmd = 0;		// Hello packet
					packet_hello.alfa = model->player_one->nro_avatar;
					packet_hello.beta = model->player_one->nro_color;
					// envio el paquete pp dicho:
					model->player_one->socket.SendData((char *)&packet_hello,sizeof(packet_hello));
					model->player_one->socket.Flush();

					// se conecto con el servidor: 
					// aviso que hay un jugador mas, tambien es la señal de empezar el juego
					model->cant_players++;
				}
				break;
			default:
			case 1:
				// info de posicion y dir del jugador remoto
				model->P[1].paquete[model->P[1].cant_paquetes++] = *packet;
				break;
		}
	}
	
	if(resto)
	{
		// procese en total cant_paquetes * sizeof(grid_packet) bytes,
		// el resto es un paquete trunco, lo tengo que leer mas adelante: dejo todo
		// preparado para cuando llegen el resto de los bytes
		memmove(buff,buff+cant_paquetes * sizeof(grid_packet),resto);
		pbuff = resto;
	}
	else
		pbuff = 0;

}

void CClientSocket::Response()
{
	char saux[255];

	Flush();
	if(pbuff_send==0)
	{
		// termine de mandar todo tengo que cerrar
		Close();
		delete this;

	}
	//si no queda bloqueado hasta poder seguir transmitiendo
}




BOOL CClientSocket::SendData(char *buff,int len)
{
	BOOL rta = TRUE;
	memcpy(buff_send+pbuff_send,buff,len);
	pbuff_send+=len;
	return rta;
}

void CClientSocket::Flush()
{
	// Mando el buffer de salida por el socket
	int bytes = Send((LPVOID)buff_send,pbuff_send);
	if(bytes==SOCKET_ERROR)
	{
		int error = GetLastError();
		if(error!=WSAEWOULDBLOCK)
		{
			// mando el error
			char buffer[255];
			sprintf(buffer,"Error %d",error);
			//AfxMessageBox(buffer);
			return ;
		}
		else
		{
			bytes = 0;
		}
	}

	// si la cantidad de bytes tx no es la que quise mandar
	// es porque se trabo la comunicacion...
	if(bytes!=pbuff_send)
	{
		if(bytes>0)
		{
			strncpy(buff_send,(char *)buff_send+bytes,pbuff_send-bytes);
			pbuff_send-=bytes;
		}
	}
	else
	{
		// se vacio el buffer (se destapo la conexion)
		pbuff_send = 0;
	}

}



void CClientSocket::OnSend(int nErrorCode) 
{
	CAsyncSocket::OnSend(nErrorCode);
	if(pbuff_send)
		// Sigo mandando la info q tengo el buffer
		Flush();
}




BOOL get_ip(BYTE *IP)
{

	IP[0] = 192;
	IP[1] = 168;
	IP[2] = 0;
	IP[3] = 100;
	return TRUE;

	BOOL hay_ip = FALSE;
	int A[4];
	char buffer[8192];
	CHttpFile *file;
	CInternetSession inet;
	try
	{
		file = (CHttpFile *)inet.OpenURL("http://www.whatismyip.com/automation/n09230945.asp");
	}
	catch (CInternetException *pEx)
	{
		AfxMessageBox("OpenURL error");
		file = NULL;
	}


	if(file!=NULL)
	{
		memset(buffer,0,sizeof(buffer));
		// viene directamente el IP
		if(file->ReadString(buffer,sizeof(buffer))!=NULL)
		{
			hay_ip = TRUE;
			// leo el ip pp dicho
			int j=0;
			for(int i=0;i<4 && buffer[j];++i)
			{
				char s[10];
				int t = 0;
				while(buffer[j] && buffer[j]!='.')
					s[t++] = buffer[j++];
				s[t] = 0;
				A[i] = atoi(s);

				if(buffer[j])
					++j;		// salteo el .
					
				// leo el siguiente byte
			}
		}


		file->Close();
	}
	inet.Close();
	
	if(hay_ip)
		for(int i=0;i<4;++i)
			IP[i] = (BYTE)A[i];

	return hay_ip;
}
