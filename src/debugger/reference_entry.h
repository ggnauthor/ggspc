#ifndef _debugger_referenceentry
#define _debugger_referenceentry

#include "main.h"
#include "stlp_wrapper.h"
#include "debugger/reference_entry.h"

class ReferenceEntryBase {
public:
  ReferenceEntryBase(class DebugDatabase* p_owner, u32 p_va, char* p_text);
  virtual ~ReferenceEntryBase() {}

  u32 va() const { return m_va; }
  const s8* text() const { return m_text; }

  void set_text(const s8* p_text) {
    strncpy(m_text, p_text, 255);
    m_text[255] = '\0';
  }

  bool is_cur();

protected:
  u32 m_va;        // 仮想アドレス
  s8  m_text[256]; // テキスト
  class DebugDatabase* m_owner;
};

class Comment : public ReferenceEntryBase {
public:
  Comment(DebugDatabase* p_owner, u32 p_va, char* p_text) :
      ReferenceEntryBase(p_owner, p_va, p_text) {}
};

class Label : public ReferenceEntryBase {
public:
  Label(DebugDatabase* p_owner, u32 p_va, char* p_text) :
      ReferenceEntryBase(p_owner, p_va, p_text) {}
};

struct MemLabelElement {
  u32    type;
  u32    size;
  s8    label[256];
};

class MemLabel : public Label {
public:
  MemLabel(DebugDatabase* p_owner, u32 p_va, char* p_text) :
      Label(p_owner, p_va, p_text),
      m_expand(false) {
    m_comment[0] = '\0';
    set_count(1);
  }
  ~MemLabel() { clearElement(); }

  void AddElement(u32 p_type, u32 p_size, s8* p_label);
  void clearElement();
  int  GetSize() { return GetStructureSize() * m_count; }
  int  GetStructureSize();
  bool GetMemoryLabelFromVA(u32 p_va, s8* p_label);
  int  GetElementOffset(int p_idx);

// static function
public:
  static void ConvertElementData(s8* p_buf, int p_bufsize,
                                 u8* p_data, u32 p_type, int p_size);

// accessor
public:
  inline char* comment() { return m_comment; }
  inline int  count() { return m_count; }
  inline bool expand() { return m_expand; }
  inline bool expand_ary(int p_idx) { return m_expand_ary[p_idx]; }
  inline MemLabelElement* element(int p_idx) { return m_elem_ary[p_idx]; }
  inline int element_size() { return m_elem_ary.size(); }

  inline void set_comment(s8* p_comment) {
    strncpy(m_comment, p_comment, 256);
  }
  inline void set_count(u32 p_val) {
    m_count = p_val;
    while (m_expand_ary.size() < m_count) {
      m_expand_ary.push_back(false);
    }
  }
  inline void set_expand(bool p_val) { m_expand = p_val; }
  inline void set_expand_ary(int p_idx, bool p_val) { m_expand_ary[p_idx] = p_val; }

// member variable
private:
  char  m_comment[256];
  u32   m_count;
  vector<MemLabelElement*>  m_elem_ary;

  bool        m_expand;     // 展開しているか？
  deque<bool> m_expand_ary; // 配列を展開しているか？
};

class String : public ReferenceEntryBase
{
public:
  String(u32 p_va, char* p_text) :
      ReferenceEntryBase(NULL, p_va, p_text) {}
};

class FindItem : public ReferenceEntryBase
{
// constant definition
public:
  enum ELocation {
    kLocation_Code = 0,
    kLocation_Memory,
  };

// constructor/destructor
public:
  FindItem(u32 p_va, ELocation p_location) :
      ReferenceEntryBase(NULL, p_va, "") {
    m_location = p_location;
  }

// accessor
public:
  ELocation location() const { return m_location; }

// member variable
private:
  ELocation  m_location;
};

class BottleNeck : public ReferenceEntryBase
{
// constant definition
public:

// constructor/destructor
public:
  BottleNeck(u32 p_va, u8* p_overhead) :
      ReferenceEntryBase(NULL, p_va, "") {
    m_overhead = p_overhead;
  }

// accessor
public:
  u8 overhead() const { return *m_overhead; }

// member variable
private:
  u8* m_overhead;
};

#endif // _debugger_referenceentry
