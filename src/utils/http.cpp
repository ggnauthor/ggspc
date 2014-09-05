#define _CRTDBG_MAP_ALLOC

#include "main.h"
#include "stlp_wrapper.h"
#include "utils/http.h"
#include "utils/misc.h"

#include <crtdbg.h>
#include "debug_malloc.h"

void HttpMgr::init() {
  curl_global_init(CURL_GLOBAL_ALL);
  curl_ = curl_easy_init();

  curl_easy_setopt(curl_, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  curl_easy_setopt(curl_, CURLOPT_PORT, kHttpPort);
  curl_easy_setopt(curl_, CURLOPT_SSLENGINE_DEFAULT, false);

  curl_easy_setopt(curl_, CURLOPT_VERBOSE, true);

  curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, ResponseBuffer::write);
}

void HttpMgr::clean() {
  curl_easy_cleanup(curl_);
  curl_ = NULL;
}

const u8* HttpMgr::get(const s8* p_url) {
  header_buf_.clear();
  data_buf_.clear();
  curl_easy_setopt(curl_, CURLOPT_WRITEHEADER, &header_buf_);
  curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &data_buf_);
  curl_easy_setopt(curl_, CURLOPT_URL, p_url);
  curl_easy_setopt(curl_, CURLOPT_POST, false);
  CURLcode res = curl_easy_perform(curl_);
  return data_buf_.buf();
}

const u8* HttpMgr::post(const s8* p_url, const s8* p_data) {
  header_buf_.clear();
  data_buf_.clear();
  curl_easy_setopt(curl_, CURLOPT_WRITEHEADER, &header_buf_);
  curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &data_buf_);
  curl_easy_setopt(curl_, CURLOPT_URL, p_url);
  curl_easy_setopt(curl_, CURLOPT_POST, true);
  curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, p_data);
  CURLcode res = curl_easy_perform(curl_);
  return data_buf_.buf();
}
