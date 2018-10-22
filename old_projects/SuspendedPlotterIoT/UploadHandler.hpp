#ifndef UPLOAD_HANDLER_H
#define UPLOAD_HANDLER_H
 
#include "HTTPRequestHandler.h"
#include "SuspendedPlotter.h"

class UploadHandler : public HTTPRequestHandler
{
public:
  UploadHandler(const char* rootPath, const char* path, TCPSocket* pTcpSocket);
  virtual ~UploadHandler();
 
  static inline HTTPRequestHandler* inst(const char* rootPath, const char* path, TCPSocket* pTcpSocket) 
  {
      return new UploadHandler(rootPath, path, pTcpSocket);
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
    int m_total_read;
    int m_post_size;
    static const float orig_x = 0.5;
    static const float orig_y = 0.5;
    float m_x;
    float m_y;
};
#endif