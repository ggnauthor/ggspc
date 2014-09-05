#ifndef _utils_ini
#define _utils_ini

class IniFileInfo {
public:
  IniFileInfo() {
  }
  ~IniFileInfo() {
  }

  void read();
  void write();
  
  s8* last_open() { return last_open_; }

private:
  s8  last_open_[_MAX_PATH];
};

#endif // _utils_ini
