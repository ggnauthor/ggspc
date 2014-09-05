#ifndef _stlp_wrapper
#define _stlp_wrapper

#include <vector>
#include <hash_map>
#include <deque>
#include <stack>

#include <crtdbg.h>

// �f�o�b�O����stlport�̃������v�[�����댟�o����̂�
// �����������邽�߂Ƀ��b�p�[�N���X���Ńt���O���삷��
#if defined _DEBUG

template <class _Tp, class _Alloc = std::allocator<_Tp>>
class vector : public std::vector<_Tp, _Alloc> {
  typedef std::vector<_Tp, _Alloc> Base;
public:
  iterator insert(iterator __pos, const _Tp& __x) {
    int flg = _CrtSetDbgFlag(0);
    iterator itr = Base::insert(__pos, __x);
    _CrtSetDbgFlag(flg);
    return itr;
  }
  void insert(iterator __pos, const bool* __first, const bool* __last) {
    int flg = _CrtSetDbgFlag(0);
    Base::insert(__pos, __first, __last);
    _CrtSetDbgFlag(flg);
  }
  void insert(iterator __pos, size_type __n, bool __x) {
    int flg = _CrtSetDbgFlag(0);
    Base::insert(__pos, __n, __x);
    _CrtSetDbgFlag(flg);
  }
  void push_back(const_reference __x) {
    int flg = _CrtSetDbgFlag(0);
    Base::push_back(__x);
    _CrtSetDbgFlag(flg);
  }
};

template <class _Key,
          class _Tp,
          class _HashFcn = std::hash<_Key>,
          class _EqualKey = std::equal_to<_Key>,
          class _Alloc = std::allocator<std::pair<_STLP_CONST _Key, _Tp>>>
class hash_map : public std::hash_map<_Key, _Tp, _HashFcn, _EqualKey, _Alloc> {
  typedef std::hash_map<_Key, _Tp, _HashFcn, _EqualKey, _Alloc> Base;
public:
  // std::hash_map�̎������o�ϐ��̏������O��_CrtSetDbgFlag��ύX����
  // �K�v������B�����ňȉ��̑Ή���₪������A����3��I�������B
  //  1. new���Z�q���I�[�o�[���C�h����
  //  2. stlport�̃\�[�X���R�s�[���ĉ��ς���B
  //  3. ���̃R���X�g���N�^�ɓn�������Ɋ֐����d���ށB
  // 1.��new���Z�q��new�̌Ăяo�����K�v�Ȃ̂ŏ��O�B
  // 2.��stlport�̉��ς͘J�͂������ARelease��Debug�œ��삪�ς��댯��������B
  // �܂����C�u�����̉��ώ��̂��C���i�܂Ȃ��׏��O�B
  // 3.�͌����ڂ������������ړI�����FDebug�r���h���̌듮�����Ȃ̂�
  // �C�ɂ��Ȃ����Ƃɂ����B
  hash_map() : Base::hash_map(
                 0,
                 hasher(),
                 key_equal(),
                 getAllocatorTypeWithDisableMemLeakCheck()) {
    _CrtSetDbgFlag(flg_);
  }
  allocator_type getAllocatorTypeWithDisableMemLeakCheck() {
    flg_ = _CrtSetDbgFlag(0);
    return allocator_type();
  }
  std::pair<iterator,bool> insert(const value_type& __obj) {
    int flg = _CrtSetDbgFlag(0);
    std::pair<iterator,bool> itr = Base::insert(__obj);
    _CrtSetDbgFlag(flg);
    return itr;
  }
  template <class _InputIterator>
  void insert(_InputIterator __f, _InputIterator __l) {
    int flg = _CrtSetDbgFlag(0);
    Base::insert(__f, __l);
    _CrtSetDbgFlag(flg);
  }
  template <class _KT> _Tp& operator[](const _KT& __key) {
    int flg = _CrtSetDbgFlag(0);
    _Tp& obj = Base::operator[](__key);
    _CrtSetDbgFlag(flg);
    return obj;
  }
private:
  int flg_;
};

template <class _Tp, _STLP_DFL_TMPL_PARAM(_Alloc, std::allocator<_Tp>) >
class deque : public std::deque<_Tp, _Alloc> {
  typedef std::deque<_Tp, _Alloc> Base;
public:
  deque() : Base::deque(
              getAllocatorTypeWithDisableMemLeakCheck()) {
    _CrtSetDbgFlag(flg_);
  }
  allocator_type getAllocatorTypeWithDisableMemLeakCheck() {
    flg_ = _CrtSetDbgFlag(0);
    return allocator_type();
  }
  void push_back(const_reference __x) {
    int flg = _CrtSetDbgFlag(0);
    Base::push_back(__x);
    _CrtSetDbgFlag(flg);
  }
  void push_front(const_reference __x) {
    int flg = _CrtSetDbgFlag(0);
    Base::push_front(__x);
    _CrtSetDbgFlag(flg);
  }
private:
  int flg_;
};

#else

template <class _Tp, _STLP_DFL_TMPL_PARAM(_Alloc, std::allocator<_Tp>) >
class vector : public std::vector<_Tp, _Alloc> {
};

template <class _Key,
          class _Tp,
          class _HashFcn = std::hash<_Key>,
          class _EqualKey = std::equal_to<_Key>,
          class _Alloc = std::allocator<std::pair<_STLP_CONST _Key, _Tp>>>
class hash_map : public std::hash_map<_Key, _Tp, _HashFcn, _EqualKey, _Alloc> {
};

template <class _Tp, _STLP_DFL_TMPL_PARAM(_Alloc, std::allocator<_Tp>) >
class deque : public std::deque<_Tp, _Alloc> {
};

#endif // defined _DEBUG

template <class _Tp, class _Sequence = deque<_Tp> >
class stack : public std::stack<_Tp, _Sequence> {
};

#endif // _stlp_wrapper
