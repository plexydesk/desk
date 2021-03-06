#ifndef URL_H
#define URL_H

#include <string>
#include <social_kit_export.h>

#include <functional>
#include <map>
#include <vector>
#include <algorithm>

namespace social_kit {

class url_response;

typedef std::function<void(const url_response &)> response_ready_callbcak_t;

class DECL_SOCIAL_KIT_EXPORT url_file_info {
public:
  url_file_info() {}
  virtual ~url_file_info() {}

  std::string m_path;
  std::string m_base_name;
  std::string m_mime_type;
  std::string m_text;
};

class DECL_SOCIAL_KIT_EXPORT url_request_context {
public:
  typedef enum { kMimeTypeMultipart, kMimeTypeUrlEncoded } request_mime_type;

  url_request_context();
  virtual ~url_request_context();

  void set_mime_type(request_mime_type a_mime_type);
  request_mime_type mime_type() const;

  void add_header(const std::string &a_key, const std::string &a_value);
  void add(const std::string &a_key, const std::string &a_value);
  void add_file(const url_file_info &a_file);

  std::string encode() const;

  std::map<std::string, std::string> multipart_data() const;
  std::vector<url_file_info> file_list() const;
  std::map<std::string, std::string> header() const;

private:
  class platform_multipart_data;
  platform_multipart_data *const ctx;
};

class DECL_SOCIAL_KIT_EXPORT url_encode {
public:
  url_encode(const std::string &a_str);
  virtual ~url_encode();

  virtual std::string to_string() const;

private:
  class platform_url_handle;
  platform_url_handle *const ctx;
};

class DECL_SOCIAL_KIT_EXPORT url_response {
public:
  url_response();
  url_response(const url_response &a_copy);
  ~url_response();

  int status_code() const;
  void set_status_code(int status_code);

  std::string uri() const;
  void set_uri(const std::string &uri);

  std::string response_body() const;
  void set_response_body(const std::string &response_body);

  std::string request_body() const;
  void set_request_body(const std::string &request_body);

  std::string http_version() const;
  void set_http_version(const std::string &http_version);

  std::string method() const;
  void set_method(const std::string &method);

  unsigned int data_buffer_size() const;
  void set_data_buffer_size(unsigned int data_buffer_size);

  unsigned char *data_buffer() const;
  void set_data_buffer(const unsigned char *data_buffer, unsigned int a_size);

private:
  int m_status_code;
  std::string m_uri;
  std::string m_response_body;
  std::string m_request_body;
  std::string m_http_version;
  std::string m_method;

  unsigned int m_data_buffer_size;
  unsigned char *m_data_buffer;
};

class DECL_SOCIAL_KIT_EXPORT url_request {
public:
  typedef enum {
    kPOSTRequest,
    kGETRequest,
    kHEADRequest,
    kDELETERequest,
    kCONNECTRequest,
    kPUTRequest,
    kOPTIONSRequest,
    kUndefinedRequest
  } url_request_type_t;

  url_request();
  virtual ~url_request();

  void submit(url_request_type_t a_type, const std::string &a_message);
  void submit(url_request_type_t a_type, const std::string &a_url,
              const url_request_context &a_form_data);
  void on_response_ready(response_ready_callbcak_t a_callback);

  class platform_url_request;

private:
  platform_url_request *const ctx;
};
}

#endif // URL_H
