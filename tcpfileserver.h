#ifndef TCPFILESERVER_H
#define TCPFILESERVER_H

#include <QtNetwork>
#include <QDialog>
#include <QtWidgets>

class TcpFileServer : public QDialog
{
    Q_OBJECT

public:
    TcpFileServer(QWidget *parent = nullptr);  // 使用 nullptr 替代 0 更现代
    ~TcpFileServer();

public slots:
    void start();                    // 启动服务器
    void acceptConnection();         // 接受连接
    void updateServerProgress();     // 更新接收进度
    void displayError(QAbstractSocket::SocketError socketError);  // 错误处理

private:
    // 进度条和状态标签
    QProgressBar     *serverProgressBar;
    QLabel           *serverStatusLabel;

    // 控制按钮
    QPushButton      *startButton;
    QPushButton      *quitButton;
    QDialogButtonBox *buttonBox;

    // 用于输入 IP 地址和端口号的控件
    QLineEdit *ipAddressLineEdit;  // 输入 IP 地址的框
    QLineEdit *portLineEdit;       // 输入端口号的框

    // 网络相关变量
    QTcpServer       tcpServer;
    QTcpSocket       *tcpServerConnection;
    qint64           totalBytes;
    qint64           byteReceived;
    qint64           fileNameSize;
    QString          fileName;
    QFile            *localFile;
    QByteArray       inBlock;
};

#endif // TCPFILESERVER_H
