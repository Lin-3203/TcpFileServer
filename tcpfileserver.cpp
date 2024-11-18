#include "tcpfileserver.h"
#define tr QStringLiteral

TcpFileServer::TcpFileServer(QWidget *parent)
    : QDialog(parent)
{
    totalBytes = 0;
    byteReceived = 0;
    fileNameSize = 0;

    // Create new UI components for IP and Port input
    ipAddressLineEdit = new QLineEdit;
    portLineEdit = new QLineEdit;
    ipAddressLineEdit->setPlaceholderText("輸入 IP 地址");
    portLineEdit->setPlaceholderText("輸入端口號");
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));  // Ensure port is valid

    // UI components for file reception
    serverProgressBar = new QProgressBar;
    serverStatusLabel = new QLabel(QStringLiteral("伺服器端就緒"));
    startButton = new QPushButton(QStringLiteral("接收"));
    quitButton = new QPushButton(QStringLiteral("退出"));
    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    // Layout setup
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(serverProgressBar);
    mainLayout->addWidget(serverStatusLabel);

    // Add IP and Port input fields to the layout
    mainLayout->addWidget(ipAddressLineEdit);
    mainLayout->addWidget(portLineEdit);

    mainLayout->addStretch();
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    setWindowTitle(QStringLiteral("接收檔案"));

    // Connect signals and slots
    connect(startButton, SIGNAL(clicked()), this, SLOT(start()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(&tcpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
    connect(&tcpServer, SIGNAL(acceptError(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));
}

TcpFileServer::~TcpFileServer()
{
}

void TcpFileServer::start()
{
    startButton->setEnabled(false);
    byteReceived = 0;
    fileNameSize = 0;

    // Get IP and Port from the input fields
    QString ipAddress = ipAddressLineEdit->text();
    quint16 port = static_cast<quint16>(portLineEdit->text().toInt());

    // Validate IP address and port
    if (ipAddress.isEmpty() || port == 0) {
        QMessageBox::warning(this, QStringLiteral("錯誤"), QStringLiteral("請輸入有效的 IP 地址和端口號"));
        return;
    }

    // Attempt to start the server with the specified IP address and port
    while(!tcpServer.isListening() && !tcpServer.listen(QHostAddress(ipAddress), port))
    {
        QMessageBox::StandardButton ret = QMessageBox::critical(this,
                                                                QStringLiteral("迴圈"),
                                                                QStringLiteral("無法啟動伺服器: %1.").arg(tcpServer.errorString()),
                                                                QMessageBox::Retry | QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel) {
            startButton->setEnabled(true);
            return;
        }
    }

    serverStatusLabel->setText(QStringLiteral("監聽中..."));
}

void TcpFileServer::acceptConnection()
{
    tcpServerConnection = tcpServer.nextPendingConnection();  // Get the client connection
    connect(tcpServerConnection, SIGNAL(readyRead()), this, SLOT(updateServerProgress()));  // Read data when ready
    connect(tcpServerConnection, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));  // Handle errors

    serverStatusLabel->setText(QStringLiteral("接受連線"));
    tcpServer.close();  // Stop accepting new client connections for now
}

void TcpFileServer::updateServerProgress()
{
    QDataStream in(tcpServerConnection);
    in.setVersion(QDataStream::Qt_4_6);

    if(byteReceived <= sizeof(qint64)*2)
    {
        if((fileNameSize == 0) && (tcpServerConnection->bytesAvailable() >= sizeof(qint64)*2))
        {
            in >> totalBytes >> fileNameSize;
            byteReceived += sizeof(qint64)*2;
        }

        if((fileNameSize != 0) && (tcpServerConnection->bytesAvailable() >= fileNameSize))
        {
            in >> fileName;
            byteReceived += fileNameSize;

            localFile = new QFile(fileName);
            if(!localFile->open(QFile::WriteOnly))
            {
                QMessageBox::warning(this, QStringLiteral("應用程式"),
                                     QStringLiteral("無法讀取檔案 %1：\n%2.").arg(fileName)
                                         .arg(localFile->errorString()));
                return;
            }
        }
        else {
            return;
        }
    }

    if (byteReceived < totalBytes)
    {
        byteReceived += tcpServerConnection->bytesAvailable();
        inBlock = tcpServerConnection->readAll();
        localFile->write(inBlock);
        inBlock.resize(0);
    }

    serverProgressBar->setMaximum(totalBytes);
    serverProgressBar->setValue(byteReceived);
    qDebug() << byteReceived;
    serverStatusLabel->setText(QStringLiteral("已接收 %1 Bytes").arg(byteReceived));

    if (byteReceived == totalBytes)
    {
        tcpServerConnection->close();
        startButton->setEnabled(true);
        localFile->fileName();
        localFile->close();
        start();
    }
}

void TcpFileServer::displayError(QAbstractSocket::SocketError socketError)
{
    QObject * server = qobject_cast<QObject *>(sender());
    if (server == tcpServerConnection) qDebug() << "Hi I am QTcpSocket";
    if (server == &tcpServer) qDebug() << "Hi I am QTcpServer";

    QMessageBox::information(this, QStringLiteral("網絡錯誤"),
                             QStringLiteral("產生如下錯誤: %1.").arg(tcpServerConnection->errorString()));
    tcpServerConnection->close();
    serverProgressBar->reset();
    serverStatusLabel->setText(QStringLiteral("伺服器就緒"));
    startButton->setEnabled(true);
}
