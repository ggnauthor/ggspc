#ifndef _debugger_analyze_info
#define _debugger_analyze_info
//-----------------------------------------------------------------------------
// include
#include "main.h"

//-----------------------------------------------------------------------------
// define

//-----------------------------------------------------------------------------
// class
class AnalyzeInfo {
// constructor/destructor
public:
  AnalyzeInfo() {
    m_enable = false;
    m_valid_estimate = false;
    m_estimate_value = 0x00000000;
  }

  ~AnalyzeInfo() {}

// member fucntion
public:
  void SetEstimate(u32 p_estimate) {
    m_valid_estimate = true;
    m_estimate_value = p_estimate;
  }
  
  void NoEstimate() {
    m_valid_estimate = false;
  }

// accessor
public:
  inline bool enable() { return m_enable; }
  inline bool valid_estimate() { return m_valid_estimate; }
  inline u32  estimate_value() { return m_estimate_value; }

  inline void set_enable(bool p_val) { m_enable = p_val; }

// member variable
private:
  bool m_enable;         // �g�p����Ă���(�G���g���|�C���g����H���)
  bool m_valid_estimate; // ���ʗ\�����L�����H
  u32  m_estimate_value; // ���ʗ\���l
};

#endif // _debugger_analyze_info
