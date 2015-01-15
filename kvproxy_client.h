#ifndef SM_MOLA_H_
#define SM_MOLA_H_
#include <string>

namespace sm {

  class Socket{
  public:
    Socket();
    ~Socket();

    int Connect (const std::string &addr, const std::string& port);
    int Send (const void *buf, size_t len);
    int Recv (void *buf, size_t len);    
  private:
    int _fd;
  };

  class KvProxyClient {
  public:
    KvProxyClient();
    ~KvProxyClient();

    int open ();

    int get(const std::string &key, std::string *value);
  private:
    Socket _socket;
  };
};

#endif
