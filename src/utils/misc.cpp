#define _CRTDBG_MAP_ALLOC

#include "main.h"
#include "stlp_wrapper.h"
#include "utils/misc.h"

#include <stdio.h>
#include <crtdbg.h>
#include "debug_malloc.h"

bool getArgValue(const s8* p_arg, const s8* p_type, int p_idx, s8* p_value) {
  // xxx.exe -a yyy zzz -b -c iii

  char arg[1024] = " ";
  char typ[32] = " ";
  strcat(arg, p_arg);
  strcat(typ, p_type);
  if (p_value) strcpy(p_value, "");
  
  int idx = p_idx;
  char* ptr = strstr(arg, typ);
  if (ptr) {
    char* st = ptr + strlen(typ);
    // p_value == NULL‚È‚çŽæ“¾‚µ‚È‚¢
    while (p_value) {
      while (*st == ' ' || *st == '\t') st++;
      if (*st == '-' || *st == '\0') break;
      
      char* ed = st;
      while (*ed != '\0' && *ed != ' ' && *ed != '\t') ed++;
      if (*ed == '\0') {
        if (p_idx-- == 0) {
          strcpy(p_value, st);
        }
        break;
      }
      *ed = '\0';
      
      if (p_idx-- == 0) {
        strcpy(p_value, st);
        break;
      }
      st = ed + 1;
    }
    return true;
  }
  return false;
}

void getStringToSockAddr(const s8* p_str, sockaddr_in* p_addr) {
  s8 temp[8];
  int count = 0;
  int item = 0;
  for (int i = 0; i < 256; i++) {
    if (p_str[i] == '.' || p_str[i] == ':') {
      temp[i] = '\0';
      switch (item) {
        case 0: p_addr->sin_addr.S_un.S_un_b.s_b1 = atoi(temp); break;
        case 1: p_addr->sin_addr.S_un.S_un_b.s_b2 = atoi(temp); break;
        case 2: p_addr->sin_addr.S_un.S_un_b.s_b3 = atoi(temp); break;
        case 3: p_addr->sin_addr.S_un.S_un_b.s_b4 = atoi(temp); break;
        case 4: p_addr->sin_port = atoi(temp); break;
      }
      if (item == 4) break;
      item++;
      count = 0;
    } else {
      temp[count++] = p_str[i];
    }
  }
}

void errMsgBox(const s8* p_fmt, ...) {
	va_list ap;
	va_start(ap, p_fmt);
	
	s8 buf[1024];
	vsprintf(buf, p_fmt, ap);
  MessageBox(NULL, buf, NULL, NULL);

	va_end(ap);
}

void debugOut(const s8* p_fmt, ...) {
	va_list ap;
	va_start(ap, p_fmt);
	
	char buf[1024];
	vsprintf(buf, p_fmt, ap);
	OutputDebugString(buf);
	
	va_end(ap);
}
