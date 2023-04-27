//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#ifndef INETWORKSYSTEM_H
#define INETWORKSYSTEM_H
#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"
#include "appframework/IAppSystem.h"
#include "inetchannel.h"
#include "inetmsghandler.h"
#include "proto_version.h"

//-----------------------------------------------------------------------------
// Forward declarations: 
//-----------------------------------------------------------------------------
//class INetworkMessageHandler;
//class INetworkMessage;
//class INetChannel;
//class INetworkMessageFactory;
class bf_read;
class bf_write;
typedef struct netadr_s netadr_t;
class CNetPacket;


//-----------------------------------------------------------------------------
// Default ports
//-----------------------------------------------------------------------------
//enum
//{
//	NETWORKSYSTEM_DEFAULT_SERVER_PORT = 27001,
//	NETWORKSYSTEM_DEFAULT_CLIENT_PORT = 27002
//};


//-----------------------------------------------------------------------------
// This interface encompasses a one-way communication path between two
//-----------------------------------------------------------------------------
//typedef int ConnectionHandle_t;

//enum ConnectionStatus_t
//{
//	CONNECTION_STATE_DISCONNECTED = 0,
//	CONNECTION_STATE_CONNECTING,
//	CONNECTION_STATE_CONNECTION_FAILED,
//	CONNECTION_STATE_CONNECTED,
//};


//-----------------------------------------------------------------------------
// This interface encompasses a one-way communication path between two machines
//-----------------------------------------------------------------------------
//abstract_class INetChannel
//{
//public:
//	virtual INetworkMessageHandler *GetMsgHandler( void ) const = 0;
//	virtual const netadr_t	&GetRemoteAddress( void ) const = 0;

	// send a net message
	// NOTE: There are special connect/disconnect messages?
	//virtual bool AddNetMsg( INetworkMessage *msg, bool bForceReliable = false ) = 0; 
//	virtual bool RegisterMessage( INetworkMessage *msg ) = 0;

//	virtual ConnectionStatus_t GetConnectionState( ) = 0;

/*
	virtual ConnectTo( const netadr_t& to ) = 0;
	virtual Disconnect() = 0;

	virtual const netadr_t& GetLocalAddress() = 0;

	virtual const netadr_t& GetRemoteAddress() = 0;
*/
//};


//-----------------------------------------------------------------------------
// Network event types + structures
//-----------------------------------------------------------------------------
//enum NetworkEventType_t
//{
//	NETWORK_EVENT_CONNECTED = 0,
//	NETWORK_EVENT_DISCONNECTED,
//	NETWORK_EVENT_MESSAGE_RECEIVED,
//};
//
//struct NetworkEvent_t
//{
//	NetworkEventType_t m_nType;
//};
//
//struct NetworkConnectionEvent_t : public NetworkEvent_t
//{
//	//INetChannel *m_pChannel;
//};
//
//struct NetworkDisconnectionEvent_t : public NetworkEvent_t
//{
//	//INetChannel *m_pChannel;
//};
//
//struct NetworkMessageReceivedEvent_t : public NetworkEvent_t
//{
//	//INetChannel *m_pChannel;
//	//INetworkMessage *m_pNetworkMessage;
//};


//-----------------------------------------------------------------------------
// Main interface for low-level networking (packet sending). This is a low-level interface
//-----------------------------------------------------------------------------
#define NETWORKSYSTEM_INTERFACE_VERSION	"NetworkSystemVersion001"
abstract_class INetworkSystem : public IAppSystem
{
public:

	// Start up networking
	virtual void		NET_Init(bool bDedicated) = 0;
	// Shut down networking
	virtual void		NET_Shutdown(void) = 0;
	// Read any incoming packets, dispatch to known netchannels and call handler for connectionless packets
	virtual void		NET_ProcessSocket(int sock, IConnectionlessPacketHandler* handler) = 0;
	// Set a port to listen mode
	virtual void		NET_ListenSocket(int sock, bool listen) = 0;
	// Send connectionsless string over the wire
	virtual void		NET_OutOfBandPrintf(int sock, const netadr_t& adr, PRINTF_FORMAT_STRING const char* format, ...) FMTFUNCTION(3, 4) = 0;
	// Send a raw packet, connectionless must be provided (chan can be NULL)
	virtual int			NET_SendPacket(INetChannel* chan, int sock, const netadr_t& to, const  unsigned char* data, int length, bf_write* pVoicePayload = NULL, bool bUseCompression = false) = 0;
	// Called periodically to maybe send any queued packets (up to 4 per frame)
	virtual void		NET_SendQueuedPackets() = 0;
	// Start set current network configuration
	virtual void		NET_SetMutiplayer(bool multiplayer) = 0;
	// Set net_time
	virtual void		NET_SetTime(double realtime) = 0;
	// RunFrame must be called each system frame before reading/sending on any socket
	virtual void		NET_RunFrame(double realtime) = 0;
	// Check configuration state
	virtual bool		NET_IsMultiplayer(void) = 0;
	virtual bool		NET_IsDedicated(void) = 0;
	// Writes a error file with bad packet content
	virtual void		NET_LogBadPacket(netpacket_t* packet) = 0;

	// bForceNew (used for bots) tells it not to share INetChannels (bots will crash when disconnecting if they
	// share an INetChannel).
	virtual INetChannel* NET_CreateNetChannel(int socket, netadr_t* adr, const char* name, INetChannelHandler* handler, bool bForceNew = false,
		int nProtocolVersion = PROTOCOL_VERSION) = 0;
	virtual void		NET_RemoveNetChannel(INetChannel* netchan, bool bDeleteNetChan) = 0;
	virtual void		NET_PrintChannelStatus(INetChannel* chan) = 0;

	//virtual void		NET_WriteStringCmd(const char* cmd, bf_write* buf) = 0;

	// Address conversion
	virtual bool		NET_StringToAdr(const char* s, netadr_t* a) = 0;
	virtual bool		NET_StringToSockaddr(const char* s, struct sockaddr* sadr) = 0;
	// Convert from host to network byte ordering
	virtual unsigned short NET_HostToNetShort(unsigned short us_in) = 0;
	// and vice versa
	virtual unsigned short NET_NetToHostShort(unsigned short us_in) = 0;

	// Find out what port is mapped to a local socket
	virtual unsigned short NET_GetUDPPort(int socket) = 0;

	// add/remove extra sockets for testing
	virtual int NET_AddExtraSocket(int port) = 0;
	virtual void NET_RemoveAllExtraSockets() = 0;

	virtual const char* NET_ErrorString(int code) = 0; // translate a socket error into a friendly string

	// Installs network message factories to be used with all connections
	//virtual bool RegisterMessage( INetworkMessage *msg ) = 0;

	// Start, shutdown a server
	//virtual bool StartServer( unsigned short nServerListenPort = NETWORKSYSTEM_DEFAULT_SERVER_PORT ) = 0;
	//virtual void ShutdownServer( ) = 0;

	// Process server-side network messages
	//virtual void ServerReceiveMessages() = 0;
	//virtual void ServerSendMessages() = 0;

	// Start, shutdown a client
	//virtual bool StartClient( unsigned short nClientListenPort = NETWORKSYSTEM_DEFAULT_CLIENT_PORT ) = 0;
	//virtual void ShutdownClient( ) = 0;

	// Process client-side network messages
	//virtual void ClientSendMessages() = 0;
	//virtual void ClientReceiveMessages() = 0;

	// Connect, disconnect a client to a server
	//virtual INetChannel* ConnectClientToServer( const char *pServer, int nServerListenPort = NETWORKSYSTEM_DEFAULT_SERVER_PORT ) = 0;
	//virtual void DisconnectClientFromServer( INetChannel* pChan ) = 0;

	// Event queue
	//virtual NetworkEvent_t *FirstNetworkEvent( ) = 0;
	//virtual NetworkEvent_t *NextNetworkEvent( ) = 0;

	// Returns the local host name
	//virtual const char* GetLocalHostName( void ) const = 0;
	//virtual const char* GetLocalAddress( void ) const = 0;

	/*
	// NOTE: Server methods
	// NOTE: There's only 1 client INetChannel ever
	// There can be 0-N server INetChannels.
	virtual INetChannel* CreateConnection( bool bIsClientConnection ) = 0;

	// Add methods for setting unreliable payloads
	*/
};


#endif // INETWORKSYSTEM_H
