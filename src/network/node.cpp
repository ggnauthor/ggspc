#include "main.h"
#include "network/node.h"
#include "utils/misc.h"

#include <stdio.h>
#include <mbstring.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

void NodeMgr::init() {
}

void NodeMgr::clean() {
  for (vector<Node*>::iterator itr = node_ary_.begin();
      itr != node_ary_.end();
      ++itr) {
    delete *itr;
  }
  node_ary_.clear();
}

void NodeMgr::readNodeList(const u8* p_data) {
  // ヘッダー～フッターの区間を抽出（無料鯖などで付加される可能性があるため）
  const u8* head = _mbsstr(p_data, (const u8*)"##head##");
  if (head == NULL) {
    return;
  }
  const u8* foot = _mbsstr(p_data, (const u8*)"##foot##");
  if (foot == NULL) {
    return;
  }
  
  head = _mbschr(head, '\n');
  if (head) {
    head++;
  } else {
    return;
  }
  const u8* line = head;
  int size = static_cast<int>(foot - head);
  for (int i = 0; i < size; i++) {
    const u8 *cr = _mbschr(line, '\r') - 1;
    const u8 *lf = _mbschr(line, '\n') - 1;
    
    int line_size = static_cast<int>((cr < lf ? cr : lf) + 1 - line);

    readNode(line, line_size);
  }
}

void NodeMgr::readNode(const u8* p_data, int p_data_size) {
  u8 name[Node::kSize_Name];
  sockaddr_in addr;

  const u8* ptr = p_data;
  while (true) {
    if (_mbscmp(ptr, (const u8*)"name=")) {
      ptr += 5;
      const u8* end = _mbschr(ptr, ' ');
      if (end == NULL) {
        return;
      }
      int size = static_cast<int>(end - ptr);
      if (size >= Node::kSize_Name) size = Node::kSize_Name - 1;
      _mbsncpy(name, const_cast<u8*>(ptr), size);
      name[size] = '\0';
      ptr += size + 1;
    } else if (_mbscmp(ptr, (const u8*)"addr=")) {

    } else if (_mbscmp(ptr, (const u8*)"port=")) {

    }
  }
  //Node* new_node = new Node();
}

/*
void NodeMgr::readLocalNodeList(const s8* p_file_path) {
  FILE* fp = fopen(p_file_path, "r");
  if (fp == NULL) return;
  
  while (true) {
    s8 buf[1024];
    if (fgets(buf, 1024, fp) == NULL) break; {
      // 各行addr:portの形式
      // スペース、タブを取り除き、改行で止める、#以降をコメント扱い
      s8 temp[1024];
      int count = 0;
      for (int i = 0; i < 1023; i++) {
        if (buf[i] == '\0') break;
        switch (buf[i]) {
          case ' ': case '\t':
            break;
          case '#': case '\n': case '\r':
            temp[count++] = '\0';
            break;
          default:
            temp[count++] = buf[i];
            break;
        }
        if (temp[count - 1] == '\0') break;
      }
      temp[count] = '\0';

      if (temp[0] != '\0') {
        sockaddr_in addr;
        getStringToSockAddr("127.0.0.1:1192", &addr);
        node_ary_.push_back(new Node("", addr));
      }
    }
  }
  fclose(fp);
}
*/
