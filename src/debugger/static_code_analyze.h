#ifndef _debugger_static_code_analyze
#define _debugger_static_code_analyze

#include "main.h"
#include "stlp_wrapper.h"

class AnalyzeInfo;

class AnalyzeContext {
public:
  AnalyzeContext() {}
  AnalyzeContext(AnalyzeContext& p_org);
  ~AnalyzeContext() {}

  void ClearGPR(void);
  void Init(u32 p_entry);

  void AnalyzeFragment(
    hash_map<int, AnalyzeInfo>& p_ana_inf_map,
    bool p_enable);

private:
  void Step();
  void Branch(u32 p_addr);
  void UncertainBranch(u32 p_addr);

  u32 pc_;
  u32 jump_addr_[2];

  stack<AnalyzeContext*> branch_stack_;

  // レジスタの状態を保存する。解析用途でしか使わないので
  // 不要な部分をかなり省略した。必要であれば都度追加していく。
  u32 gpr_[32];
};

#endif // _debugger_static_code_analyze
