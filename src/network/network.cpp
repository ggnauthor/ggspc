#define _CRTDBG_MAP_ALLOC

#include "main.h"
#include "network/network.h"
#include "network/node.h"
#include "utils/http.h"
#include "utils/misc.h"

#include <ws2tcpip.h>
#include <mmsystem.h>
#include <crtdbg.h>
#include "debug_malloc.h"

void NetworkMgr::init(u16 p_port) {
  WSADATA wsadata;
  int err = WSAStartup(MAKEWORD(2, 0), &wsadata);
  if (err != 0) {
    debugOut("WSAStartup err\n");
    return;
  }
  debugOut("WSAStartup ok\n");

  sockaddr_in  addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_port        = htons(p_port);
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;

  socket_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_ == INVALID_SOCKET) {
    debugOut("socket() err\n");
    WSACleanup();
    return;
  }
  debugOut("socket() ok\n");

  if (bind(socket_, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
    debugOut("bind() err\n");
    closesocket(socket_);
    WSACleanup();
    return;
  }
  debugOut("bind() ok\n");

  unsigned long val = 1;
  if (ioctlsocket(socket_, FIONBIO, &val) == SOCKET_ERROR) {
    debugOut("ioctlsocket(FIONBIO) err\n");
    closesocket(socket_);
    WSACleanup();
    return;
  }
  debugOut("ioctlsocket(FIONBIO) ok\n");

  DWORD tid;
  recv_thread_ = CreateThread(NULL, 0, recvThreadProc, this, CREATE_SUSPENDED, &tid);
  SetThreadPriority(recv_thread_, THREAD_PRIORITY_NORMAL);
  recv_thread_end_ = false;
  debugOut("recv thread ok\n");

  lobby_thread_ = CreateThread(NULL, 0, lobbyThreadProc, this, CREATE_SUSPENDED, &tid);
  SetThreadPriority(lobby_thread_, THREAD_PRIORITY_NORMAL);
  lobby_thread_end_ = false;
  debugOut("lobby thread ok\n");

  packet_ = new Packet_Unknown;

  http_mgr_ = new HttpMgr();
  http_mgr_->init();
  debugOut("http mgr ok\n");

  node_mgr_ = new NodeMgr();
  node_mgr_->init();
  debugOut("node mgr ok\n");
}

void NetworkMgr::clean() {
  if (node_mgr_) {
    node_mgr_->clean();
    delete node_mgr_;
    node_mgr_ = NULL;
  }

  if (http_mgr_) {
    http_mgr_->clean();
    delete http_mgr_;
    http_mgr_ = NULL;
  }

  if (packet_) {
    delete packet_;
  }

  // スレッドが停止するまで待つ
  shutdown_ = true;
  for (int i = 0; i < 50; i++) {
    if (recv_thread_end_ && lobby_thread_end_) break;
    Sleep(20); // 1秒でtimeout
  }

  if (recv_thread_) {
    CloseHandle(recv_thread_);
    recv_thread_ = NULL;
    recv_thread_end_ = true;
  }
  if (lobby_thread_) {
    CloseHandle(lobby_thread_);
    lobby_thread_ = NULL;
    lobby_thread_end_ = true;
  }
  WSACleanup();
}

void NetworkMgr::startThread() {
  ResumeThread(recv_thread_);
  ResumeThread(lobby_thread_);
}

void NetworkMgr::entryLobby(bool p_busy) {
  in_lobby_ = true;
  lobby_entry_busy_ = p_busy;
  lobby_entry_lasttime_ = 0;
}

void NetworkMgr::leaveLobby() {
  in_lobby_ = false;
}

void NetworkMgr::lookupLobby() {
  lookupLobbyServer();
}

DWORD WINAPI NetworkMgr::recvThreadProc(LPVOID lpParameter) {
  NetworkMgr* mgr = reinterpret_cast<NetworkMgr*>(lpParameter);

  while (!mgr->shutdown_) {
    sockaddr_in sender_addr;
    int packet_size = mgr->recv(&sender_addr, reinterpret_cast<s8*>(mgr->packet_), kPacketMaxSize);
    if (packet_size <= 0) return 0;

    switch (mgr->packet_->packet_type) {
      case kPacketType_Ping:
        break;
    }
  }
  mgr->recv_thread_end_ = true;
  debugOut("recv thread end\n");
  return 0;
}

DWORD WINAPI NetworkMgr::lobbyThreadProc(LPVOID lpParameter) {
  NetworkMgr* mgr = reinterpret_cast<NetworkMgr*>(lpParameter);

  while (!mgr->shutdown_) {
    // TODO:Ping送信処理をここで

    // 放っておくと切れてしまうので定期的に再エントリ
    u32 progress_time = timeGetTime() - mgr->lobby_entry_lasttime_;
    if (mgr->in_lobby_ &&
        progress_time > kEntryLobbyInterval) {

      //mgr->entryLobbyServer();
    }
    Sleep(1);
  }
  mgr->lobby_thread_end_ = true;
  debugOut("lobby thread end\n");
  return 0;
}

void NetworkMgr::entryLobbyServer() {
  s8 buf[1024];
  sprintf(buf, "action=entry&name=%s&addr=%s&port=%d&busy=%d", "test", "127.0.0.1", kDefaultPort, lobby_entry_busy_);
  http_mgr_->post("http://localhost:8081/lobby.php", buf);

  lobby_entry_lasttime_ = timeGetTime();
}

void NetworkMgr::lookupLobbyServer() {
  const u8* buf = http_mgr_->post("http://localhost:8081/lobby.php", "action=get_node_list");
  node_mgr_->readNodeList(buf);
}

int NetworkMgr::send(const sockaddr_in* p_addr, const s8* p_buf, int p_buf_size) {
  return sendto(
           socket_,
           p_buf,
           p_buf_size,
           0,
           reinterpret_cast<const sockaddr*>(p_addr),
           sizeof(*p_addr));
}

int NetworkMgr::recv(sockaddr_in* p_addr, s8* p_buf, int p_buf_size) {
  int sockaddr_size = sizeof(*p_addr);
  return recvfrom(
           socket_,
           p_buf,
           p_buf_size,
           0,
           reinterpret_cast<sockaddr*>(p_addr),
           &sockaddr_size);
}
