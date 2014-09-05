#ifndef _utils_http
#define _utils_http

#include "main.h"

#include <curl/curl.h>

class ResponseBuffer {
public:
  enum {
    kDefaultSize = 64000,
    kExtendSize = 32000
  };

  ResponseBuffer() {
    capacity_ = kDefaultSize;
    size_ = 0;
    buf_ = (u8*)malloc(capacity_);
  }
  ~ResponseBuffer() { free(buf_); }

  void clear() { size_ = 0; }

  static size_t write(void *p_src, size_t p_size, size_t p_nmemb, ResponseBuffer* p_dst) {
    size_t size = p_size * p_nmemb;

    // バッファを超過する場合、拡張
    if (p_dst->size_ + size > p_dst->capacity_) {
      p_dst->capacity_ += kExtendSize;
      p_dst->buf_ = (u8*)realloc(p_dst->buf_, p_dst->capacity_);
    }
    memcpy(p_dst->buf_ + p_dst->size_, p_src, size);
    p_dst->size_ += size;
    return size;
  }

  const u8* buf() { return const_cast<const u8*>(buf_); }
  int size() { return (int)size_; }

private:
  u8*    buf_;
  size_t capacity_;
  size_t size_;
};

class HttpMgr {
public:
  enum {
#if defined _DEBUG
    kHttpPort = 8081,
#else
    kHttpPort = 80,
#endif
  };
  void init();
  void clean();
  const u8* get(const s8* p_url);
  const u8* post(const s8* p_url, const s8* p_data);

private:
  CURL *curl_;
  ResponseBuffer header_buf_;
  ResponseBuffer data_buf_;
};

#endif // _utils_http
