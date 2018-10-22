#include "mbed.h"
#include "MSCFileSystem.h"
#include "EthernetNetIf.h"
#include "HTTPServer.h"
#include "UploadHandler.hpp"
#include "HomePageHandler.hpp"

MSCFileSystem msc("usb");
Serial pc(USBTX, USBRX);
DigitalOut myled(LED1);

//IpAddr ip(10,0,0,85);
//IpAddr netmask(255,25,255,0);
//IpAddr nullip(0,0,0,0);
//EthernetNetIf eth(ip, netmask, nullip, nullip);
EthernetNetIf eth;
HTTPServer svr;

int main()
{
    pc.baud(115200);
    printf("Suspended Plotter\n\r");

    printf("Setting up...\n\r");
    EthernetErr ethErr = eth.setup();
    if(ethErr)
    {
        printf("Error %d in setup.\n\r", ethErr);
        return -1;
    }
    printf("Setup OK\n\r");

    svr.addHandler<HomePageHandler>("/");
    svr.addHandler<UploadHandler>("/upload");
    svr.bind(80);
    
    printf("Listening...\n\r");
    
    Timer tm;
    tm.start();

    while (true)
    {
        Net::poll();
        if(tm.read()>.5)
        {
            //Show that we are alive
            myled=!myled;
            tm.start();
        }
/*
        printf("Menu\n\r");
        printf("0: unroll 10000 steps\n\r");
        printf("h,j: unroll,roll 100 left\n\r");
        printf("k,l: unroll,roll 100 right\n\r");
        printf("w,s,a,d: move up,down,left,right by 0.01\n\r");
        printf("r: reset to (0.0, 0.0)\n\r");
        printf("o,p: start,stop draw\n\r");

        switch (pc.getc())
        {
        case 'k':
            plotter.unrollRight(100);
            break;
        case 'l':
            plotter.rollRight(100);
            break;

        case 'r':
            x = orig_x;
            y = orig_y;
            plotter.moveTo(x, y);
            break;
        case 'o':
            plotter.startDraw();
            break;
        case 'p':
            plotter.stopDraw();
            break;
        }
        printf("New position: %f, %f\n\r", x, y);
*/
    }
}
