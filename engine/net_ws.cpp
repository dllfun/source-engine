//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
// net_ws.c
// Windows IP Support layer.

#include "tier0/etwprof.h"
#include "tier0/vprof.h"
#include "net_ws_headers.h"
#include "net_ws_queued_packet_sender.h"
#include "fmtstr.h"
#include "host.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define NET_COMPRESSION_STACKBUF_SIZE 4096 

static ConVar net_showudp_wire( "net_showudp_wire", "0", 0, "Show incoming packet information" );

#define UDP_SO_RCVBUF_SIZE 131072

static ConVar net_udp_rcvbuf( "net_udp_rcvbuf", NETSTRING( UDP_SO_RCVBUF_SIZE ), FCVAR_ALLOWED_IN_COMPETITIVE, "Default UDP receive buffer size", true, 8192, true, 128 * 1024 );

static ConVar net_showsplits( "net_showsplits", "0", 0, "Show info about packet splits" );

static ConVar net_splitrate( "net_splitrate", "1", 0, "Number of fragments for a splitpacket that can be sent per frame" );

static ConVar ipname        ( "ip", "localhost", FCVAR_ALLOWED_IN_COMPETITIVE, "Overrides IP for multihomed hosts" );
static ConVar hostport      ( "hostport", NETSTRING( PORT_SERVER ) , FCVAR_ALLOWED_IN_COMPETITIVE, "Host game server port" );
static ConVar hostip		( "hostip", "", FCVAR_ALLOWED_IN_COMPETITIVE, "Host game server ip" );

static ConVar clientport    ( "clientport", NETSTRING( PORT_CLIENT ), FCVAR_ALLOWED_IN_COMPETITIVE, "Host game client port" );
static ConVar hltvport		( "tv_port", NETSTRING( PORT_HLTV ), FCVAR_ALLOWED_IN_COMPETITIVE, "Host SourceTV port" );
static ConVar matchmakingport( "matchmakingport", NETSTRING( PORT_MATCHMAKING ), FCVAR_ALLOWED_IN_COMPETITIVE, "Host Matchmaking port" );
static ConVar systemlinkport( "systemlinkport", NETSTRING( PORT_SYSTEMLINK ), FCVAR_ALLOWED_IN_COMPETITIVE, "System Link port" );

static ConVar fakelag		( "net_fakelag", "0", FCVAR_CHEAT, "Lag all incoming network data (including loopback) by this many milliseconds." );
static ConVar fakeloss		( "net_fakeloss", "0", FCVAR_CHEAT, "Simulate packet loss as a percentage (negative means drop 1/n packets)" ); 
static ConVar droppackets	( "net_droppackets", "0", FCVAR_CHEAT, "Drops next n packets on client" ); 
static ConVar fakejitter	( "net_fakejitter", "0", FCVAR_CHEAT, "Jitter fakelag packet time" );

static ConVar net_compressvoice( "net_compressvoice", "0", 0, "Attempt to compress out of band voice payloads (360 only)." );
ConVar net_usesocketsforloopback( "net_usesocketsforloopback", "1", 0, "Use network sockets layer even for listen server local player's packets (multiplayer only)." );

#ifdef _DEBUG
static ConVar fakenoise		( "net_fakenoise", "0", FCVAR_CHEAT, "Simulate corrupt network packets (changes n bits per packet randomly)" ); 
static ConVar fakeshuffle	( "net_fakeshuffle", "0", FCVAR_CHEAT, "Shuffles order of every nth packet (needs net_fakelag)" ); 
static ConVar recvpackets	( "net_recvpackets", "-1", FCVAR_CHEAT, "Receive exactly next n packets if >= 0" ); 
static ConVar	net_savelargesplits( "net_savelargesplits", "-1", 0, "If not -1, then if a split has this many or more split parts, save the entire packet to disc for analysis." );
#endif

#ifdef _X360
static void NET_LogServerCallback( IConVar *var, const char *pOldString, float flOldValue );
static ConVar net_logserver( "net_logserver", "0", 0,  "Dump server stats to a file", NET_LogServerCallback );
static ConVar net_loginterval( "net_loginterval", "1", 0, "Time in seconds between server logs" );
#endif

//-----------------------------------------------------------------------------
// Toggle Xbox 360 network security to allow cross-platform testing
//-----------------------------------------------------------------------------
#if !defined( _X360 )
#define X360SecureNetwork() false
#define IPPROTO_VDP	IPPROTO_UDP
#elif defined( _RETAIL )
#define X360SecureNetwork() true
#else
bool X360SecureNetwork( void )
{
	if ( CommandLine()->FindParm( "-xnet_bypass_security" ) )
	{
		return false;
	}
	return true;
}
#endif

extern ConVar net_showudp;
extern ConVar net_showtcp;
extern ConVar net_blocksize;
extern ConVar host_timescale;
//extern int host_framecount;

void NET_ClearQueuedPacketsForChannel( INetChannel *chan );


//typedef struct
//{
//	int			nPort;		// UDP/TCP use same port number
//	bool		bListening;	// true if TCP port is listening
//	int			hUDP;		// handle to UDP socket from socket()
//	int			hTCP;		// handle to TCP socket from socket()
//} netsocket_t;

#include "tier0/memdbgoff.h"

#include "tier0/memdbgon.h"

DEFINE_FIXEDSIZE_ALLOCATOR( loopback_t, 2, CUtlMemoryPool::GROW_SLOW );

static ConVar sv_maxroutable
	( 
	"sv_maxroutable", 
	"1260", 
	0, 
	"Server upper bound on net_maxroutable that a client can use.", 
	true, MIN_USER_MAXROUTABLE_SIZE, 
	true, MAX_USER_MAXROUTABLE_SIZE 
	);

ConVar net_maxroutable
	( 
	"net_maxroutable", 
	"1260", 
	FCVAR_ARCHIVE | FCVAR_USERINFO, 
	"Requested max packet size before packets are 'split'.", 
	true, MIN_USER_MAXROUTABLE_SIZE, 
	true, MAX_USER_MAXROUTABLE_SIZE 
	);

netadr_t	net_local_adr;

CNetworkSystem s_NetworkSystem;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CNetworkSystem, INetworkSystem,
	NETWORKSYSTEM_INTERFACE_VERSION, s_NetworkSystem);
CNetworkSystem* g_pLocalNetworkSystem = &s_NetworkSystem;

unsigned short CNetworkSystem::NET_HostToNetShort( unsigned short us_in )
{
	return htons( us_in );
}

unsigned short CNetworkSystem::NET_NetToHostShort( unsigned short us_in )
{
	return ntohs( us_in );
}

// This macro is used to capture the return value of a function call while recording
// a VCR file. During playback, it will get the return value out of the VCR file
// instead of actually calling the function.
#if !defined( NO_VCR )
#define VCR_NONPLAYBACKFN( call, resultVar, eventName ) \
	{ \
		if ( VCRGetMode() != VCR_Playback ) \
			resultVar = call; \
		\
		VCRGenericValue( eventName, &resultVar, sizeof( resultVar ) ); \
	}
#else
#define VCR_NONPLAYBACKFN( call, resultVar, eventName ) \
	{ \
		if ( VCRGetMode() != VCR_Playback ) \
			resultVar = call; \
		\
	}
#endif

/*
====================
NET_ErrorString
====================
*/
const char *CNetworkSystem::NET_ErrorString (int code)
{
#if defined( _WIN32 )
	switch (code)
	{
	case WSAEINTR: return "WSAEINTR";
	case WSAEBADF: return "WSAEBADF";
	case WSAEACCES: return "WSAEACCES";
	case WSAEDISCON: return "WSAEDISCON";
	case WSAEFAULT: return "WSAEFAULT";
	case WSAEINVAL: return "WSAEINVAL";
	case WSAEMFILE: return "WSAEMFILE";
	case WSAEWOULDBLOCK: return "WSAEWOULDBLOCK";
	case WSAEINPROGRESS: return "WSAEINPROGRESS";
	case WSAEALREADY: return "WSAEALREADY";
	case WSAENOTSOCK: return "WSAENOTSOCK";
	case WSAEDESTADDRREQ: return "WSAEDESTADDRREQ";
	case WSAEMSGSIZE: return "WSAEMSGSIZE";
	case WSAEPROTOTYPE: return "WSAEPROTOTYPE";
	case WSAENOPROTOOPT: return "WSAENOPROTOOPT";
	case WSAEPROTONOSUPPORT: return "WSAEPROTONOSUPPORT";
	case WSAESOCKTNOSUPPORT: return "WSAESOCKTNOSUPPORT";
	case WSAEOPNOTSUPP: return "WSAEOPNOTSUPP";
	case WSAEPFNOSUPPORT: return "WSAEPFNOSUPPORT";
	case WSAEAFNOSUPPORT: return "WSAEAFNOSUPPORT";
	case WSAEADDRINUSE: return "WSAEADDRINUSE";
	case WSAEADDRNOTAVAIL: return "WSAEADDRNOTAVAIL";
	case WSAENETDOWN: return "WSAENETDOWN";
	case WSAENETUNREACH: return "WSAENETUNREACH";
	case WSAENETRESET: return "WSAENETRESET";
	case WSAECONNABORTED: return "WSWSAECONNABORTEDAEINTR";
	case WSAECONNRESET: return "WSAECONNRESET";
	case WSAENOBUFS: return "WSAENOBUFS";
	case WSAEISCONN: return "WSAEISCONN";
	case WSAENOTCONN: return "WSAENOTCONN";
	case WSAESHUTDOWN: return "WSAESHUTDOWN";
	case WSAETOOMANYREFS: return "WSAETOOMANYREFS";
	case WSAETIMEDOUT: return "WSAETIMEDOUT";
	case WSAECONNREFUSED: return "WSAECONNREFUSED";
	case WSAELOOP: return "WSAELOOP";
	case WSAENAMETOOLONG: return "WSAENAMETOOLONG";
	case WSAEHOSTDOWN: return "WSAEHOSTDOWN";
	case WSASYSNOTREADY: return "WSASYSNOTREADY";
	case WSAVERNOTSUPPORTED: return "WSAVERNOTSUPPORTED";
	case WSANOTINITIALISED: return "WSANOTINITIALISED";
	case WSAHOST_NOT_FOUND: return "WSAHOST_NOT_FOUND";
	case WSATRY_AGAIN: return "WSATRY_AGAIN";
	case WSANO_RECOVERY: return "WSANO_RECOVERY";
	case WSANO_DATA: return "WSANO_DATA";
	default: return "UNKNOWN ERROR";
	}
#else
	return strerror( code );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *s - 
//			*sadr - 
// Output : bool	NET_StringToSockaddr
//-----------------------------------------------------------------------------
bool CNetworkSystem::NET_StringToSockaddr( const char *s, struct sockaddr *sadr )
{
	char	*colon;
	char	copy[128];
	
	Q_memset (sadr, 0, sizeof(*sadr));
	((struct sockaddr_in *)sadr)->sin_family = AF_INET;
	((struct sockaddr_in *)sadr)->sin_port = 0;

	Q_strncpy (copy, s, sizeof( copy ) );
	// strip off a trailing :port if present
	for (colon = copy ; *colon ; colon++)
	{
		if (*colon == ':')
		{
			*colon = 0;
			((struct sockaddr_in *)sadr)->sin_port = NET_HostToNetShort((short)atoi(colon+1));	
		}
	}
	
	if (copy[0] >= '0' && copy[0] <= '9' && Q_strstr( copy, "." ) )
	{
		*(int *)&((struct sockaddr_in *)sadr)->sin_addr = inet_addr(copy);
	}
	else
	{
		if ( net_nodns )
			return false;	// DNS names disabled

		struct hostent	*h;
		if ( (h = gethostbyname(copy)) == NULL )
			return false;
		*(int *)&((struct sockaddr_in *)sadr)->sin_addr = *(int *)h->h_addr_list[0];
	}
	
	return true;
}

void CNetworkSystem::NET_ClearLastError( void )
{
	net_error = 0;
}

int CNetworkSystem::NET_GetLastError( void )
{
#if defined( _WIN32 )
	net_error = WSAGetLastError();
#else
	net_error = errno;
#endif
#if !defined( NO_VCR )
	VCRGenericValue( "WSAGetLastError", &net_error, sizeof( net_error ) );
#endif
	return net_error;
}

/*
==================
NET_ClearLaggedList

==================
*/
void NET_ClearLaggedList(netpacket_t **pList)
{
	netpacket_t * p = (*pList);

	while ( p )
	{
		netpacket_t * n = p->pNext;

		if ( p->data )
		{
			delete[] p->data;
			p->data = NULL;
		}
		delete p;
		p = n;
	}

	(*pList) = NULL;
}

void CNetSocket::NET_ClearLagData( )
{
	if ( s_pLagData )
	{
		NET_ClearLaggedList( &s_pLagData );
	}
}

/*
=============
NET_StringToAdr

localhost
idnewt
idnewt:28000
192.246.40.70
192.246.40.70:28000
=============
*/
bool CNetworkSystem::NET_StringToAdr ( const char *s, netadr_t *a)
{
	struct sockaddr saddr;

	char address[128];
	
	Q_strncpy( address, s, sizeof(address) );

	if ( !Q_strncmp( address, "localhost", 10 ) || !Q_strncmp( address, "localhost:", 10 ) )
	{
		// subsitute 'localhost' with '127.0.0.1", both have 9 chars
		// this way we can resolve 'localhost' without DNS and still keep the port
		Q_memcpy( address, "127.0.0.1", 9 );
	}

	
	if ( !NET_StringToSockaddr (address, &saddr) )
		return false;
		
	a->SetFromSockadr( &saddr );

	return true;
}

INetChannel*CNetSocket::NET_FindNetChannel(netadr_t &adr)
{
	AUTO_LOCK_FM( s_NetChannels );

	int numChannels = s_NetChannels.Count();

	for ( int i = 0; i < numChannels; i++ )
	{
		INetChannel* chan = s_NetChannels[i];

		// sockets must match
		//if ( socket != chan->GetSocket() )
		//	continue;

		// and the IP:Port address 
		if ( adr.CompareAdr( chan->GetRemoteAddress() )  )
		{
			return chan;	// found it
		}
	}

	return NULL;	// no channel found
}

void CNetworkSystem::NET_CloseSocket( int hSocket , CNetSocket* sock)
{
	if ( !hSocket )
		return;

	// close socket handle
	int ret;
	VCR_NONPLAYBACKFN( closesocket( hSocket ), ret, "closesocket" );
	if ( ret == -1 )
	{
		NET_GetLastError();
		ConMsg ("WARNING! NET_CloseSocket: %s\n", NET_ErrorString(net_error));
	}

	// if hSocket mapped to hTCP, clear hTCP
	if ( sock != NULL )
	{
		if (sock->hTCP == hSocket )
		{
			sock->hTCP = 0;
			sock->bListening = false;
		}
	}
}

/*
====================
NET_IPSocket
====================
*/
int CNetworkSystem::NET_OpenSocket ( const char *net_interface, int& port, int protocol )
{
	struct sockaddr_in	address;
	unsigned int		opt;
	int					newsocket = -1;

	if ( protocol == IPPROTO_TCP )
	{
		VCR_NONPLAYBACKFN( socket (PF_INET, SOCK_STREAM, IPPROTO_TCP), newsocket, "socket()" );
	}
	else // as UDP or VDP
	{
		VCR_NONPLAYBACKFN( socket (PF_INET, SOCK_DGRAM, protocol), newsocket, "socket()" );
	}

	if ( newsocket == -1 )
	{
		NET_GetLastError(); 
		if ( net_error != WSAEAFNOSUPPORT )
			Msg ("WARNING: NET_OpenSockett: socket failed: %s", g_pLocalNetworkSystem->NET_ErrorString(net_error));

		return 0;
	}

	
	opt =  1; // make it non-blocking
	int ret;
	VCR_NONPLAYBACKFN( ioctlsocket (newsocket, FIONBIO, (unsigned long*)&opt), ret, "ioctlsocket" );
	if ( ret == -1 )
	{
		NET_GetLastError();
		Msg ("WARNING: NET_OpenSocket: ioctl FIONBIO: %s\n", g_pLocalNetworkSystem->NET_ErrorString(net_error) );
	}
	
	if ( protocol == IPPROTO_TCP )
	{
		if ( !IsX360() ) // SO_KEEPALIVE unsupported on the 360
		{
			opt = 1; // set TCP options: keep TCP connection alive
			VCR_NONPLAYBACKFN( setsockopt(newsocket, SOL_SOCKET, SO_KEEPALIVE, (char *)&opt, sizeof(opt)), ret, "setsockopt" );
			if (ret == -1)
			{
				NET_GetLastError();		
				Msg ("WARNING: NET_OpenSocket: setsockopt SO_KEEPALIVE: %s\n", g_pLocalNetworkSystem->NET_ErrorString(net_error));
				return 0;
			}
		}

		linger optlinger;	// set TCP options: Does not block close waiting for unsent data to be sent
		optlinger.l_linger = 0;
		optlinger.l_onoff = 0;
		VCR_NONPLAYBACKFN( setsockopt(newsocket, SOL_SOCKET, SO_LINGER, (char *)&optlinger, sizeof(optlinger)), ret, "setsockopt" );
		if (ret == -1)
		{
			NET_GetLastError();		
			Msg ("WARNING: NET_OpenSocket: setsockopt SO_LINGER: %s\n", g_pLocalNetworkSystem->NET_ErrorString(net_error));
			return 0;
		}

		opt = 1; // set TCP options: Disables the Nagle algorithm for send coalescing.
		VCR_NONPLAYBACKFN( setsockopt(newsocket, IPPROTO_TCP, TCP_NODELAY, (char *)&opt, sizeof(opt)), ret, "setsockopt" );
		if (ret == -1)
		{
			NET_GetLastError();		
			Msg ("WARNING: NET_OpenSocket: setsockopt TCP_NODELAY: %s\n", g_pLocalNetworkSystem->NET_ErrorString(net_error));
			return 0;
		}

		opt = NET_MAX_MESSAGE; // set TCP options: set send buffer size
		VCR_NONPLAYBACKFN( setsockopt(newsocket, SOL_SOCKET, SO_SNDBUF, (char *)&opt, sizeof(opt)), ret, "setsockopt" );
		if (ret == -1)
		{
			NET_GetLastError();		
			Msg ("WARNING: NET_OpenSocket: setsockopt SO_SNDBUF: %s\n", g_pLocalNetworkSystem->NET_ErrorString(net_error));
			return 0;
		}

		opt = NET_MAX_MESSAGE; // set TCP options: set receive buffer size
		VCR_NONPLAYBACKFN( setsockopt(newsocket, SOL_SOCKET, SO_RCVBUF, (char *)&opt, sizeof(opt)), ret, "setsockopt" );
		if (ret == -1)
		{
			NET_GetLastError();		
			Msg ("WARNING: NET_OpenSocket: setsockopt SO_RCVBUF: %s\n", g_pLocalNetworkSystem->NET_ErrorString(net_error));
			return 0;
		}
		

		return newsocket;	// don't bind TCP sockets by default
	}

	// rest is UDP only

	opt = 0;
	socklen_t len = sizeof( opt );
	VCR_NONPLAYBACKFN( getsockopt( newsocket, SOL_SOCKET, SO_RCVBUF, (char *)&opt, &len ), ret, "getsockopt" );
	if ( ret == -1 )
	{
		NET_GetLastError();		
		Msg ("WARNING: NET_OpenSocket: getsockopt SO_RCVBUF: %s\n", g_pLocalNetworkSystem->NET_ErrorString(net_error));
		return 0;
	}

	if ( net_showudp.GetBool() )
	{
		static bool bFirst = true;
		if ( bFirst )
		{
			Msg( "UDP socket SO_RCVBUF size %d bytes, changing to %d\n", opt, net_udp_rcvbuf.GetInt() );
		}
		bFirst = false;
	}

	opt = net_udp_rcvbuf.GetInt(); // set UDP receive buffer size
	VCR_NONPLAYBACKFN( setsockopt(newsocket, SOL_SOCKET, SO_RCVBUF, (char *)&opt, sizeof(opt)), ret, "setsockopt" );
	if (ret == -1)
	{
		NET_GetLastError();		
		Msg ("WARNING: NET_OpenSocket: setsockopt SO_RCVBUF: %s\n", g_pLocalNetworkSystem->NET_ErrorString(net_error));
		return 0;
	}

	opt = net_udp_rcvbuf.GetInt(); // set UDP send buffer size
	VCR_NONPLAYBACKFN( setsockopt(newsocket, SOL_SOCKET, SO_SNDBUF, (char *)&opt, sizeof(opt)), ret, "setsockopt" );
	if (ret == -1)
	{
		NET_GetLastError();		
		Msg ("WARNING: NET_OpenSocket: setsockopt SO_SNDBUF: %s\n", g_pLocalNetworkSystem->NET_ErrorString(net_error));
		return 0;
	}


	// VDP protocol (Xbox 360 secure network) doesn't support SO_BROADCAST
 	if ( !X360SecureNetwork() || protocol != IPPROTO_VDP )
 	{
		opt = 1; // set UDP options: make it broadcast capable
		VCR_NONPLAYBACKFN( setsockopt(newsocket, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt)), ret, "setsockopt" );
		if (ret == -1)
		{
			NET_GetLastError();		
			Msg ("WARNING: NET_OpenSocket: setsockopt SO_BROADCAST: %s\n", g_pLocalNetworkSystem->NET_ErrorString(net_error));
			return 0;
		}
	}
	
	if ( CommandLine()->FindParm( "-reuse" ) )
	{
		opt = 1; // make it reusable
		VCR_NONPLAYBACKFN( setsockopt(newsocket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)), ret, "setsockopt" );
		if (ret == -1)
		{
			NET_GetLastError();
			Msg ("WARNING: NET_OpenSocket: setsockopt SO_REUSEADDR: %s\n", g_pLocalNetworkSystem->NET_ErrorString(net_error));
			return 0;
		}
	}

	if (!net_interface || !net_interface[0] || !Q_strcmp(net_interface, "localhost"))
	{
		address.sin_addr.s_addr = INADDR_ANY;
	}
	else
	{
		g_pLocalNetworkSystem->NET_StringToSockaddr (net_interface, (struct sockaddr *)&address);
	}

	address.sin_family = AF_INET;

	int port_offset;	// try binding socket to port, try next 10 is port is already used

	for ( port_offset = 0; port_offset < PORT_TRY_MAX; port_offset++ )
	{
		if ( port == PORT_ANY )
		{
			address.sin_port = 0;	// = INADDR_ANY
		}
		else
		{
			address.sin_port = g_pLocalNetworkSystem->NET_HostToNetShort((short)( port + port_offset ));
		}

		VCR_NONPLAYBACKFN( bind (newsocket, (struct sockaddr *)&address, sizeof(address)), ret, "bind" );
		if ( ret != -1 )
		{
			if ( port != PORT_ANY && port_offset != 0 )
			{
				port += port_offset;	// update port
				ConDMsg( "Socket bound to non-default port %i because original port was already in use.\n", port );
			}
			break;
		}

		NET_GetLastError();

		if ( port == PORT_ANY || net_error != WSAEADDRINUSE )
		{
			Msg ("WARNING: NNET_OpenSocket: bind: %s\n", g_pLocalNetworkSystem->NET_ErrorString(net_error));
			g_pLocalNetworkSystem->NET_CloseSocket(newsocket);//,-1
			return 0;
		}

		// Try next port
	}

	const bool bStrictBind = CommandLine()->FindParm( "-strictportbind" );
	if ( port_offset == PORT_TRY_MAX && !bStrictBind )
	{
		Msg( "WARNING: UDP_OpenSocket: unable to bind socket\n" );
		g_pLocalNetworkSystem->NET_CloseSocket( newsocket );//,-1
		return 0;
	}

	if ( port_offset > 0 )
	{
		if ( bStrictBind )
		{
			// The server op wants to exit if the desired port was not avialable.
			Sys_Exit( "ERROR: Port %i was unavailable - quitting due to \"-strictportbind\" command-line flag!\n", port - port_offset );
		}
		else
		{
			Warning( "WARNING: Port %i was unavailable - bound to port %i instead\n", port - port_offset, port );
		}
	}
	
	return newsocket;
}

int CNetSocket::NET_ConnectSocket( const netadr_t &addr )
{
	Assert( (sock >= 0) && (sock < net_sockets.Count()) );

	//netsocket_t *netsock = &net_sockets[sock];

	if ( hTCP )
	{
		g_pLocalNetworkSystem->NET_CloseSocket( hTCP ,this);
	}

	if (g_pLocalNetworkSystem->net_notcp )
		return 0;

	sockaddr saddr;

	addr.ToSockadr( &saddr );

	int anyport = PORT_ANY;

	hTCP = g_pLocalNetworkSystem->NET_OpenSocket( ipname.GetString(), anyport, true );

	if ( !hTCP )
	{
		Msg( "Warning! NET_ConnectSocket failed opening socket %s, port %i.\n", Net_GetSocketName(), nPort);
		return false;
	}

	int ret;
	VCR_NONPLAYBACKFN( connect( hTCP, &saddr, sizeof(saddr) ), ret, "connect" );
	if ( ret == -1 )
	{
		g_pLocalNetworkSystem->NET_GetLastError();

		if (g_pLocalNetworkSystem->net_error != WSAEWOULDBLOCK )
		{
			Msg ("NET_ConnectSocket: %s\n", g_pLocalNetworkSystem->NET_ErrorString( g_pLocalNetworkSystem->net_error ) );
			return 0;
		}
	}

	return hTCP;
}

int CNetworkSystem::NET_SendStream( int nSocket, const char * buf, int len, int flags )
{
	//int ret = send( nSock, buf, len, flags );
	int ret = VCRHook_send(nSocket, buf, len, flags );
	if ( ret == -1 )
	{
		NET_GetLastError();

		if ( net_error == WSAEWOULDBLOCK )
		{
			return 0; // ignore EWOULDBLOCK
		}

		Msg ("NET_SendStream: %s\n", NET_ErrorString( net_error ) );
	}

	return ret;
}

int CNetworkSystem::NET_ReceiveStream( int nSocket, char * buf, int len, int flags )
{
	int ret = VCRHook_recv(nSocket, buf, len, flags );
	if ( ret == -1 )
	{
		NET_GetLastError();

		if ( net_error == WSAEWOULDBLOCK || 
			 net_error == WSAENOTCONN )
		{
			return 0; // ignore EWOULDBLOCK
		}

		Msg ("NET_ReceiveStream: %s\n", NET_ErrorString( net_error ) );
	}

	return ret;
}

INetChannel *CNetSocket::NET_CreateNetChannel(netadr_t *adr, const char * name, INetChannelHandler * handler, bool bForceNewChannel/*=false*/,
								  int nProtocolVersion/*=PROTOCOL_VERSION*/)
{
	INetChannel* chan = NULL;

	if ( !bForceNewChannel && adr != NULL )
	{
		// try to find real network channel if already existing
		if ( ( chan = NET_FindNetChannel( *adr ) ) != NULL )
		{
			// channel already known, clear any old stuff before Setup wipes all
			chan->Clear();
		}
	}

	if ( !chan )
	{
		// create new channel
		chan = new CNetChan();

		AUTO_LOCK_FM( s_NetChannels );
		s_NetChannels.AddToTail( chan );
	}

	NET_ClearLagData();

	// just reset and return
	((CNetChan*)chan)->Setup( this, adr, name, handler, nProtocolVersion );

	return chan;
}

void CNetSocket::NET_RemoveNetChannel(INetChannel *netchan, bool bDeleteNetChan)
{
	if ( !netchan )
	{
		return;
	}

	AUTO_LOCK_FM( s_NetChannels );
	if ( s_NetChannels.Find( static_cast<CNetChan*>(netchan) ) == s_NetChannels.InvalidIndex() )
	{
		DevMsg(1, "NET_CloseNetChannel: unknown channel.\n");
		return;
	}

	s_NetChannels.FindAndRemove( static_cast<CNetChan*>(netchan) );

	NET_ClearQueuedPacketsForChannel( netchan );
	
	if ( bDeleteNetChan )
		delete netchan;
}


/*
=============================================================================

LOOPBACK BUFFERS FOR LOCAL PLAYER

=============================================================================
*/


void CNetSocket::NET_SendLoopPacket (int length, const unsigned char *data, const netadr_t &to)
{
	loopback_t	*loop;

	if ( length > NET_MAX_PAYLOAD )
	{
		DevMsg( "NET_SendLoopPacket:  packet too big (%i).\n", length );
		return;
	}

	loop = new loopback_t;

	if ( length <= DEF_LOOPBACK_SIZE )
	{
		loop->data = loop->defbuffer;
	}
	else
	{
		loop->data  = new char[ length ];
	}

	Q_memcpy (loop->data, data, length);
	loop->datalen = length;

	if (m_LoopBackSocket)
	{
		m_LoopBackSocket->s_LoopBack.PushItem( loop );
	}
	else
	{
		DevMsg( "NET_SendLoopPacket:  invalid socket (%s).\n", Net_GetSocketName() );
		return;
	}
}

//=============================================================================

int NET_CountLaggedList( netpacket_t *pList )
{
	int c = 0;
	netpacket_t *p = pList;
	
	while ( p )
	{
		c++;
		p = p->pNext;
	}

	return c;
}

/*
===================
NET_AddToLagged

===================
*/
void NET_AddToLagged( netpacket_t **pList, netpacket_t *pPacket )
{
	if ( pPacket->pNext )
	{
		Msg("NET_AddToLagged::Packet already linked\n");
		return;
	}

	// first copy packet

	netpacket_t *newPacket = new netpacket_t;

	(*newPacket) = (*pPacket);  // copy packet infos
	newPacket->data = new unsigned char[ pPacket->size ];	// create new data buffer
	Q_memcpy( newPacket->data, pPacket->data, pPacket->size ); // copy packet data
	newPacket->pNext = NULL;

	// if list is empty, this is our first element
	if ( (*pList) == NULL )
	{
		(*pList) = newPacket;	// put packet in top of list
	}
	else
	{
		netpacket_t *last = (*pList);

		while ( last->pNext )
		{
			// got to end of list
			last = last->pNext;
		}

		// add at end
		last->pNext = newPacket;
	}
}

// Actual lag to use in msec
static float s_FakeLag = 0.0;

float NET_GetFakeLag()
{
	return s_FakeLag;
}

// How quickly we converge to a new value for fakelag
#define FAKELAG_CONVERGE	200  // ms per second

/*
==============================
NET_AdjustLag

==============================
*/
void NET_AdjustLag( void )
{
	static double s_LastTime = 0;
	
	// Bound time step
	
	float dt = g_pNetworkSystem->NET_GetTime() - s_LastTime;
	dt = clamp( dt, 0.0f, 0.2f );
	
	s_LastTime = g_pNetworkSystem->NET_GetTime();

	// Already converged?
	if ( fakelag.GetFloat() == s_FakeLag )
		return;

	// Figure out how far we have to go
	float diff = fakelag.GetFloat() - s_FakeLag;

	// How much can we converge this frame
	float converge = FAKELAG_CONVERGE * dt;

	// Last step, go the whole way
	if ( converge > fabs( diff ) )
	{
		converge = fabs( diff );
	}

	// Converge toward fakelag.GetFloat()
	if ( diff < 0.0 )
	{
		// Converge toward fakelag.GetFloat()
		s_FakeLag -= converge;
	}
	else
	{
		s_FakeLag += converge;
	}
}


bool CNetSocket::NET_LagPacket (bool newdata, netpacket_t * packet)
{

//	if ( packet->source >= MAX_SOCKETS )
//		return newdata; // fake lag not supported for extra sockets

	if ( (droppackets.GetInt() > 0)  && newdata && (this == g_pLocalNetworkSystem->m_pClientSocket) )
	{
		droppackets.SetValue( droppackets.GetInt() - 1 );
		return false;
	}

	if ( fakeloss.GetFloat() && newdata )
	{
		losscount++;

		if ( fakeloss.GetFloat() > 0.0f )
		{
			// Act like we didn't hear anything if we are going to lose the packet.
			// Depends on random # generator.
			if (RandomInt(0,100) <= (int)fakeloss.GetFloat())
				return false;
		}
		else
		{
			int ninterval;

			ninterval = (int)(fabs( fakeloss.GetFloat() ) );
			ninterval = max( 2, ninterval );

			if ( !( losscount % ninterval ) )
			{
				return false;
			}
		}
	}

	if (s_FakeLag <= 0.0)
	{
		// Never leave any old msgs around
	//	for ( int i=0; i<MAX_SOCKETS; i++ )
	//	{
			NET_ClearLagData();
	//	}
	//	return newdata;
	}

	// if new packet arrived in fakelag list
	if ( newdata )
	{
		NET_AddToLagged( &s_pLagData, packet );
	}

	// Now check the correct list and feed any message that is old enough.
	netpacket_t *p = s_pLagData; // current packet

	if ( !p )
		return false;	// no packet in lag list

	float target = s_FakeLag;
	float maxjitter = min( fakejitter.GetFloat(), target * 0.5f );
	target += RandomFloat( -maxjitter, maxjitter );

	if ( (p->received + (target/1000.0f)) > g_pNetworkSystem->NET_GetTime())
		return false;	// not time yet for this packet

#ifdef _DEBUG
	if ( fakeshuffle.GetInt() && p->pNext )
	{
		if ( !RandomInt( 0, fakeshuffle.GetInt() ) )
		{
			// swap p and p->next
			netpacket_t * t = p->pNext;
			p->pNext = t->pNext;
			t->pNext = p;
			p = t; 
		}
	}
#endif
	
	// remove packet p from list (is head)
	s_pLagData = p->pNext;
		
	// copy & adjust content
	//packet->source	= p->source;	
	packet->from	= p->from;		
	packet->pNext	= NULL;			// no next
	packet->received = g_pNetworkSystem->NET_GetTime();	// new time
	packet->size	= p->size;		
	packet->wiresize = p->wiresize;
	packet->stream	= p->stream;
			
	Q_memcpy( packet->data, p->data, p->size );

	// free lag packet
					
	delete[] p->data;
	delete p;
			
	return true;
}





//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNetSocket::NET_DiscardStaleSplitpackets()
{
	vecSplitPacketEntries_t &splitPacketEntries = net_splitpacket;
	int i;
	for ( i = splitPacketEntries.Count() - 1; i >= 0; i-- )
	{
		CSplitPacketEntry *entry = &splitPacketEntries[ i ];
		Assert( entry );

		if (g_pNetworkSystem->NET_GetTime() < ( entry->lastactivetime + SPLIT_PACKET_STALE_TIME ) )
			continue;

		splitPacketEntries.Remove( i );
	}

	if ( splitPacketEntries.Count() > SPLIT_PACKET_TRACKING_MAX )
	{
		while ( splitPacketEntries.Count() > SPLIT_PACKET_TRACKING_MAX )
		{
			CSplitPacketEntry *entry = &splitPacketEntries[ i ];
			if (g_pNetworkSystem->NET_GetTime() != entry->lastactivetime )
				splitPacketEntries.Remove(0); // we add to tail each time, so head is the oldest entry, kill them first
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *from - 
// Output : CSplitPacketEntry
//-----------------------------------------------------------------------------
CSplitPacketEntry *CNetSocket::NET_FindOrCreateSplitPacketEntry( netadr_t *from )
{
	vecSplitPacketEntries_t &splitPacketEntries = net_splitpacket;
	int i, count = splitPacketEntries.Count();
	CSplitPacketEntry *entry = NULL;
	for ( i = 0; i < count; i++ )
	{
		entry = &splitPacketEntries[ i ];
		Assert( entry );

		if ( from->CompareAdr(entry->from) )
			break;
	}

	if ( i >= count )
	{
		CSplitPacketEntry newentry;
		newentry.from = *from;

		splitPacketEntries.AddToTail( newentry );

		entry = &splitPacketEntries[ splitPacketEntries.Count() - 1 ];
	}

	Assert( entry );
	return entry;
}

//static char const *DescribeSocket( CNetSocket* sock )
//{
//	switch ( sock )
//	{
//	default:
//		break;
//	case NS_CLIENT:
//		return "cl ";
//	case NS_SERVER:
//		return "sv ";
//	case NS_HLTV:
//		return "htv";
//	case NS_MATCHMAKING:
//		return "mat";
//	case NS_SYSTEMLINK:
//		return "lnk";
//#ifdef LINUX
//	case NS_SVLAN:
//		return "lan";
//#endif
//	}
//
//	return "??";
//}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pData - 
//			size - 
//			*outSize - 
// Output : bool
//-----------------------------------------------------------------------------
bool CNetSocket::NET_GetLong( netpacket_t *packet )
{
	int				packetNumber, packetCount, sequenceNumber, offset;
	short			packetID;
	SPLITPACKET		*pHeader;
	
	if ( packet->size < sizeof(SPLITPACKET) ) 
	{
		Msg( "Invalid split packet length %i\n", packet->size );
		return false;
	}

	pHeader = ( SPLITPACKET * )packet->data;
	// pHeader is network endian correct
	sequenceNumber	= LittleLong( pHeader->sequenceNumber );
	packetID		= LittleShort( (short)pHeader->packetID );
	// High byte is packet number
	packetNumber	= ( packetID >> 8 );	
	// Low byte is number of total packets
	packetCount		= ( packetID & 0xff );	

	int nSplitSizeMinusHeader = (int)LittleShort( (short)pHeader->nSplitSize );
	if ( nSplitSizeMinusHeader < MIN_SPLIT_SIZE ||
		 nSplitSizeMinusHeader > MAX_SPLIT_SIZE )
	{
		Msg( "NET_GetLong:  Split packet from %s with invalid split size (number %i/ count %i) where size %i is out of valid range [%llu - %llu]\n", 
			packet->from.ToString(), 
			packetNumber, 
			packetCount, 
			nSplitSizeMinusHeader,
			(uint64)MIN_SPLIT_SIZE,
			(uint64)MAX_SPLIT_SIZE );
		return false;
	}

	if ( packetNumber >= MAX_SPLITPACKET_SPLITS ||
		 packetCount > MAX_SPLITPACKET_SPLITS )
	{
		Msg( "NET_GetLong:  Split packet from %s with too many split parts (number %i/ count %i) where %llu is max count allowed\n", 
			packet->from.ToString(), 
			packetNumber, 
			packetCount, 
			(uint64)MAX_SPLITPACKET_SPLITS );
		return false;
	}

	CSplitPacketEntry *entry = NET_FindOrCreateSplitPacketEntry( &packet->from );
	Assert( entry );
	if ( !entry )
		return false;

	entry->lastactivetime = g_pNetworkSystem->NET_GetTime();
	Assert( packet->from.CompareAdr( entry->from ) );

	// First packet in split series?
	if ( entry->netsplit.currentSequence == -1 || 
		sequenceNumber != entry->netsplit.currentSequence )
	{
		entry->netsplit.currentSequence	= sequenceNumber;
		entry->netsplit.splitCount		= packetCount;
		entry->netsplit.nExpectedSplitSize = nSplitSizeMinusHeader;
	}

	if ( entry->netsplit.nExpectedSplitSize != nSplitSizeMinusHeader )
	{
		Msg( "NET_GetLong:  Split packet from %s with inconsistent split size (number %i/ count %i) where size %i not equal to initial size of %i\n", 
			packet->from.ToString(), 
			packetNumber, 
			packetCount, 
			nSplitSizeMinusHeader,
			entry->netsplit.nExpectedSplitSize
			);
		entry->lastactivetime = g_pNetworkSystem->NET_GetTime() + SPLIT_PACKET_STALE_TIME;
		return false;
	}

	int size = packet->size - sizeof(SPLITPACKET);

	if ( entry->splitflags[ packetNumber ] != sequenceNumber )
	{
		// Last packet in sequence? set size
		if ( packetNumber == (packetCount-1) )
		{
			entry->netsplit.totalSize = (packetCount-1) * nSplitSizeMinusHeader + size;
		}

		entry->netsplit.splitCount--;		// Count packet
		entry->splitflags[ packetNumber ] = sequenceNumber;

		if ( net_showsplits.GetInt() && net_showsplits.GetInt() != 3 )
		{
			Msg( "<-- [%s] Split packet %4i/%4i seq %5i size %4i mtu %4llu from %s\n", 
				Net_GetSocketName(),
				packetNumber + 1, 
				packetCount, 
				sequenceNumber,
				size, 
				(uint64)(nSplitSizeMinusHeader + sizeof( SPLITPACKET )), 
				packet->from.ToString() );
		}
	}
	else
	{
		Msg( "NET_GetLong:  Ignoring duplicated split packet %i of %i ( %i bytes ) from %s\n", packetNumber + 1, packetCount, size, packet->from.ToString() );
	}


	// Copy the incoming data to the appropriate place in the buffer
	offset = (packetNumber * nSplitSizeMinusHeader);
	memcpy( entry->netsplit.buffer + offset, packet->data + sizeof(SPLITPACKET), size );
	
	// Have we received all of the pieces to the packet?
	if ( entry->netsplit.splitCount <= 0 )
	{
		entry->netsplit.currentSequence = -1;	// Clear packet
		if ( entry->netsplit.totalSize > sizeof(entry->netsplit.buffer) )
		{
			Msg("Split packet too large! %d bytes from %s\n", entry->netsplit.totalSize, packet->from.ToString() );
			return false;
		}

		Q_memcpy( packet->data, entry->netsplit.buffer, entry->netsplit.totalSize );
		packet->size = entry->netsplit.totalSize;
		packet->wiresize = entry->netsplit.totalSize;
		return true;
	}

	return false;
}


bool CNetSocket::NET_GetLoopPacket ( netpacket_t * packet )
{
	Assert ( packet );

	loopback_t	*loop;

//	if ( packet->source > NS_SERVER )
//		return false;
		
	if ( !s_LoopBack.PopItem( &loop ) )
	{
		return false;
	}

	if (loop->datalen == 0)
	{
		// no packet in loopback buffer
		delete loop;
		return ( NET_LagPacket( false, packet ) );
	}

	// copy data from loopback buffer to packet 
	packet->from.SetType( NA_LOOPBACK );
	packet->size = loop->datalen;
	packet->wiresize = loop->datalen;
	Q_memcpy ( packet->data, loop->data, packet->size );
	
	loop->datalen = 0; // buffer is avalibale again

	if ( loop->data != loop->defbuffer )
	{
		delete[] loop->data;
		loop->data = loop->defbuffer;
	}

	delete loop;

	// allow lag system to modify packet
	return ( NET_LagPacket( true, packet ) );	
}

bool CNetworkSystem::NET_ReceiveDatagram (CNetSocket* sock, netpacket_t * packet )
{
	VPROF_BUDGET( "NET_ReceiveDatagram", VPROF_BUDGETGROUP_OTHER_NETWORKING );

	Assert ( packet );
	Assert ( net_multiplayer );

	struct sockaddr	from;
	int				fromlen = sizeof(from);
	int				net_socket = sock->hUDP;

	int ret = 0;
	{
		VPROF_BUDGET( "recvfrom", VPROF_BUDGETGROUP_OTHER_NETWORKING );
		ret = VCRHook_recvfrom(net_socket, (char *)packet->data, NET_MAX_MESSAGE, 0, (struct sockaddr *)&from, (int *)&fromlen );
	}
	if ( ret >= NET_MIN_MESSAGE )
	{
		packet->wiresize = ret;
		packet->from.SetFromSockadr( &from );
		packet->size = ret;

		if ( net_showudp_wire.GetBool() )
		{
			Msg( "WIRE:  UDP sz=%d tm=%f rt %f from %s\n", ret, net_time, Plat_FloatTime(), packet->from.ToString() );
		}

		MEM_ALLOC_CREDIT();
		CUtlMemoryFixedGrowable< byte, NET_COMPRESSION_STACKBUF_SIZE > bufVoice( NET_COMPRESSION_STACKBUF_SIZE );

		unsigned int nVoiceBits = 0u;

		if ( X360SecureNetwork() )
		{
			// X360TBD: Check for voice data and forward it to XAudio
			// For now, just pull off the 2-byte VDP header and shift the data
			unsigned short nDataBytes = ( *( unsigned short * )packet->data );

			Assert( nDataBytes > 0 && nDataBytes <= ret );

			int nVoiceBytes = ret - nDataBytes - 2;
			if ( nVoiceBytes > 0 )
			{
				char *pVoice = (char *)packet->data + 2 + nDataBytes;

				nVoiceBits = (unsigned int)LittleShort( *( unsigned short *)pVoice );
				unsigned int nExpectedVoiceBytes = Bits2Bytes( nVoiceBits );
				pVoice += sizeof( unsigned short );

				int nCompressedSize = nVoiceBytes - sizeof( unsigned short );
				int nDecompressedVoice = COM_GetUncompressedSize( pVoice, nCompressedSize );
				if ( nDecompressedVoice >= 0 )
				{
					if ( (unsigned)nDecompressedVoice != nExpectedVoiceBytes )
					{
						return false;
					}

					bufVoice.EnsureCapacity( nDecompressedVoice );

					// Decompress it
					unsigned unActualDecompressedSize = (unsigned)nDecompressedVoice;
					if ( !COM_BufferToBufferDecompress( (char*)bufVoice.Base(), &unActualDecompressedSize, pVoice, nCompressedSize ) )
						return false;
					Assert( unActualDecompressedSize == (unsigned)nDecompressedVoice );

					nVoiceBytes = unActualDecompressedSize;
				}
				else
				{
					bufVoice.EnsureCapacity( nVoiceBytes );
					Q_memcpy( bufVoice.Base(), pVoice, nVoiceBytes );
				}
			}

			Q_memmove( packet->data, &packet->data[2], nDataBytes );

			ret = nDataBytes;
		}


		if ( ret < NET_MAX_MESSAGE )
		{
			// Check for split message
			if ( LittleLong( *(int *)packet->data ) == NET_HEADER_FLAG_SPLITPACKET )	
			{
				if ( !sock->NET_GetLong( packet ) )
					return false;
			}
			
			// Next check for compressed message
			if ( LittleLong( *(int *)packet->data) == NET_HEADER_FLAG_COMPRESSEDPACKET )
			{
				char *pCompressedData = (char*)packet->data + sizeof( unsigned int );
				unsigned nCompressedDataSize = packet->wiresize - sizeof( unsigned int );

				// Decompress
				int actualSize = COM_GetUncompressedSize( pCompressedData, nCompressedDataSize );
				if ( actualSize <= 0 || actualSize > NET_MAX_PAYLOAD )
					return false;

				MEM_ALLOC_CREDIT();
				CUtlMemoryFixedGrowable< byte, NET_COMPRESSION_STACKBUF_SIZE > memDecompressed( NET_COMPRESSION_STACKBUF_SIZE );
				memDecompressed.EnsureCapacity( actualSize );

				unsigned uDecompressedSize = (unsigned)actualSize;
				COM_BufferToBufferDecompress( (char*)memDecompressed.Base(), &uDecompressedSize, pCompressedData, nCompressedDataSize );
				if ( uDecompressedSize == 0 || ((unsigned int)actualSize) != uDecompressedSize )
				{
					if ( net_showudp.GetBool() )
					{
						Msg( "UDP:  discarding %d bytes from %s due to decompression error [%d decomp, actual %d] at tm=%f rt=%f\n", ret, packet->from.ToString(), uDecompressedSize, actualSize, 
							(float)net_time, (float)Plat_FloatTime() );
					}
					return false;
				}

				// packet->wiresize is already set
				Q_memcpy( packet->data, memDecompressed.Base(), uDecompressedSize );

				packet->size = uDecompressedSize;
			}

			if ( nVoiceBits > 0 )
			{
				// 9th byte is flag byte
				byte flagByte = *( (byte *)packet->data + sizeof( unsigned int ) + sizeof( unsigned int ) );
				unsigned int unPacketBits = packet->size << 3;
				int nPadBits = DECODE_PAD_BITS( flagByte );
				unPacketBits -= nPadBits;

				bf_write fixup;
				fixup.SetDebugName( "X360 Fixup" );
				fixup.StartWriting( packet->data, NET_MAX_MESSAGE, unPacketBits );
				fixup.WriteBits( bufVoice.Base(), nVoiceBits );

				// Make sure we have enough bits to read a final net_NOP opcode before compressing 
				int nRemainingBits = fixup.GetNumBitsWritten() % 8;
				if ( nRemainingBits > 0 &&  nRemainingBits <= (8-NETMSG_TYPE_BITS) )
				{
					fixup.WriteUBitLong( net_NOP, NETMSG_TYPE_BITS );
				}

				packet->size = fixup.GetNumBytesWritten();
			}

			return sock->NET_LagPacket( true, packet );
		}
		else
		{
			ConDMsg ( "NET_ReceiveDatagram:  Oversize packet from %s\n", packet->from.ToString() );
		}
	}
	else if ( ret == -1  )									// error?
	{
		NET_GetLastError();

		switch ( net_error )
		{
		case WSAEWOULDBLOCK:
		case WSAECONNRESET:
		case WSAECONNREFUSED:
			break;
		case WSAEMSGSIZE:
			ConDMsg ("NET_ReceivePacket: %s\n", g_pLocalNetworkSystem->NET_ErrorString(net_error));
			break;
		default:
			// Let's continue even after errors
			ConDMsg ("NET_ReceivePacket: %s\n", g_pLocalNetworkSystem->NET_ErrorString(net_error));
			break;
		}
	}

	return false;
}

bool CNetworkSystem::NET_ReceiveValidDatagram (CNetSocket* sock, netpacket_t * packet )
{
#ifdef _DEBUG
	if ( recvpackets.GetInt() >= 0 )
	{
		unsigned long bytes = 0;

		ioctlsocket( net_sockets[ sock ].hUDP , FIONREAD, &bytes );

		if ( bytes <= 0 )
			return false;

		if ( recvpackets.GetInt() == 0 )
			return false;

		recvpackets.SetValue( recvpackets.GetInt() - 1 );
	}
#endif

	// Failsafe: never call recvfrom more than a fixed number of times per frame.
	// We don't like the potential for infinite loops. Yes this means that 66000
	// invalid packets per frame will effectively DOS the server, but at that point
	// you're basically flooding the network and you need to solve this at a higher
	// firewall or router level instead which is beyond the scope of our netcode.
	// --henryg 10/12/2011
	for ( int i = 1000; i > 0; --i )
	{
		// Attempt to receive a valid packet.
		NET_ClearLastError();
		if ( NET_ReceiveDatagram (sock, packet ) )
		{
			// Received a valid packet.
			return true;
		}
		// NET_ReceiveDatagram calls Net_GetLastError() in case of socket errors
		// or a would-have-blocked-because-there-is-no-data-to-read condition.
		if ( net_error )
		{
			break;
		}
	}
	return false;
}


netpacket_t *CNetSocket::NET_GetPacket (byte *scratch )
{
	VPROF_BUDGET( "NET_GetPacket", VPROF_BUDGETGROUP_OTHER_NETWORKING );

	// Each socket has its own netpacket to allow multithreading
	netpacket_t &inpacket = net_packet;

	NET_AdjustLag();
	NET_DiscardStaleSplitpackets();

	// setup new packet
	inpacket.from.SetType( NA_IP );
	inpacket.from.Clear();
	inpacket.received = g_pNetworkSystem->NET_GetTime();
//	inpacket.source = sock;	
	inpacket.data = scratch;
	inpacket.size = 0;
	inpacket.wiresize = 0;
	inpacket.pNext = NULL;
	inpacket.message.SetDebugName("inpacket.message");

	// Check loopback first
	if ( !NET_GetLoopPacket( &inpacket ) )
	{
		if ( !g_pLocalNetworkSystem->NET_IsMultiplayer() )
		{
			return NULL;
		}

		// then check UDP data 
		if ( !g_pLocalNetworkSystem->NET_ReceiveValidDatagram(this, &inpacket ) )
		{
			// at last check if the lag system has a packet for us
			if ( !NET_LagPacket (false, &inpacket) )
			{
				return NULL;	// we don't have any new packet
			}
		}
	}
	
	Assert ( inpacket.size ); 

#ifdef _DEBUG
	if ( fakenoise.GetInt() > 0 )
	{
		COM_AddNoise( inpacket.data, inpacket.size, fakenoise.GetInt() );
	}
#endif
	
	// prepare bitbuffer for reading packet with new size
	inpacket.message.StartReading( inpacket.data, inpacket.size );

	return &inpacket;
}

void CNetSocket::NET_ProcessPending( void )
{
	AUTO_LOCK_FM( s_PendingSockets );
	for ( int i=0; i<s_PendingSockets.Count();i++ )
	{
		pendingsocket_t * psock = &s_PendingSockets[i];

		ALIGN4 char	headerBuf[5] ALIGN4_POST;

		if ( (g_pNetworkSystem->NET_GetTime() - psock->time) > TCP_CONNECT_TIMEOUT )
		{
			g_pLocalNetworkSystem->NET_CloseSocket( psock->newsock );
			s_PendingSockets.Remove( i );
			continue;
		}

		int ret = g_pLocalNetworkSystem->NET_ReceiveStream( psock->newsock, headerBuf, sizeof(headerBuf), 0 );

		if ( ret == 0 )
		{
			continue;	// nothing received
		}
		else if ( ret == -1 )
		{
			g_pLocalNetworkSystem->NET_CloseSocket( psock->newsock );
			s_PendingSockets.Remove( i );
			continue;	// connection closed somehow
		}
		
		bf_read		header( headerBuf, sizeof(headerBuf) );

		int cmd = header.ReadByte();
		unsigned long challengeNr = header.ReadLong();
		bool bOK = false;	

		if ( cmd == STREAM_CMD_ACKN )
		{
			AUTO_LOCK_FM( s_NetChannels );
			for ( int j = 0; j < s_NetChannels.Count(); j++ )
			{
				CNetChan * chan = (CNetChan*)s_NetChannels[j];

				//if ( chan->GetSocket() != psock->netsock )
				//	continue;

				if ( challengeNr == chan->GetChallengeNr() && !chan->m_StreamSocket )
				{
					if ( psock->addr.CompareAdr( chan->remote_address, true ) )
					{
						chan->m_StreamSocket = psock->newsock;
						chan->m_StreamActive = true;
						
						chan->ResetStreaming();

						bOK = true;

						if ( net_showtcp.GetInt() )
						{
							Msg ("TCP <- %s: connection accepted\n", psock->addr.ToString() );
						}
						
						break;
					}
					else
					{
						Msg ("TCP <- %s: IP address mismatch.\n", psock->addr.ToString() );
					}
				}
			}
		}

		if ( !bOK )
		{
			Msg ("TCP <- %s: invalid connection request.\n", psock->addr.ToString() );
			g_pLocalNetworkSystem->NET_CloseSocket( psock->newsock );
		}

		s_PendingSockets.Remove( i );
	}
}

void CNetSocket::NET_ListenSocket( bool bListen)
{
	Assert((sock >= 0) && (sock < net_sockets.Count()));

	//netsocket_t* netsock = &net_sockets[sock];

	if (hTCP)
	{
		g_pLocalNetworkSystem->NET_CloseSocket(hTCP,this);
	}

	if (!g_pLocalNetworkSystem->NET_IsMultiplayer() || g_pLocalNetworkSystem->net_notcp)
		return;

	if (bListen)
	{
		const char* net_interface = ipname.GetString();

		hTCP = g_pLocalNetworkSystem->NET_OpenSocket(net_interface, nPort, true);

		if (!hTCP)
		{
			Msg("Warning! NET_ListenSocket failed opening socket %i, port %i.\n", 0, nPort);
			return;
		}

		struct sockaddr_in	address;

		if (!net_interface || !net_interface[0] || !Q_strcmp(net_interface, "localhost"))
		{
			address.sin_addr.s_addr = INADDR_ANY;
		}
		else
		{
			g_pLocalNetworkSystem->NET_StringToSockaddr(net_interface, (struct sockaddr*)&address);
		}

		address.sin_family = AF_INET;
		address.sin_port = g_pLocalNetworkSystem->NET_HostToNetShort((short)(nPort));

		int ret;
		VCR_NONPLAYBACKFN(bind(hTCP, (struct sockaddr*)&address, sizeof(address)), ret, "bind");
		if (ret == -1)
		{
			g_pLocalNetworkSystem->NET_GetLastError();
			Msg("WARNING: NET_ListenSocket bind failed on socket %i, port %i.\n", hTCP, nPort);
			return;
		}

		VCR_NONPLAYBACKFN(listen(hTCP, TCP_MAX_ACCEPTS), ret, "listen");
		if (ret == -1)
		{
			g_pLocalNetworkSystem->NET_GetLastError();
			Msg("WARNING: NET_ListenSocket listen failed on socket %i, port %i.\n", hTCP, nPort);
			return;
		}

		bListening = true;
	}
}

void CNetSocket::NET_ProcessListen()
{
	//netsocket_t * netsock = &net_sockets[sock];
		
	if ( !bListening )
		return;

	sockaddr sa;
	int nLengthAddr = sizeof(sa);
		
	int newSocket;

	VCR_NONPLAYBACKFN( accept( hTCP, &sa, (socklen_t*)&nLengthAddr), newSocket, "accept" );
#if !defined( NO_VCR )
	VCRGenericValue( "sockaddr", &sa, sizeof( sa ) );
#endif
	if ( newSocket == -1 )
	{
		g_pLocalNetworkSystem->NET_GetLastError();

		if (g_pLocalNetworkSystem->net_error != WSAEWOULDBLOCK )
		{
			ConDMsg ("NET_ThreadListen: %s\n", g_pLocalNetworkSystem->NET_ErrorString(g_pLocalNetworkSystem->net_error));
		}
		return;
	}

	// new connection TCP request, put in pending queue

	pendingsocket_t psock;

	psock.newsock = newSocket;
	//psock.netsock = sock;
	psock.addr.SetFromSockadr( &sa );
	psock.time = g_pNetworkSystem->NET_GetTime();

	AUTO_LOCK_FM( s_PendingSockets );
	s_PendingSockets.AddToTail( psock );

	// tell client to send challenge number to identify

	char authcmd = STREAM_CMD_AUTH;

	g_pLocalNetworkSystem->NET_SendStream( newSocket, &authcmd, 1 , 0 );

	if ( net_showtcp.GetInt() )
	{
		Msg ("TCP <- %s: connection request.\n", psock.addr.ToString() );
	}
}

struct NetScratchBuffer_t : TSLNodeBase_t
{
	byte data[NET_MAX_MESSAGE];
};
CTSSimpleList<NetScratchBuffer_t> g_NetScratchBuffers;

void CNetSocket::NET_ProcessSocket( IConnectionlessPacketHandler *handler )
{
	VPROF_BUDGET( "NET_ProcessSocket", VPROF_BUDGETGROUP_OTHER_NETWORKING );

	netpacket_t * packet;
	
	Assert ( (sock >= 0) && (sock<net_sockets.Count()) );

	// Scope for the auto_lock
	{
		AUTO_LOCK_FM( s_NetChannels );

		// get streaming data from channel sockets
		int numChannels = s_NetChannels.Count();

		for ( int i = (numChannels-1); i >= 0 ; i-- )
		{
			INetChannel *netchan = s_NetChannels[i];

			// sockets must match
			//if ( sock != netchan->GetSocket() )
			//	continue;

			if ( !netchan->ProcessStream() )
			{
				netchan->GetMsgHandler()->ConnectionCrashed("TCP connection failed.");
			}
		}
	}

	// now get datagrams from sockets
	NetScratchBuffer_t *scratch = g_NetScratchBuffers.Pop();
	if ( !scratch )
	{
		scratch = new NetScratchBuffer_t;
	}
	while ( ( packet = NET_GetPacket ( scratch->data ) ) != NULL )
	{
		if ( Filter_ShouldDiscard ( packet->from ) )	// filtering is done by network layer
		{
			Filter_SendBan( packet->from );	// tell them we aren't listening...
			continue;
		} 

		// check for connectionless packet (0xffffffff) first
		if ( LittleLong( *(unsigned int *)packet->data ) == CONNECTIONLESS_HEADER )
		{
			packet->message.ReadLong();	// read the -1

			if ( net_showudp.GetInt() )
			{
				Msg("UDP <- %s: sz=%i OOB '%c' wire=%i\n", packet->from.ToString(), packet->size, packet->data[4], packet->wiresize );
			}

			handler->ProcessConnectionlessPacket( packet );
			continue;
		}

		// check for packets from connected clients
		
		INetChannel* netchan = NET_FindNetChannel( packet->from );

		if ( netchan )
		{
			netchan->ProcessPacket( packet, true );
		}
		/* else	// Not an error that may happen during connect or disconnect
		{
			Msg ("Sequenced packet without connection from %s\n" , packet->from.ToString() );
		}*/
	}
	g_NetScratchBuffers.Push( scratch );
}

void CNetworkSystem::NET_LogBadPacket(netpacket_t * packet)
{
	FileHandle_t fp;
	int i = 0;
	char filename[ MAX_OSPATH ];
	bool done = false;

	while ( i < 1000 && !done )
	{
		Q_snprintf( filename, sizeof( filename ), "badpacket%03i.dat", i );
		fp = g_pFileSystem->Open( filename, "rb" );
		if ( !fp )
		{
			fp = g_pFileSystem->Open( filename, "wb" );
			g_pFileSystem->Write( packet->data, packet->size, fp );
			done = true;
		}
		if ( fp )
		{
			g_pFileSystem->Close( fp );
		}
		i++;
	}

	if ( i < 1000 )
	{
		Msg( "Error buffer for %s written to %s\n", packet->from.ToString(), filename );
	}
	else
	{
		Msg( "Couldn't write error buffer, delete error###.dat files to make space\n" );
	}
}

int NET_SendToImpl( SOCKET s, const char FAR * buf, int len, const struct sockaddr FAR * to, int tolen, int iGameDataLength )
{
	int nSend = 0;
#if defined( _X360 )
	if ( X360SecureNetwork() )
	{
		// 360 uses VDP protocol to piggyback voice data across the network.
		// Two-byte VDP Header contains the number of game data bytes

		// NOTE: The header bytes *should* be swapped to network endian, however when communicating 
		// with XLSP servers (the only cross-platform communication possible with a secure network)
		// the server's network stack swaps the header at the receiving end.
		const int nVDPHeaderBytes = 2;
		Assert( len < (unsigned short)-1 );

		const unsigned short nDataBytes = iGameDataLength == -1 ? len : iGameDataLength;

		WSABUF buffers[2];
		buffers[0].len = nVDPHeaderBytes;
		buffers[0].buf = (char*)&nDataBytes;

		buffers[1].len = len;
		buffers[1].buf = const_cast<char*>( buf );

		WSASendTo( s, buffers, 2, (DWORD*)&nSend, 0, to, tolen, NULL, NULL );
	}
	else
#endif //defined( _X360 )
	{
		nSend = sendto( s, buf, len, 0, to, tolen );
	}

	return nSend;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : sock - 
//			s - 
//			buf - 
//			len - 
//			flags - 
//			to - 
//			tolen - 
// Output : int
//-----------------------------------------------------------------------------
bool CL_IsHL2Demo();
bool CL_IsPortalDemo();
int NET_SendTo( bool verbose, SOCKET s, const char FAR * buf, int len, const struct sockaddr FAR * to, int tolen, int iGameDataLength )
{	
	int nSend = 0;

	VPROF_BUDGET( "NET_SendTo", VPROF_BUDGETGROUP_OTHER_NETWORKING );
	
	// If it's 0.0.0.0:0, then it's a fake player + sv_stressbots and we've plumbed everything all 
	// the way through here, where we finally bail out.
	sockaddr_in *pInternetAddr = (sockaddr_in*)to;
#ifdef _WIN32
	if ( pInternetAddr->sin_addr.S_un.S_addr == 0
#else
	if ( pInternetAddr->sin_addr.s_addr == 0 
#endif
		&& pInternetAddr->sin_port == 0 )
	{		
		return len;
	}

	// Normally, we shouldn't need to write this data to the file, but it can help catch
	// out-of-sync errors earlier.
	if ( VCRGetMode() != VCR_Disabled && vcr_verbose.GetInt() )
	{
#if !defined( NO_VCR )
		VCRGenericValue( "senddata", &len, sizeof( len ) );
		VCRGenericValue( "senddata2", (char*)buf, len );
#endif
	}

	// Don't send anything out in VCR mode.. it just annoys other people testing in multiplayer.
	if ( VCRGetMode() != VCR_Playback )
	{
#ifndef SWDS
		if ( ( CL_IsHL2Demo() || CL_IsPortalDemo() ) && !g_pLocalNetworkSystem->NET_IsDedicated() )
		{
			Error( " " );
		}
#endif // _WIN32

		nSend = NET_SendToImpl
		( 
			s, 
			buf,
			len,
			to, 
			tolen, 
			iGameDataLength 
		);
	}

#if defined( _DEBUG )
	if ( verbose && 
		( nSend > 0 ) && 
		( len > MAX_ROUTABLE_PAYLOAD ) )
	{
		ConDMsg( "NET_SendTo:  Packet length (%i) > (%i) bytes\n", len, MAX_ROUTABLE_PAYLOAD );
	}
#endif
	return nSend;
}

#if defined( _DEBUG )

#include "filesystem.h"
#include "filesystem_engine.h"

//-----------------------------------------------------------------------------
// Purpose: 
// Output : char const
//-----------------------------------------------------------------------------
char const *NET_GetDebugFilename( char const *prefix )
{
	static char filename[ MAX_OSPATH ];

	int i;

	for ( i = 0; i < 10000; i++ )
	{
		Q_snprintf( filename, sizeof( filename ), "debug/%s%04i.dat", prefix, i );
		if ( g_pFileSystem->FileExists( filename ) )
			continue;

		return filename;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *filename - 
//			*buf - 
//			len - 
//-----------------------------------------------------------------------------
void NET_StorePacket( char const *filename, byte const *buf, int len )
{
	FileHandle_t fh;

	g_pFileSystem->CreateDirHierarchy( "debug/", "DEFAULT_WRITE_PATH" );
	fh = g_pFileSystem->Open( filename, "wb" );
	if ( FILESYSTEM_INVALID_HANDLE != fh )
	{
		g_pFileSystem->Write( buf, len, fh );
		g_pFileSystem->Close( fh );
	}
}

#endif // _DEBUG

struct SendQueueItem_t
{
	SendQueueItem_t() :
		m_pChannel( NULL ),
		m_Socket( (SOCKET)-1 )
	{
	}

	CNetChan	*m_pChannel;
	SOCKET		m_Socket;
	CUtlBuffer	m_Buffer;
	CUtlBuffer	m_To;
};

struct SendQueue_t
{
	SendQueue_t() : 
		m_nHostFrame( 0 )
	{
	}
	int									m_nHostFrame;
	CUtlLinkedList< SendQueueItem_t >	m_SendQueue;
};

static SendQueue_t g_SendQueue;

int NET_QueuePacketForSend( CNetChan *chan, bool verbose, SOCKET s, const char FAR *buf, int len, const struct sockaddr FAR * to, int tolen, uint32 msecDelay )
{
	// If net_queued_packet_thread was -1 at startup, then we don't even have a thread.
	if ( net_queued_packet_thread.GetInt() && g_pQueuedPackedSender->IsRunning() )
	{
		g_pQueuedPackedSender->QueuePacket( chan, s, buf, len, to, tolen, msecDelay );
	}
	else
	{
		Assert( chan );
		// Set up data structure
		SendQueueItem_t *sq = &g_SendQueue.m_SendQueue[ g_SendQueue.m_SendQueue.AddToTail() ];
		sq->m_Socket = s;
		sq->m_pChannel = chan;
		sq->m_Buffer.Put( (const void *)buf, len );
		sq->m_To.Put( (const void *)to, tolen );
		sq->m_pChannel->IncrementQueuedPackets();
	}
	
	return len;
}

void NET_SendQueuedPacket( SendQueueItem_t *sq )
{
	// Msg( "Send queued packet %d\n", sq->m_Buffer.TellPut() );
	NET_SendTo
	( 
		false, 
		sq->m_Socket, 
		( const char FAR * )sq->m_Buffer.Base(), 
		sq->m_Buffer.TellPut(), 
		( const struct sockaddr FAR * )sq->m_To.Base(), 
		sq->m_To.TellPut() , -1
	);

	sq->m_pChannel->DecrementQueuedPackets();
}

void NET_ClearQueuedPacketsForChannel( INetChannel *channel )
{
	CUtlLinkedList< SendQueueItem_t >& list = g_SendQueue.m_SendQueue;

	for ( unsigned short i = list.Head(); i != list.InvalidIndex();  )
	{
		unsigned short n = list.Next( i );
		SendQueueItem_t &e = list[ i ];
		if ( e.m_pChannel == channel )
		{
			list.Remove( i );
		}
		i = n;
	}
}

void CNetworkSystem::NET_SendQueuedPackets()
{
	// Only do this once per frame
	if ( g_pHost->Host_GetFrameCount() == g_SendQueue.m_nHostFrame)
		return;
	g_SendQueue.m_nHostFrame = g_pHost->Host_GetFrameCount();

	CUtlLinkedList< SendQueueItem_t >& list = g_SendQueue.m_SendQueue;

	int nRemaining = net_splitrate.GetInt();
	while ( nRemaining )
	{
		if ( list.IsValidIndex( list.Head() ) )
		{
			SendQueueItem_t *sq = &list[ list.Head() ];
			NET_SendQueuedPacket( sq );
			list.Remove( list.Head() );
			--nRemaining;
		}
		else
		{
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : sock - 
//			s - 
//			buf - 
//			len - 
//			flags - 
//			to - 
//			tolen - 
// Output : int
//-----------------------------------------------------------------------------
static ConVar net_splitpacket_maxrate( "net_splitpacket_maxrate", SPLITPACKET_MAX_DATA_BYTES_PER_SECOND, 0, "Max bytes per second when queueing splitpacket chunks", true, MIN_RATE, true, MAX_RATE );

int NET_SendLong( INetChannel *chan, INetSocket* sock, SOCKET s, const char FAR * buf, int len, const struct sockaddr FAR * to, int tolen, int nMaxRoutableSize )
{
	VPROF_BUDGET( "NET_SendLong", VPROF_BUDGETGROUP_OTHER_NETWORKING );

	CNetChan *netchan = dynamic_cast< CNetChan * >( chan );

	CNetSocket *netSocket= dynamic_cast<CNetSocket*>(sock);

	short nSplitSizeMinusHeader = nMaxRoutableSize - sizeof( SPLITPACKET );

	int nSequenceNumber = -1;
	if ( netchan )
	{
		nSequenceNumber = netchan->IncrementSplitPacketSequence();
	}
	else
	{
		nSequenceNumber = ThreadInterlockedIncrement( &netSocket->GetSplitPacketSequenceNumber() );
	}

	const char *sendbuf = buf;
	int sendlen = len;

	char			packet[ MAX_ROUTABLE_PAYLOAD ];
	SPLITPACKET		*pPacket = (SPLITPACKET *)packet;

	// Make pPacket data network endian correct
	pPacket->netID = LittleLong( NET_HEADER_FLAG_SPLITPACKET );
	pPacket->sequenceNumber = LittleLong( nSequenceNumber );
	pPacket->nSplitSize = LittleShort( nSplitSizeMinusHeader );
	
	int nPacketCount = (sendlen + nSplitSizeMinusHeader - 1) / nSplitSizeMinusHeader;

#if defined( _DEBUG )
	if ( net_savelargesplits.GetInt() != -1 && nPacketCount >= net_savelargesplits.GetInt() )
	{
		char const *filename = NET_GetDebugFilename( "splitpacket" );
		if ( filename )
		{
			Msg( "Saving split packet of %i bytes and %i packets to file %s\n",
				sendlen, nPacketCount, filename );

			NET_StorePacket( filename, (byte const *)sendbuf, sendlen );
		}
		else
		{
			Msg( "Too many files in debug directory, clear out old data!\n" );
		}
	}
#endif

	int nBytesLeft = sendlen;
	int nPacketNumber = 0;
	int nTotalBytesSent = 0;
	int nFragmentsSent = 0;

	while ( nBytesLeft > 0 )
	{
		int size = min( (int)nSplitSizeMinusHeader, nBytesLeft );

		pPacket->packetID = LittleShort( (short)(( nPacketNumber << 8 ) + nPacketCount) );
		
		Q_memcpy( packet + sizeof(SPLITPACKET), sendbuf + (nPacketNumber * nSplitSizeMinusHeader), size );
		
		int ret = 0;

		// Setting net_queued_packet_thread to NET_QUEUED_PACKET_THREAD_DEBUG_VALUE goes into a mode where all packets are queued.. can be used to stress-test it.
		// Linux threads aren't prioritized well enough for this to work well (i.e. the queued packet thread doesn't get enough
		// attention to flush itself well). The behavior the queue fixes is that if you send too many DP packets
		// without giving up your timeslice, it'll just discard the 7th and later packets until you Sleep() (issue might be on client recipient side, need to
		// snif packets to double check)

		if ( netchan && (nFragmentsSent >= net_splitrate.GetInt() || net_queued_packet_thread.GetInt() == NET_QUEUED_PACKET_THREAD_DEBUG_VALUE) )
		{
			// Don't let this rate get too high (SPLITPACKET_MAX_DATA_BYTES_PER_SECOND == 15000 bytes/sec) 
			// or user's won't be able to receive all of the parts since they'll be too close together.
			/// XXX(JohnS): (float)cv.GetInt() is just preserving what this was doing before to avoid changing the
			///             semantics of this convar
			float flMaxSplitpacketDataRateBytesPerSecond = min( (float)netchan->GetDataRate(), (float)net_splitpacket_maxrate.GetInt() );

			// Calculate the delay (measured from now) for when this packet should be sent.
			uint32 delay = (int)( 1000.0f * ( (float)( nPacketNumber * ( nMaxRoutableSize + UDP_HEADER_SIZE ) ) / flMaxSplitpacketDataRateBytesPerSecond ) + 0.5f );

			ret = NET_QueuePacketForSend( netchan, false, s, packet, size + sizeof(SPLITPACKET), to, tolen, delay );
		}
		else
		{
			// Also, we send the first packet no matter what
			// w/o a netchan, if there are too many splits, its possible the packet can't be delivered.  However, this would only apply to out of band stuff like
			//  server query packets, which should never require splitting anyway.
			ret = NET_SendTo( false, s, packet, size + sizeof(SPLITPACKET), to, tolen, -1 );
		}

		// First split send
		++nFragmentsSent;

		if ( ret < 0 )
		{
			return ret;
		}

		if ( ret >= size )
		{
			nTotalBytesSent += size;
		}

		nBytesLeft -= size;
		++nPacketNumber;

		// Always bitch about split packets in debug
		if ( net_showsplits.GetInt() && net_showsplits.GetInt() != 2 )
		{
			netadr_t adr;
			
			adr.SetFromSockadr( (struct sockaddr*)to );

			Msg( "--> [%s] Split packet %4i/%4i seq %5i size %4i mtu %4i to %s [ total %4i ]\n",
				sock->Net_GetSocketName(),
				nPacketNumber, 
				nPacketCount, 
				nSequenceNumber,
				size,
				nMaxRoutableSize,
				adr.ToString(),
				sendlen );
		}
	}
	
	return nTotalBytesSent;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : sock - 
//			length - 
//			*data - 
//			to - 
// Output : void NET_SendPacket
//-----------------------------------------------------------------------------

int CNetSocket::NET_SendPacket ( INetChannel *chan, const netadr_t &to, const unsigned char *data, int length, bf_write *pVoicePayload /* = NULL */, bool bUseCompression /*=false*/ )
{
	VPROF_BUDGET( "NET_SendPacket", VPROF_BUDGETGROUP_OTHER_NETWORKING );
	ETWSendPacket( to.ToString() , length , 0 , 0 );

	int		ret;
	struct sockaddr	addr;
	int		net_socket;

	if ( net_showudp.GetInt() && (*(unsigned int*)data == CONNECTIONLESS_HEADER) )
	{
		Assert( !bUseCompression );
		Msg("UDP -> %s: sz=%i OOB '%c'\n", to.ToString(), length, data[4] );
	}

	if ( !g_pLocalNetworkSystem->NET_IsMultiplayer() || to.type == NA_LOOPBACK || ( to.IsLocalhost() && !net_usesocketsforloopback.GetBool() ) )
	{
		Assert( !pVoicePayload );

		NET_SendLoopPacket (length, data, to);
		return length;
	}

	if ( to.type == NA_BROADCAST )
	{
		net_socket = hUDP;
		if (!net_socket)
			return length;
	}
	else if ( to.type == NA_IP )
	{
		net_socket = hUDP;
		if (!net_socket)
			return length;
	}
	else
	{
		DevMsg("NET_SendPacket: bad address type (%i)\n", to.type );
		return length;
	}

	if ( (droppackets.GetInt() < 0)  && this == g_pLocalNetworkSystem->m_pClientSocket )
	{
		droppackets.SetValue( droppackets.GetInt() + 1 );
		return length;
	}

	if ( fakeloss.GetFloat() > 0.0f )
	{
		// simulate sending this packet
		if (RandomInt(0,100) <= (int)fakeloss.GetFloat())
			return length;
	}

	to.ToSockadr ( &addr );

	MEM_ALLOC_CREDIT();
	CUtlMemoryFixedGrowable< byte, NET_COMPRESSION_STACKBUF_SIZE > memCompressed( NET_COMPRESSION_STACKBUF_SIZE );
	CUtlMemoryFixedGrowable< byte, NET_COMPRESSION_STACKBUF_SIZE > memCompressedVoice( NET_COMPRESSION_STACKBUF_SIZE );

	int iGameDataLength = pVoicePayload ? length : -1;

	bool bWroteVoice = false;
	unsigned int nVoiceBytes = 0;

	if ( pVoicePayload )
	{
		VPROF_BUDGET( "NET_SendPacket_CompressVoice", VPROF_BUDGETGROUP_OTHER_NETWORKING );
		unsigned int nCompressedLength = COM_GetIdealDestinationCompressionBufferSize_Snappy( pVoicePayload->GetNumBytesWritten() );
		memCompressedVoice.EnsureCapacity( nCompressedLength + sizeof( unsigned short ) );

		byte *pVoice = (byte *)memCompressedVoice.Base();

		unsigned short usVoiceBits = pVoicePayload->GetNumBitsWritten();
		*( unsigned short * )pVoice = LittleShort( usVoiceBits );
		pVoice += sizeof( unsigned short );
		
		byte *pOutput = NULL;
		if ( net_compressvoice.GetBool() )
		{
			if ( COM_BufferToBufferCompress_Snappy( pVoice, &nCompressedLength, pVoicePayload->GetData(), pVoicePayload->GetNumBytesWritten()
				&& ( (int)nCompressedLength < pVoicePayload->GetNumBytesWritten() ) ) )
			{
				pOutput = pVoice;
			}
		}
		if ( !pOutput )
		{
			Q_memcpy( pVoice, pVoicePayload->GetData(), pVoicePayload->GetNumBytesWritten() );
		}

		nVoiceBytes = nCompressedLength + sizeof( unsigned short );
	}

	if ( bUseCompression )
	{
		VPROF_BUDGET( "NET_SendPacket_Compress", VPROF_BUDGETGROUP_OTHER_NETWORKING );
		unsigned int nCompressedLength = COM_GetIdealDestinationCompressionBufferSize_Snappy( length );
	
		memCompressed.EnsureCapacity( nCompressedLength + nVoiceBytes + sizeof( unsigned int ) );

		*(int *)memCompressed.Base() = LittleLong( NET_HEADER_FLAG_COMPRESSEDPACKET );

		if ( COM_BufferToBufferCompress_Snappy( memCompressed.Base() + sizeof( unsigned int ), &nCompressedLength, data, length )
			&& (int)nCompressedLength < length )
		{
			data	= memCompressed.Base();
			length	= nCompressedLength + sizeof( unsigned int );

			if ( pVoicePayload && pVoicePayload->GetNumBitsWritten() > 0 )
			{
				byte *pVoice = (byte *)memCompressed.Base() + length;
				Q_memcpy( pVoice, memCompressedVoice.Base(), nVoiceBytes );
			}
			
			iGameDataLength = length;

			length += nVoiceBytes;

			bWroteVoice = true;
		}
	}
	
	if ( !bWroteVoice && pVoicePayload && pVoicePayload->GetNumBitsWritten() > 0 )
	{
		memCompressed.EnsureCapacity( length + nVoiceBytes );

		byte *pVoice = (byte *)memCompressed.Base();
		Q_memcpy( pVoice, (const void *)data, length );
		pVoice += length;
		Q_memcpy( pVoice, memCompressedVoice.Base(), nVoiceBytes );
		data	= memCompressed.Base();

		length  += nVoiceBytes;
	}

	// Do we need to break this packet up?
	int nMaxRoutable = MAX_ROUTABLE_PAYLOAD;
	if ( chan )
	{
		nMaxRoutable = clamp( chan->GetMaxRoutablePayloadSize(), MIN_USER_MAXROUTABLE_SIZE, min( sv_maxroutable.GetInt(), MAX_USER_MAXROUTABLE_SIZE ) );
	}

	if ( length <= nMaxRoutable && 
		!(net_queued_packet_thread.GetInt() == NET_QUEUED_PACKET_THREAD_DEBUG_VALUE && chan ) )	
	{
		// simple case, small packet, just send it
		ret = NET_SendTo( true, net_socket, (const char *)data, length, &addr, sizeof(addr), iGameDataLength );
	}
	else
	{
		// split packet into smaller pieces
		ret = NET_SendLong( chan, this, net_socket, (const char *)data, length, &addr, sizeof(addr), nMaxRoutable );
	}
	
	if (ret == -1)
	{
		g_pLocalNetworkSystem->NET_GetLastError();
		
		// wouldblock is silent
		if (g_pLocalNetworkSystem->net_error == WSAEWOULDBLOCK )
			return 0;

		if (g_pLocalNetworkSystem->net_error == WSAECONNRESET )
			return 0;

		// some PPP links dont allow broadcasts
		if ( (g_pLocalNetworkSystem->net_error == WSAEADDRNOTAVAIL) && ( to.type == NA_BROADCAST ) )
			return 0;

		ConDMsg ("NET_SendPacket Warning: %s : %s\n", g_pLocalNetworkSystem->NET_ErrorString(g_pLocalNetworkSystem->net_error), to.ToString() );
		ret = length;
	}
	

	return ret;
}

void CNetSocket::NET_OutOfBandPrintf(const netadr_t &adr, const char *format, ...)
{
	va_list		argptr;
	char		string[MAX_ROUTABLE_PAYLOAD];
	
	*(unsigned int*)string = CONNECTIONLESS_HEADER;

	va_start (argptr, format);
	Q_vsnprintf (string+4, sizeof( string ) - 4, format,argptr);
	va_end (argptr);

	int length = Q_strlen(string+4) + 5;

	NET_SendPacket ( NULL, adr, (byte *)string, length );
}

void CNetSocket::NET_ClosePending() {
	// shut down all pending sockets
	AUTO_LOCK_FM(s_PendingSockets);
	for (int j = 0; j < s_PendingSockets.Count(); j++)
	{
		g_pLocalNetworkSystem->NET_CloseSocket(s_PendingSockets[j].newsock);
	}

	s_PendingSockets.RemoveAll();
}

/*
====================
NET_CloseAllSockets
====================
*/
void CNetworkSystem::NET_CloseAllSockets (void)
{
	// shut down any existing and open sockets
	for (int i=0 ; i<net_sockets.Count() ; i++)
	{
		if ( net_sockets[i]->nPort )
		{
			NET_CloseSocket( net_sockets[i]->hUDP );
			NET_CloseSocket( net_sockets[i]->hTCP );

			net_sockets[i]->nPort = 0;
			net_sockets[i]->bListening = false;
			net_sockets[i]->hUDP = 0;
			net_sockets[i]->hTCP = 0;
		}

		net_sockets[i]->NET_ClosePending();
		delete net_sockets[i];
	}

	m_DefaultSocketCount = 0;
}

/*
====================
NET_FlushAllSockets
====================
*/
void CNetworkSystem::NET_FlushAllSockets( void )
{
	// drain any packets that my still lurk in our incoming queue
	char data[2048];
	struct sockaddr	from;
	int	fromlen = sizeof(from);
	
	for (int i=0 ; i<net_sockets.Count() ; i++)
	{
		if ( net_sockets[i]->hUDP )
		{
			int bytes = 1;

			// loop until no packets are pending anymore
			while ( bytes > 0  )
			{
				bytes = VCRHook_recvfrom( net_sockets[i]->hUDP, data, sizeof(data), 0, (struct sockaddr *)&from, (int *)&fromlen );
			}
		}
	}
}



bool CNetSocket::OpenSocketInternal( int nSetPort, int nDefaultPort, const char *pName, int nProtocol, bool bTryAny,
								int flags )
{
	int port = nSetPort ? nSetPort : nDefaultPort;
	int *handle = NULL;

	if( nProtocol == IPPROTO_TCP )
	{
		handle = &hTCP;
	}
	else if ( nProtocol == IPPROTO_UDP || nProtocol == IPPROTO_VDP )
	{
		handle = &hUDP;
	}
	else
	{
		Sys_Error( "Unrecognized protocol type %d", nProtocol );
		return false;
	}

	if ( !nPort )
	{
		const char *netinterface = ( flags & OSOCKET_FLAG_USE_IPNAME ) ? ipname.GetString() : NULL;

		*handle = g_pLocalNetworkSystem->NET_OpenSocket (netinterface, port, nProtocol );
		if ( !*handle && bTryAny )
		{
			port = PORT_ANY;	// try again with PORT_ANY
			*handle = g_pLocalNetworkSystem->NET_OpenSocket ( netinterface, port, nProtocol );
		}

		if ( !*handle )
		{
			if ( flags & OSOCKET_FLAG_FAIL )
				Sys_Exit( "Couldn't allocate any %s IP port", pName );
			return false;
		}

		nPort = port;
	}
	else
	{
		Msg( "WARNING: NET_OpenSockets: %s port %i already open.\n", pName, nPort );
		return false;
	}

	return ( nPort != 0 );
}

/*
====================
NET_OpenSockets
====================
*/
void CNetworkSystem::NET_OpenSockets (void)
{	
	// Xbox 360 uses VDP protocol to combine encrypted game data with clear voice data
	const int nProtocol = X360SecureNetwork() ? IPPROTO_VDP : IPPROTO_UDP;

	m_pClientSocket = new CNetSocket("ClientSocket");
	m_pClientSocket->OpenSocketInternal(clientport.GetInt(), PORT_SERVER, "client", nProtocol, true);
	net_sockets.AddToTail(m_pClientSocket);
	m_DefaultSocketCount++;

	m_pServerSocket = new CNetSocket("ServerSocket");
	m_pServerSocket->OpenSocketInternal( hostport.GetInt(), PORT_SERVER, "server", nProtocol, false );
	net_sockets.AddToTail(m_pServerSocket);
	m_DefaultSocketCount++;

	m_pServerSocket->SetLoopBackSocket(m_pClientSocket);
	m_pClientSocket->SetLoopBackSocket(m_pServerSocket);

	if ( !net_nohltv )
	{
		m_pHLTVSocket = new CNetSocket("HLTVSocket");
		m_pHLTVSocket->OpenSocketInternal( hltvport.GetInt(), PORT_HLTV, "hltv", nProtocol, false );
		net_sockets.AddToTail(m_pHLTVSocket);
		m_DefaultSocketCount++;
	}

	if ( IsX360() )
	{
		m_pMatchMakingSocket = new CNetSocket("MatchMakingSocket");
		m_pMatchMakingSocket->OpenSocketInternal( matchmakingport.GetInt(), PORT_MATCHMAKING, "matchmaking", nProtocol, false );
		net_sockets.AddToTail(m_pMatchMakingSocket);
		m_DefaultSocketCount++;

		m_pSystemLinkSocket = new CNetSocket("SystemLinkSocket");
		m_pSystemLinkSocket->OpenSocketInternal( systemlinkport.GetInt(), PORT_SYSTEMLINK, "systemlink", IPPROTO_UDP, false );
		net_sockets.AddToTail(m_pSystemLinkSocket);
		m_DefaultSocketCount++;
	}

#ifdef LINUX
	// On Linux, if you bind to a specific address then you will NOT receive broadcast messages.
	// This means that if you do a +ip X.X.X.X, your game will not show up on the LAN server browser page.
	// To workaround this, if the user has specified sv_lan and an IP address, we open an INADDR_ANY port.
	// See http://developerweb.net/viewtopic.php?id=5722 for more information.
	extern ConVar sv_lan;
	if ( sv_lan.GetBool() )
	{
		const char *net_interface = ipname.GetString();
		// If net_interface was specified and it's not localhost...
		if ( net_interface[ 0 ] && ( Q_strcmp( net_interface, "localhost" ) != 0 ) )
		{
			// From clientdll/matchmaking/ServerList.cpp, the ports queried are:
			//   27015 - 27020, 26900 - 26905
			//   4242: RDKF, 27215: Lost Planet
			static int s_ports[] =
			{
				26900, 26901, 26902, 26903, 26904, 26905,
				27015, 27016, 27017, 27018, 27019, 27020
			};

			for ( size_t iport = 0; iport < ARRAYSIZE( s_ports ); iport++ )
			{
				bool bPortUsed = false;
				for ( int i = NS_CLIENT; i < NS_SVLAN; i++ )
				{
					// Move along if this port is already used.
					if ( net_sockets[ i ].nPort == s_ports[ iport ] )
					{
						bPortUsed = true;
						break;
					}
				}

				if ( !bPortUsed )
				{
					// Try to open the socket and break if we succeeded.
					if ( OpenSocketInternal( NS_SVLAN, s_ports[ iport ], PORT_SERVER, "lan", nProtocol, false, 0 ) )
						break;
				}
			}

			if ( net_sockets[ NS_SVLAN ].nPort )
				Msg( "Opened sv_lan port %d\n", net_sockets[ NS_SVLAN ].nPort );
			else
				Warning( "%s, Failed to open sv_lan port.\n", __FUNCTION__ );
		}
	}
#endif // LINUX
}

INetSocket* CNetworkSystem::NET_AddExtraSocket( int port )
{
	CNetSocket* sock = new CNetSocket("ExtraSocket"+ net_sockets.Count());

	int newSocket = net_sockets.AddToTail(sock);

	//Q_memset( &net_sockets[newSocket], 0, sizeof(netsocket_t) );

	sock->OpenSocketInternal( port, PORT_ANY, "extra", IPPROTO_UDP, true );

	//net_packets.EnsureCount( newSocket+1 );
	//net_splitpackets.EnsureCount( newSocket+1 );

	return sock;
}

void CNetworkSystem::NET_RemoveAllExtraSockets()
{
	for (int i= m_DefaultSocketCount; i<net_sockets.Count() ; i++)
	{
		if ( net_sockets[i]->nPort )
		{
			NET_CloseSocket( net_sockets[i]->hUDP );
			NET_CloseSocket( net_sockets[i]->hTCP );
		}
		delete net_sockets[i];
	}
	net_sockets.RemoveMultiple(m_DefaultSocketCount, net_sockets.Count()- m_DefaultSocketCount);

	Assert( net_sockets.Count() == MAX_SOCKETS );
}

unsigned short CNetSocket::NET_GetUDPPort()
{
	//if ( socket < 0 || socket >= net_sockets.Count() )
	//	return 0;

	return nPort;
}


/*
================
NET_GetLocalAddress

Returns the servers' ip address as a string.
================
*/
void CNetworkSystem::NET_GetLocalAddress (void)
{
	net_local_adr.Clear();

	if ( net_noip )
	{
		Msg("TCP/UDP Disabled.\n");
	}
	else
	{
		char	buff[512];

		// If we have changed the ip var from the command line, use that instead.
		if ( Q_strcmp(ipname.GetString(), "localhost") )
		{
			Q_strncpy(buff, ipname.GetString(), sizeof( buff ) );	// use IP set with ipname
		}
		else
		{
			gethostname( buff, sizeof(buff) );	// get own IP address
			buff[sizeof(buff)-1] = 0;			// Ensure that it doesn't overrun the buffer
		}

		NET_StringToAdr (buff, &net_local_adr);

		int ipaddr = ( net_local_adr.ip[0] << 24 ) + 
					 ( net_local_adr.ip[1] << 16 ) + 
					 ( net_local_adr.ip[2] << 8 ) + 
					   net_local_adr.ip[3];

		hostip.SetValue( ipaddr );
	}
}


/*
====================
NET_IsConfigured

Is winsock ip initialized?
====================
*/
bool CNetworkSystem::NET_IsMultiplayer( void )
{
	return net_multiplayer;
}

bool CNetworkSystem::NET_IsDedicated( void )
{
	return net_dedicated;
}

#ifdef _X360
#include "iengine.h"
static FileHandle_t g_fh;
void NET_LogServerStatus( void )
{
	if ( !g_fh )
		return;

	static float fNextTime = 0.f;
	float fCurrentTime = eng->GetCurTime();

	if ( fCurrentTime >= fNextTime )
	{
		fNextTime = fCurrentTime + net_loginterval.GetFloat();
	}
	else
	{
		return;
	}

	AUTO_LOCK_FM( s_NetChannels );
	int numChannels = s_NetChannels.Count();

	if ( numChannels == 0 )
	{
		ConMsg( "No active net channels.\n" );
		return;
	}

	enum
	{
		NET_LATENCY,
		NET_LOSS,
		NET_PACKETS_IN,
		NET_PACKETS_OUT,
		NET_CHOKE_IN,
		NET_CHOKE_OUT,
		NET_FLOW_IN,
		NET_FLOW_OUT,
		NET_TOTAL_IN,
		NET_TOTAL_OUT,
		NET_LAST,
	};
	float fStats[NET_LAST] = {0.f};

	for ( int i = 0; i < numChannels; ++i )
	{
		INetChannel *chan = s_NetChannels[i];
		fStats[NET_LATENCY] += chan->GetAvgLatency(FLOW_OUTGOING);
		fStats[NET_LOSS] += chan->GetAvgLoss(FLOW_INCOMING);
		fStats[NET_PACKETS_IN] += chan->GetAvgPackets(FLOW_INCOMING);
		fStats[NET_PACKETS_OUT] += chan->GetAvgPackets(FLOW_OUTGOING);
		fStats[NET_CHOKE_IN] += chan->GetAvgChoke(FLOW_INCOMING);
		fStats[NET_CHOKE_OUT] += chan->GetAvgChoke(FLOW_OUTGOING);
		fStats[NET_FLOW_IN] += chan->GetAvgData(FLOW_INCOMING);
		fStats[NET_FLOW_OUT] += chan->GetAvgData(FLOW_OUTGOING);
		fStats[NET_TOTAL_IN] += chan->GetTotalData(FLOW_INCOMING);
		fStats[NET_TOTAL_OUT] += chan->GetTotalData(FLOW_OUTGOING);
	}

	for ( int i = 0; i < NET_LAST; ++i )
	{
		fStats[i] /= numChannels;
	}

	const unsigned int size = 128;
	char msg[size];
	Q_snprintf( msg, size, "%.0f,%d,%.0f,%.0f,%.0f,%.1f,%.1f,%.1f,%.1f,%.1f\n", 
				fCurrentTime,
				numChannels,
				fStats[NET_LATENCY],
				fStats[NET_LOSS],
				fStats[NET_PACKETS_IN], 
				fStats[NET_PACKETS_OUT],
				fStats[NET_FLOW_IN]/1024.0f, 
				fStats[NET_FLOW_OUT]/1024.0f,
				fStats[NET_CHOKE_IN],
				fStats[NET_CHOKE_OUT]
			 );

	g_pFileSystem->Write( msg, Q_strlen( msg ), g_fh );
}

void NET_LogServerCallback( IConVar *pConVar, const char *pOldString, float flOldValue )
{
	ConVarRef var( pConVar );

	if ( var.GetBool() )
	{
		if ( g_fh )
		{
			g_pFileSystem->Close( g_fh );
			g_fh = 0;
		}

		g_fh = g_pFileSystem->Open( "dump.csv", "wt" );
		if ( !g_fh )
		{
			Msg( "Failed to open log file\n" );
			pConVar->SetValue( 0 );
			return;
		}

		char msg[128];
		Q_snprintf( msg, 128, "Time,Channels,Latency,Loss,Packets In,Packets Out,Flow In(kB/s),Flow Out(kB/s),Choke In,Choke Out\n" );
		g_pFileSystem->Write( msg, Q_strlen( msg ), g_fh );
	}
	else
	{
		if ( g_fh )
		{
			g_pFileSystem->Close( g_fh );
			g_fh = 0;
		}
	}
}
#endif

/*
====================
NET_SetTime

Updates net_time
====================
*/
void CNetworkSystem::NET_SetTime( double flRealtime )
{
	static double s_last_realtime = 0;

	double frametime = flRealtime - s_last_realtime;
	s_last_realtime = flRealtime;

	if ( frametime > 1.0f )
	{
		// if we have very long frame times because of loading stuff
		// don't apply that to net time to avoid unwanted timeouts
		frametime = 1.0f;
	}
	else if ( frametime < 0.0f )
	{
		frametime = 0.0f;
	}

	// adjust network time so fakelag works with host_timescale
	net_time += frametime * host_timescale.GetFloat();
}

/*
====================
NET_RunFrame

RunFrame must be called each system frame before reading/sending on any socket
====================
*/
void CNetworkSystem::NET_RunFrame( double flRealtime )
{
	NET_SetTime( flRealtime );

	RCONServer().RunFrame();

#ifdef ENABLE_RPT
	RPTServer().RunFrame();
#endif // ENABLE_RPT

#ifndef SWDS
	RCONClient().RunFrame();
#ifdef ENABLE_RPT
	RPTClient().RunFrame();
#endif // ENABLE_RPT

#endif // SWDS

#ifdef _X360
	if ( net_logserver.GetInt() )
	{
		NET_LogServerStatus();
	}
	g_pMatchmaking->RunFrame();
#endif
	if ( !NET_IsMultiplayer() || net_notcp )
		return;

	// process TCP sockets:
	for ( int i=0; i< net_sockets.Count(); i++ )
	{
		if ( net_sockets[i]->hTCP && net_sockets[i]->bListening )
		{
			net_sockets[i]->NET_ProcessListen();
			net_sockets[i]->NET_ProcessPending();
		}
	}

}

void CNetSocket::NET_ClearLoopbackBuffer()
{
	loopback_t* loop;

	while (s_LoopBack.PopItem(&loop))
	{
		if (loop->data && loop->data != loop->defbuffer)
		{
			delete[] loop->data;
		}
		delete loop;
	}
}

void CNetworkSystem::NET_ClearLoopbackBuffers()
{
	for (int i = 0; i < net_sockets.Count(); i++)
	{
		net_sockets[i]->NET_ClearLoopbackBuffer();
	}
}

void CNetworkSystem::NET_ConfigLoopbackBuffers( bool bAlloc )
{
	NET_ClearLoopbackBuffers();
}

/*
====================
NET_Config

A single player game will only use the loopback code
====================
*/

void CNetworkSystem::NET_Config ( void )
{
	// free anything
	NET_CloseAllSockets();	// close all UDP/TCP sockets

	net_time = 0.0f;

	// now reconfiguare

	if ( net_multiplayer )
	{	
		// don't allocate loopback buffers
		NET_ConfigLoopbackBuffers( false );

		// get localhost IP address
		NET_GetLocalAddress();

		// reopen sockets if in MP mode
		NET_OpenSockets();

		// setup the rcon server sockets
		if ( net_dedicated || CommandLine()->FindParm( "-usercon" ) )
		{
			netadr_t rconAddr = net_local_adr;
			rconAddr.SetPort( m_pServerSocket->nPort );
			RCONServer().SetAddress( rconAddr.ToString() );
			RCONServer().CreateSocket();
		}
	}
	else
	{
		// allocate loopback buffers
		NET_ConfigLoopbackBuffers( true );
	}

	Msg( "Network: IP %s, mode %s, dedicated %s, ports %i SV / %i CL\n", 
		net_local_adr.ToString(true), net_multiplayer?"MP":"SP", net_dedicated?"Yes":"No", 
		m_pServerSocket->nPort, m_pClientSocket->nPort );
}

/*
====================
NET_SetDedicated

A single player game will only use the loopback code
====================
*/

void CNetworkSystem::NET_SetDedicated ()
{
	if ( net_noip )
	{
		Msg( "Warning! Dedicated not possible with -noip parameter.\n");
		return;		
	}

	net_dedicated = true;
}



void CNetworkSystem::NET_SetMutiplayer(bool multiplayer)
{
	if ( net_noip && multiplayer )
	{
		Msg( "Warning! Multiplayer mode not available with -noip parameter.\n");
		return;		
	}

	if ( net_dedicated && !multiplayer )
	{
		Msg( "Warning! Singleplayer mode not available on dedicated server.\n");
		return;		
	}

	// reconfigure if changed
	if ( net_multiplayer != multiplayer )
	{
		net_multiplayer = multiplayer;
		NET_Config();
	}

	// clear loopback buffer in single player mode
	if ( !multiplayer )
	{
		NET_ClearLoopbackBuffers();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bIsDedicated - 
//-----------------------------------------------------------------------------
void CNetworkSystem::NET_Init( bool bIsDedicated )
{

	if ( CommandLine()->FindParm( "-NoQueuedPacketThread" ) )
		Warning( "Found -NoQueuedPacketThread, so no queued packet thread will be created.\n" );
	else
		g_pQueuedPackedSender->Setup();


	if (CommandLine()->FindParm("-nodns"))
	{
		net_nodns = true;
	}

	if (CommandLine()->FindParm("-usetcp"))
	{
		net_notcp = false;
	}

	if (CommandLine()->FindParm("-nohltv"))
	{
		net_nohltv = true;
	}

	if (CommandLine()->FindParm("-noip"))
	{
		net_noip = true;
	}
	else
	{
#if defined(_WIN32)

#if defined(_X360)
		XNetStartupParams xnsp;
		memset( &xnsp, 0, sizeof( xnsp ) );
		xnsp.cfgSizeOfStruct = sizeof( XNetStartupParams );
		if ( X360SecureNetwork() )
		{
			Msg( "Xbox 360 network is Secure\n" );
		}
		else
		{
			// Allow cross-platform communication
			xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
			Msg( "Xbox 360 network is Unsecure\n" );
		}

		INT err = XNetStartup( &xnsp );
		if ( err )
		{
			ConMsg( "Error! Failed to set XNET Security Bypass.\n");
		}
		err = XOnlineStartup();
		if ( err != ERROR_SUCCESS )
		{
			ConMsg( "Error! XOnlineStartup failed.\n");
		}
#else
		// initialize winsock 2.0
		WSAData wsaData;
		if ( WSAStartup( MAKEWORD(2,0), &wsaData ) != 0 )
		{
			ConMsg( "Error! Failed to load network socket library.\n");
			net_noip = true;
		}
#endif	// _X360
#endif	// _WIN32
	}

	COMPILE_TIME_ASSERT( SVC_LASTMSG < (1<<NETMSG_TYPE_BITS) );
	COMPILE_TIME_ASSERT( MAX_FILE_SIZE < (1<<MAX_FILE_SIZE_BITS) );

	net_time = 0.0f;

	
	int hPort = CommandLine()->ParmValue( "-port", -1 );
	if ( hPort == -1 )
	{
		hPort = CommandLine()->ParmValue( "+port", -1 ); // check if they used +port by mistake
	}

	if ( hPort != -1 )
	{
		hostport.SetValue( hPort );
	}

	// clear static stuff
	//net_sockets.EnsureCount( MAX_SOCKETS );
	//net_packets.EnsureCount( MAX_SOCKETS );
	//net_splitpackets.EnsureCount( MAX_SOCKETS );

	//for ( int i = 0; i < MAX_SOCKETS; ++i )
	//{
	//	//s_pLagData[i] = NULL;
	//	Q_memset( &net_sockets[i], 0, sizeof(CNetSocket) );
	//}

	const char *ip = CommandLine()->ParmValue( "-ip" );

	if ( ip ) // if they had a command line option for IP
	{
		ipname.SetValue( ip );  // update the cvar right now, this will get overwritten by "stuffcmds" later
	}

	if ( bIsDedicated )
	{
		// set dedicated MP mode
		NET_SetDedicated();
	}
	else
	{
		// set SP mode
		NET_ConfigLoopbackBuffers( true );
	}
}

/*
====================
NET_Shutdown

====================
*/
void CNetworkSystem::NET_Shutdown (void)
{
	int nError = 0;

	for (int i = 0; i < net_sockets.Count(); i++)
	{
		net_sockets[i]->NET_ClearLagData();
	}

	g_pQueuedPackedSender->Shutdown();

	net_multiplayer = false;
	net_dedicated = false;

	NET_ConfigLoopbackBuffers(false);
	NET_CloseAllSockets();

#if defined(_WIN32)
	if ( !net_noip )
	{
		nError = WSACleanup();
		if ( nError )
		{
			Msg("Failed to complete WSACleanup = 0x%x.\n", nError );
		}
#if defined(_X360)
		nError = XOnlineCleanup();
		if ( nError != ERROR_SUCCESS )
		{
			Msg( "Warning! Failed to complete XOnlineCleanup = 0x%x.\n", nError );
		}
#endif	// _X360
	}
#endif	// _WIN32

	Assert( s_NetChannels.Count() == 0 );
	Assert( s_PendingSockets.Count() == 0);
}

void CNetSocket::NET_PrintChannelStatus( INetChannel * chan )
{
	Msg( "NetChannel '%s':\n", chan->GetName() );
	Msg( "- remote IP: %s %s\n", chan->GetAddress(), chan->IsPlayback()?"(Demo)":"" );
	Msg( "- online: %s\n", COM_FormatSeconds( chan->GetTimeConnected() ) );
	Msg( "- reliable: %s\n", chan->HasPendingReliableData()?"pending data":"available" );
	Msg( "- latency: %.1f, loss %.2f\n", chan->GetAvgLatency(FLOW_OUTGOING), chan->GetAvgLoss(FLOW_INCOMING) );
	Msg( "- packets: in %.1f/s, out %.1f/s\n", chan->GetAvgPackets(FLOW_INCOMING), chan->GetAvgPackets(FLOW_OUTGOING) );
	Msg( "- choke: in %.2f, out %.2f\n", chan->GetAvgChoke(FLOW_INCOMING), chan->GetAvgChoke(FLOW_OUTGOING) );
	Msg( "- flow: in %.1f, out %.1f kB/s\n", chan->GetAvgData(FLOW_INCOMING)/1024.0f, chan->GetAvgData(FLOW_OUTGOING)/1024.0f );
	Msg( "- total: in %.1f, out %.1f MB\n\n", (float)chan->GetTotalData(FLOW_INCOMING)/(1024*1024), (float)chan->GetTotalData(FLOW_OUTGOING)/(1024*1024) );
}

CON_COMMAND( net_channels, "Shows net channel info" )
{
	for (int s = 0; s < g_pLocalNetworkSystem->GetSocketCount(); s++)
	{
		INetSocket* socket = g_pLocalNetworkSystem->GetSocket(s);

		int numChannels = socket->GetNetChannelCount();

		if (numChannels == 0)
		{
			ConMsg("No active net channels.\n");
			continue;
		}

		AUTO_LOCK_FM(socket->GetNetChannels());
		for (int i = 0; i < numChannels; i++)
		{
			socket->NET_PrintChannelStatus(socket->GetNetChannel(i));
		}
	}
}

CON_COMMAND( net_start, "Inits multiplayer network sockets" )
{
	g_pLocalNetworkSystem->NET_SetMutiplayer(true);
	s_NetworkSystem.NET_Config();
}

CON_COMMAND( net_status, "Shows current network status" )
{
	ConMsg("Net status for host %s:\n",
		net_local_adr.ToString(true));
	ConMsg("- Config: %s, %s \n",
		g_pLocalNetworkSystem->NET_IsMultiplayer() ? "Multiplayer" : "Singleplayer",
		g_pLocalNetworkSystem->NET_IsDedicated() ? "dedicated" : "listen");

	CFmtStrN<128> lan_str;
#ifdef LINUX
	lan_str.sprintf(", Lan %u", net_sockets[NS_SVLAN].nPort);
#endif
	ConMsg("- Ports: Client %u, Server %u, HLTV %u, Matchmaking %u, Systemlink %u%s\n",
		g_pLocalNetworkSystem->GetClientSocket()->NET_GetUDPPort(),
		g_pLocalNetworkSystem->GetServerSocket()->NET_GetUDPPort(),
		g_pLocalNetworkSystem->GetHLTVSocket()->NET_GetUDPPort(),
		g_pLocalNetworkSystem->GetMatchMakingSocket()->NET_GetUDPPort(),
		g_pLocalNetworkSystem->GetSystemLinkSocket()->NET_GetUDPPort(),
			lan_str.Get());

	for (int s = 0; s < g_pLocalNetworkSystem->GetSocketCount(); s++)
	{
		INetSocket* socket = g_pLocalNetworkSystem->GetSocket(s);

		AUTO_LOCK_FM(socket->GetNetChannels());
		int numChannels = socket->GetNetChannelCount();

		ConMsg("- %i %i connections\n", s, numChannels);

		if (numChannels <= 0)
		{
			continue;
		}

		// gather statistics:

		float avgLatencyOut = 0;
		float avgLatencyIn = 0;
		float avgPacketsOut = 0;
		float avgPacketsIn = 0;
		float avgLossOut = 0;
		float avgLossIn = 0;
		float avgDataOut = 0;
		float avgDataIn = 0;

		for (int i = 0; i < numChannels; i++)
		{
			INetChannel* chan = socket->GetNetChannel(i);

			avgLatencyOut += chan->GetAvgLatency(FLOW_OUTGOING);
			avgLatencyIn += chan->GetAvgLatency(FLOW_INCOMING);

			avgLossIn += chan->GetAvgLoss(FLOW_INCOMING);
			avgLossOut += chan->GetAvgLoss(FLOW_OUTGOING);

			avgPacketsIn += chan->GetAvgPackets(FLOW_INCOMING);
			avgPacketsOut += chan->GetAvgPackets(FLOW_OUTGOING);

			avgDataIn += chan->GetAvgData(FLOW_INCOMING);
			avgDataOut += chan->GetAvgData(FLOW_OUTGOING);
		}

		ConMsg("- Latency: avg out %.2fs, in %.2fs\n", avgLatencyOut / numChannels, avgLatencyIn / numChannels);
		ConMsg("- Loss:    avg out %.1f, in %.1f\n", avgLossOut / numChannels, avgLossIn / numChannels);
		ConMsg("- Packets: net total out  %.1f/s, in %.1f/s\n", avgPacketsOut, avgPacketsIn);
		ConMsg("           per client out %.1f/s, in %.1f/s\n", avgPacketsOut / numChannels, avgPacketsIn / numChannels);
		ConMsg("- Data:    net total out  %.1f, in %.1f kB/s\n", avgDataOut / 1024.0f, avgDataIn / 1024.0f);
		ConMsg("           per client out %.1f, in %.1f kB/s\n", (avgDataOut / numChannels) / 1024.0f, (avgDataIn / numChannels) / 1024.0f);
	}
	
}
