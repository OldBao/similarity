#include <iostream>

#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>

#include "log.h"
#include "kvproxy_client.h"
#include "interface.h"

using namespace std;
using namespace sm;


static const int MAX_PACK_SIZE = 2048;
static const char *PROVIDER = "Similarity";
static const uint16_t NS = TopicNamespace;
static const int HEADSIZE = sizeof (nshead_t) + sizeof (cache_req_t);
static const int RES_HEADSIZE = sizeof (nshead_t) + sizeof (cache_res_t);


Socket::Socket() : _fd(-1) {
  _fd = socket(AF_INET, SOCK_STREAM, 0);
  SM_ASSERT(_fd > 0, "create socket error : %s", strerror(_fd));
}


Socket::~Socket(){
  if (_fd > 0) {
    close (_fd);
    _fd = -1;
  }
}

int
Socket::Connect(const std::string& addr, const std::string& port) {
  int ret;
  struct addrinfo hints, *res;
  struct sockaddr sockaddr;
  socklen_t socklen;

  bzero (&hints, sizeof (hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_INET;
  hints.ai_protocol = 0;
  SM_CHECK_RET_ERR ( (ret = getaddrinfo (addr.c_str(), port.c_str(), &hints, &res)) == 0, 
                     "get addr info [%s:%s] error : %s", 
                     addr.c_str(), port.c_str(), gai_strerror (ret));
  memcpy (&sockaddr, res->ai_addr, res->ai_addrlen);
  socklen = res->ai_addrlen;
  freeaddrinfo (res);

  SM_CHECK_RET_ERR ( (ret =  connect (_fd, &sockaddr, socklen)) == 0, 
                     "connect to server [%s:%s] error : %s",
                     addr.c_str(), port.c_str(), gai_strerror (ret));

  return 0;
}

void
Socket::Close (){
  if (_fd != -1) {
    close (_fd);
    _fd = -1;
  }
}

int
Socket::Recv (void *buffer, size_t cnt) {
  SM_CHECK_RET_ERR (buffer, "buffer can't be NULL");
  int left = cnt;
  int ret;

  while (left > 0) {
    ret = read (_fd, buffer+cnt-left, left);
    SM_ASSERT (ret <= left, "ret bytes[%d] can't be more than left[%d]", ret, left);
    
    switch (ret) {
    case -1:
      switch (errno) {
      case EINTR:
        SM_LOG_TRACE ("read routine is interrupted");
        continue;
      case EAGAIN:
        return cnt - left;
      default:
        return -1;
      }
    case 0:
      return cnt - left;
    default:
      left -= ret;
    }
  }
  

  return cnt - left;
}

int
Socket::Send (const void *buf, size_t cnt) {
  size_t left = cnt;
  int ret;

  while (left > 0) {
    ret = write (_fd, buf-cnt+left, left);
    
    switch (ret) {
    case -1:
      switch (errno) {
      case EINTR:
        SM_LOG_NOTICE ("write routine interruted");
        continue;
      case EAGAIN:
        return cnt - left;
      default:
        return -1;
      }
    case 0:
      return cnt-left;
    default:
      left -= ret;
    }
  }

  return cnt-left;
}



KvProxyClient::KvProxyClient() {
}

KvProxyClient::~KvProxyClient(){

}

int
KvProxyClient::open(){

}

int
KvProxyClient::get(const std::string &key, std::string *value) {
  SM_ASSERT (value && value->size() == 0, "caller given a wrong value");
  Socket _socket;
  _socket.Connect("10.40.35.35", "8318");

  char request_packet[HEADSIZE], recv_packet[RES_HEADSIZE];

  nshead_t    *head  = (nshead_t *) request_packet, 
    *res_head = (nshead_t *) recv_packet;
  cache_req_t *req = (cache_req_t *) (request_packet + sizeof (nshead_t));
  cache_res_t *res = (cache_res_t *) (recv_packet + sizeof (nshead_t));
  char *body;
  int expect_body_size;
  int ret;
  memset (request_packet, 0, sizeof (request_packet));
  memset (recv_packet, 0, sizeof (recv_packet));

  head->body_len = sizeof (cache_req_t);
  head->magic_num = NSHEAD_MAGICNUM;
  memcpy (head->provider, PROVIDER, sizeof (PROVIDER));
  
  req->name_space = NS;
  req->cmd_id = CMD_CACHE_SEEK;
  memcpy(req->key, key.c_str(), key.size());
  
  ret = _socket.Send (request_packet, HEADSIZE);
  if (ret != HEADSIZE) {
    SM_LOG_WARNING ("write send packet error : %d", ret);
    return -1;
  }

  ret = _socket.Recv (recv_packet, RES_HEADSIZE);
  if (ret != RES_HEADSIZE) {
    SM_LOG_WARNING ("receive packet error: %d", ret);
    return -1;
  }
  
  SM_CHECK_RET_ERR(res_head->magic_num == NSHEAD_MAGICNUM, "not a nshead");

  switch (res->ret_no != 0){
  case 0:
    body = (char *) malloc (res_head->body_len);
    expect_body_size = res_head->body_len - sizeof (cache_res_t);
    if ( expect_body_size != _socket.Recv(body, expect_body_size) ){
      SM_LOG_WARNING ("receive packet error");
    } else {
      value->assign(body, expect_body_size);
    }
    free (body);
    return 0;

  case 1:
    SM_LOG_NOTICE ("%s non exists in cache", key.c_str());

  default:
    //if unknown error happens, and has body ,try to read all and ignore
    return -1;
  }

  _socket.Close();
  return -1;
}
