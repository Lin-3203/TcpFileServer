#include "tcpfileserver.h"
#include <QApplication>
#include "tcpfilesender.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TcpFileServer w1;
    TcpFileSender w2;
    w1.show();
    w2.show();
    return a.exec();
}
