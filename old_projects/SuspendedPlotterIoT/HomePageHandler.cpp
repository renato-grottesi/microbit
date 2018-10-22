#include "HomePageHandler.hpp"
#include "mbed.h"

#define CHUNK_SIZE 128

HomePageHandler::HomePageHandler(const char* rootPath, const char* path, TCPSocket* pTcpSocket)
: HTTPRequestHandler(rootPath, path, pTcpSocket)
{
}

HomePageHandler::~HomePageHandler()
{
}

void HomePageHandler::doGet()
{
    const char* opening = 
    "<html>\n"
    "  <head>\n"
    "    <title>ARM MBED Plotter</title>\n"
    "    <style>\n"
    "    table {\n"
    "        text-align: center;\n"
    "        width:352px;\n"
    "    }\n"
    "    #btn {\n"
    "        border: 1px solid black;\n"
    "        background-color: powderblue;\n"
    "    }\n"
    "    td, tr {\n"
    "        width: 32px;\n"
    "        height: 32px;\n"
    "    }\n"
    "    </style>\n"
    "    <script>\n"
    "      var client;\n"
    "      function upload()\n"
    "      {\n"
    "        var filetag = document.getElementById('uploadfile');\n"
    "        var file = filetag.files[0];\n"
    "        if (!file) {\n"
    "          return;\n"
    "        }\n"
    "        client = new XMLHttpRequest();\n"
    "        client.open('POST', 'upload');\n"
    "        client.setRequestHeader('Content-Type', 'text/plain');\n"
    "        client.setRequestHeader('Request', 'upload');\n"
    "        client.setRequestHeader('X-File-Name', file.name);\n"
    "        client.setRequestHeader('X-File-Size', file.size);\n"
    "        client.send(file);\n"
    "      }\n"
    "      function plot()\n"
    "      {\n"
    "        var filetag = document.getElementById('to_plot');\n"
    "        var file = filetag.value;\n"
    "        var formData = new FormData();\n"
    "        formData.append('plot', file);\n"
    "        client = new XMLHttpRequest();\n"
    "        client.open('POST', 'upload');\n"
    "        client.setRequestHeader('Request', 'plot');\n"
    "        client.setRequestHeader('X-File-Name', file);\n"
    "        client.setRequestHeader('Content-Type', 'multipart/form-data');\n"
    "        client.send(formData);\n"
    "      }\n"
    "      function move(value)\n"
    "      {\n"
    "        var formData = new FormData();\n"
    "        formData.append('value', value);\n"
    "        client = new XMLHttpRequest();\n"
    "        client.open('POST', 'upload');\n"
    "        client.setRequestHeader('Request', 'move');\n"
    "        client.setRequestHeader('Direction', value);\n"
    "        client.setRequestHeader('Content-Type', 'multipart/form-data');\n"
    "        client.send(formData);\n"
    "      }\n"
    "    </script>\n"
    "  </head>\n"
    "  <body>\n"
    "    <h1>ARM MBED Plotter</h1>\n"
    "    <br/>\n"
    "    <br/>\n"
    "    <input name='FileSubmit' type='submit' value='Upload' onclick='upload()' />\n"
    "    <input type='file' id='uploadfile' name='uploadfile' />\n"
    "    <br/>\n"
    "    <br/>\n"
    "    <input name='FilePlot' type='submit' value='Plot' onclick='plot()' />\n"
    "    <select id='to_plot' name='to_plot'>\n"
    "";

    const char* closing = 
    "    </select>\n"
    "    <br/>\n"
    "    <br/>\n"
    "    <table>\n"
    "      <tr>\n"
    "        <td></td>\n"
    "        <td id='btn' onclick='move(0)' >&#8657;</td>\n"
    "        <td></td>\n"
    "        <td></td>\n"
    "        <td id='btn' onclick='move(4)' >&#8624;</td>\n"
    "        <td></td>\n"
    "        <td id='btn' onclick='move(6)' >&#8662;</td>\n"
    "        <td></td>\n"
    "        <td id='btn' onclick='move(10)'>   0   </td>\n"
    "        <td></td>\n"
    "        <td id='btn' onclick='move(9)' >&#8663;</td>\n"
    "      </tr>\n"
    "      <tr>\n"
    "        <td id='btn' onclick='move(1)' >&#8656;</td>\n"
    "        <td id='btn' onclick='move(2)' >&#8659;</td>\n"
    "        <td id='btn' onclick='move(3)' >&#8658;</td>\n"
    "        <td></td>\n"
    "        <td id='btn' onclick='move(5)' >&#8627;</td>\n"
    "        <td></td>\n"
    "        <td></td>\n"
    "        <td id='btn' onclick='move(7)' >&#8664;</td>\n"
    "        <td></td>\n"
    "        <td id='btn' onclick='move(8)' >&#8665;</td>\n"
    "        <td></td>\n"
    "      </tr>\n"
    "    </table>\n"
    "  </body>\n"
    "</html>\n";

    m_fp = fopen("/usb/tmpidx", "w");

    m_idx_size = 0;
    for(int i=0; i<strlen(opening); i++)
    {
        m_idx_size += fwrite(&opening[i], 1, 1, m_fp);
    }
    fclose(m_fp);
    m_fp = fopen("/usb/tmpidx", "a");
    
    DIR *d = opendir("/usb");    
    if ( d != NULL )
    {
        struct dirent *p;
        while ( (p = readdir(d)) != NULL )
        {
            if(strncmp(p->d_name, "tmpidx", sizeof("tmpidx"))==0) continue;
            char filename[256];
            strncpy(filename, "      <option value='", 256);
            strncat(filename, p->d_name, 256);
            strncat(filename, "'>", 256);
            strncat(filename, p->d_name, 256);
            strncat(filename, "</option>\n", 256);
            m_idx_size += fwrite(filename, strlen(filename), 1, m_fp);
        }
        closedir(d);
    }
    for(int i=0; i<strlen(closing); i++)
    {
        m_idx_size += fwrite(&closing[i], 1, 1, m_fp);
    }
    // workaroud: add 2KB of new lines to allow the tcp socket to flush...
    for(int i=0; i<2048; i++)
    {
        const char* nl = "\n";
        m_idx_size += fwrite(nl, 1, 1, m_fp);
    }
    fclose(m_fp); 

    m_fp = fopen("/usb/tmpidx", "r");

    setContentLen(m_idx_size);
    respHeaders()["Connection"] = "close";
    onWriteable();
}

void HomePageHandler::doPost()
{
}

void HomePageHandler::doHead()
{
}

void HomePageHandler::onReadable()
{
}

void HomePageHandler::onWriteable()
{
  static char rBuf[CHUNK_SIZE];
  while(true)
  {
    int len = fread(rBuf, 1, CHUNK_SIZE, m_fp);
    if(len>0)
    {
      int writtenLen = writeData(rBuf, len);
      if(writtenLen < 0) //Socket error
      {
        if(writtenLen == TCPSOCKET_MEM)
        {
          fseek(m_fp, -len, SEEK_CUR);
          return; //Wait for the queued TCP segments to be transmitted
        }
        else
        {
          //This is a critical error
          close();
          return; 
        }
      }
      else if(writtenLen < len) //Short write, socket's buffer is full
      {
        fseek(m_fp, writtenLen - len, SEEK_CUR);
        return;
      }
    }
    else
    {
      close(); //Data written, we can close the connection
      return;
    }
  }
}

void HomePageHandler::onClose()
{
    if(m_fp)
    {
        fclose(m_fp);
        m_fp = NULL;
    }
}
