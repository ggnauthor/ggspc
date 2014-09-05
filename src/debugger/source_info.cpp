/*---------*/
/* include */
/*---------*/
#include "debugger/source_info.h"

#include "main.h"
#include "cpu/r5900.h"
#include "memory/memory.h"
#include "debugger/debugger.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

/*---------*/
/* structs */
/*---------*/

/*--------*/
/* global */
/*--------*/

/*----------*/
/* function */
/*----------*/
SourceFileInfo::SourceFileInfo(char* p_data, int p_fsize, SourceFileEntry* p_sfe) {
  m_data = p_data;
  m_scroll = 0;
  m_src_file = p_sfe;

  // ソース指定されていたら行分割する
  if (m_data) {
    char* p = m_data;

    m_line_data_ary.push_back(p);
    while (p - m_data < p_fsize) {
      while (*p == '\r') {
        *p++ = '\0';
        if (*p == '\n') break;
        m_line_data_ary.push_back(p);
      }
      while (*p == '\n') {
        *p++ = '\0';
        m_line_data_ary.push_back(p);
      }
      p++;
    }
  }
}
