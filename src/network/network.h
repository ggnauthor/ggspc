#ifndef _network_network
#define _network_network

#include <winsock2.h>

class HttpMgr;
class NodeMgr;

class NetworkMgr {
public:
  enum {
    kDefaultPort = 18782,
    kPacketMaxSize = 1024,
  };
  enum {
    kEntryLobbyInterval = 60000, // 60sec
  };
  enum {
    kPacketType_Ping = 0,
  };
  NetworkMgr(void) :
      http_mgr_(NULL),
      node_mgr_(NULL),
      shutdown_(false),
      recv_thread_end_(true),
      recv_thread_(NULL),
      lobby_thread_end_(true),
      lobby_thread_(NULL),
      socket_(INVALID_SOCKET),
      packet_(NULL),
      in_lobby_(false),
      lobby_entry_busy_(false),
      lobby_entry_lasttime_(0) {
    memset(&socket_addr_, 0, sizeof(socket_addr_));
    memset(&packet_, 0, sizeof(packet_));
  }
  ~NetworkMgr(void) {}
  
  void init(u16 p_port);
  void clean();
  void startThread();
  void stopThread();

  void entryLobby(bool p_busy);
  void leaveLobby();
  void lookupLobby();

private:public:
  static DWORD WINAPI recvThreadProc(LPVOID lpParameter);
  static DWORD WINAPI lobbyThreadProc(LPVOID lpParameter);
  void entryLobbyServer();
  void lookupLobbyServer();
  int send(const sockaddr_in* p_addr, const s8* p_buf, int p_buf_size);
  int recv(sockaddr_in* p_addr, s8* p_buf, int p_buf_size);
  
  HttpMgr* http_mgr_;
  NodeMgr* node_mgr_;

  volatile bool shutdown_;

  volatile bool recv_thread_end_;
  HANDLE        recv_thread_;
  volatile bool lobby_thread_end_;
  HANDLE        lobby_thread_;

  SOCKET      socket_;
  sockaddr_in socket_addr_;
  
  struct Packet_Unknown* packet_;

  bool in_lobby_;
  bool lobby_entry_busy_;
  u32  lobby_entry_lasttime_;
};

struct Packet_Unknown {
  s8 packet_type;
  s8 data[NetworkMgr::kPacketMaxSize - 1];
};

struct Packet_Ping {
  s8 packet_type;
  s8 name[64];
};

#endif // _network_network
