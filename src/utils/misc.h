#ifndef _utils_misc
#define _utils_misc

#include <winsock2.h>

bool getArgValue(const s8* p_arg, const s8* p_type, int p_idx, s8* p_value);
void getStringToSockAddr(const s8* p_str, sockaddr_in* p_addr);
void debugOut(const s8* p_fmt, ...);

#endif // _utils_misc
