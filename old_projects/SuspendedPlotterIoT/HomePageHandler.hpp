#ifndef HOME_PAGE_HANDLER_H
#define HOME_PAGE_HANDLER_H
 
#include "HTTPRequestHandler.h"

class HomePageHandler : public HTTPRequestHandler
{
public:
  HomePageHandler(const char* rootPath, const char* path, TCPSocket* pTcpSocket);
  virtual ~HomePageHandler();
 
  static inline HTTPRequestHandler* inst(const char* rootPath, const char* path, TCPSocket* pTcpSocket) 
  {
      return new HomePageHandler(rootPath, path, pTcpSocket);
  } 
 
protected:
  virtual void doGet();
  virtual void doPost();
  virtual void doHead();
  
  virtual void onReadable(); //Data has been read
  virtual void onWriteable(); //Data has been written & buf is free
  virtual void onClose(); //Connection is closing

private:
    FILE* m_fp;
    int   m_idx_size;
};
#endif