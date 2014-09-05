#ifndef _network_node
#define _network_node

#include "main.h"
#include "stlp_wrapper.h"

#include <winsock2.h>

class Node {
public:
  enum {
    kSize_Name = 64,
    kSize_Trip = 8,
    kSize_ClientCode = 16,
    kSize_Ping = 16,
  };

  enum {
    kValid_ClientCode = 0x00000001,
    kValid_Version    = 0x00000002,
  };

  Node(const s8* p_name, const sockaddr_in& p_addr) {
    name_[0] = '\0';
    trip_[0] = '\0';
    memset(client_code_, 0, sizeof(client_code_));

    strncpy(name_, p_name, 63);
    name_[63] = '\0';

    uncertain_addrs_.push_back(p_addr);
  }
  ~Node() {}

private:
  // ユーザ名
  s8 name_[kSize_Name];
  // トリップ名
  s8 trip_[8];
  // クライアントコード
  u8 client_code_[16];
  // 過去16回のPing平均で回線の状態を判断する
  s32 ping_[16];
  // 通信確立済のアドレス＆ポート番号
  sockaddr_in addr_;
  // 仮のノードアドレスリスト
  // UDPHolePunchingの為に複数のアドレスをまとめて管理する必要がある。
  // NoResponceへのPing送信はuncertain_addrs_すべてに対して行い、
  // そうでない場合は確定済アドレスを使用する。
  // 通信が確立された時点でuncertain_addrs_はクリアされる。
  vector<sockaddr_in> uncertain_addrs_;
  // 状態
  s32 state_;
  // リリースリビジョン
  u32 revision_;
  // PingReplyで詳細情報が必要か？
  bool need_detail_;
  // Pingを送った時間
  u32 send_ping_time_;
  // ロビーアドレスから算出した32bitの識別値
  u32 lobby_code_;
  // オプション情報の取得状態フラグ
  u32 valid_info_flg_;
};

class NodeMgr {
public:
  NodeMgr() {}
  ~NodeMgr() {}
  
  void init();
  void clean();
  void readNodeList(const u8* p_data);

  void addNode(Node* p_node) {
    node_ary_.push_back(p_node);
  }
private:
  void readNode(const u8* p_data, int p_data_size);

  vector<Node*> node_ary_;
};

#endif // _network_node
