#include "UploadHandler.hpp"
#include "mbed.h"
#include "nanosvg.h"

// The static plotter instance
static SuspendedPlotter plotter;

static void moveTo(float x, float y)
{
    plotter.moveTo(x, y);
    printf("Moving to %f, %f\n\r", x, y);
}

static void beginPath()
{
    plotter.startDraw();
    printf("Begin path\n\r");
}

static void endPath()
{
    plotter.stopDraw();
    printf("End path\n\r");
}

UploadHandler::UploadHandler(const char* rootPath, const char* path, TCPSocket* pTcpSocket)
: HTTPRequestHandler(rootPath, path, pTcpSocket)
{
    m_x = orig_x;
    m_y = orig_y;
}

UploadHandler::~UploadHandler()
{
}

void UploadHandler::doGet()
{
    const char* opening = "<html><head><title>Upload Page</title></head><body><p>Upload Page</p>";
    const char* closing = "</body></html>";
    char filename[256];
    int len = 0;
    len += writeData(opening, strlen(opening));
    
    DIR *d = opendir("/usb");    
    if ( d != NULL )
    {
        struct dirent *p;
        while ( (p = readdir(d)) != NULL )
        {
            strncpy(filename, "<p>", strlen("<p>")+1);
            strncat(filename, p->d_name, 128);
            strncat(filename, "</p>", strlen("</p>")+1);
            len += writeData(filename, strlen(filename));
        }
        closedir(d);
    }
    len += writeData(closing, strlen(closing));

    setContentLen(len);
    respHeaders()["Connection"] = "close";
}

void UploadHandler::doPost()
{
    m_post_size = dataLen();
    m_total_read = 0;
    const char* request_type = reqHeaders()["Request"].c_str();

    if(strncmp(request_type, "upload", sizeof("upload"))==0)
    {
        const char* file_name = reqHeaders()["X-File-Name"].c_str();
        int file_size = atoi(reqHeaders()["X-File-Size"].c_str());
    
        char full_file_name[512];
        strncpy(full_file_name, "/usb/", sizeof("/usb/"));
        strncat(full_file_name, file_name, 510 - sizeof("/usb/"));
    
        m_fp = fopen( full_file_name, "w");
    
        onReadable();
    }
    else if(strncmp(request_type, "plot", sizeof("plot"))==0)
    {
        const char* file_name = reqHeaders()["X-File-Name"].c_str();
        char full_file_name[512];
        strncpy(full_file_name, "/usb/", sizeof("/usb/"));
        strncat(full_file_name, file_name, 510 - sizeof("/usb/"));

        const char* answer =    "<html><head><title>Upload Page</title></head>"
                                "<body><p>Plot Handled</p></body></html>";
        writeData(answer, strlen(answer));
        setContentLen(strlen(answer));
        respHeaders()["Connection"] = "close";
        close();

        nsvgDrawFromFile(full_file_name, moveTo, beginPath, endPath, 0.3, 0.35, 0.4);
        plotter.moveTo(orig_x, orig_y);
    }
    else if(strncmp(request_type, "move", sizeof("plot"))==0)
    {
        int dir = atoi(reqHeaders()["Direction"].c_str());

        const char* answer =    "<html><head><title>Upload Page</title></head>"
                                "<body><p>Move Handled</p></body></html>";
        writeData(answer, strlen(answer));
        setContentLen(strlen(answer));
        respHeaders()["Connection"] = "close";
        close();

        switch (dir)
        {
        case 0:
            m_y -= 0.01;
            plotter.moveTo(m_x, m_y);
            break;
        case 1:
            m_x -= 0.01;
            plotter.moveTo(m_x, m_y);
            break;
        case 2:
            m_y += 0.01;
            plotter.moveTo(m_x, m_y);
            break;
        case 3:
            m_x += 0.01;
            plotter.moveTo(m_x, m_y);
            break;
        case 4:
            plotter.startDraw();
            break;
        case 5:
            plotter.stopDraw();
            break;
        case 6:
            plotter.rollLeft(100);
            break;
        case 7:
            plotter.unrollLeft(100);
            break;
        case 8:
            plotter.unrollRight(100);
            break;
        case 9:
            plotter.rollRight(100);
            break;
       case 10:
            m_x = orig_x;
            m_y = orig_y;
            plotter.moveTo(m_x, m_y);
            break;
         }
    }
}

void UploadHandler::doHead()
{
}

void UploadHandler::onReadable()
{
    int rlen = 0;
    do
    {
        char buf[4096];
        memset(buf, 0, 4096);
        rlen = readData(buf, 4096);
        if(rlen>0)
        {
            fwrite(buf, rlen, 1, m_fp);
        }
        m_total_read += rlen>0?rlen:-rlen; // negative rlen is an error
    } while (rlen>0);

    // If we got an error or if we reached the end of the socket
    if(rlen<0 || m_total_read >= m_post_size)
    {
        fclose(m_fp); 
        const char* answer =    "<html><head><title>Upload Page</title></head>"
                                "<body><p>Post Handled</p></body></html>";
        writeData(answer, strlen(answer));
        setContentLen(strlen(answer));
        respHeaders()["Connection"] = "close";
        close();
    } // else wait for next call to onReadable()
}

void UploadHandler::onWriteable()
{
        close();
}

void UploadHandler::onClose()
{
}
