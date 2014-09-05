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
  // ���[�U��
  s8 name_[kSize_Name];
  // �g���b�v��
  s8 trip_[8];
  // �N���C�A���g�R�[�h
  u8 client_code_[16];
  // �ߋ�16���Ping���ςŉ���̏�Ԃ𔻒f����
  s32 ping_[16];
  // �ʐM�m���ς̃A�h���X���|�[�g�ԍ�
  sockaddr_in addr_;
  // ���̃m�[�h�A�h���X���X�g
  // UDPHolePunching�ׂ̈ɕ����̃A�h���X���܂Ƃ߂ĊǗ�����K�v������B
  // NoResponce�ւ�Ping���M��uncertain_addrs_���ׂĂɑ΂��čs���A
  // �����łȂ��ꍇ�͊m��σA�h���X���g�p����B
  // �ʐM���m�����ꂽ���_��uncertain_addrs_�̓N���A�����B
  vector<sockaddr_in> uncertain_addrs_;
  // ���
  s32 state_;
  // �����[�X���r�W����
  u32 revision_;
  // PingReply�ŏڍ׏�񂪕K�v���H
  bool need_detail_;
  // Ping�𑗂�������
  u32 send_ping_time_;
  // ���r�[�A�h���X����Z�o����32bit�̎��ʒl
  u32 lobby_code_;
  // �I�v�V�������̎擾��ԃt���O
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
