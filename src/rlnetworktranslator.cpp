#include "rlnetworktranslator.h"
//#include "../inc/MainFrame.h"

RlNetworkTranslator::RlNetworkTranslator(QObject *parent, int mode, const QString &hostName,
                                         quint16 listening_port, quint16 connecting_port,
                                         QAbstractSocket::NetworkLayerProtocol protocol)
    : NetworkTranslator()
{
    m_udpSocket = new QUdpSocket(this);
    m_tcpSocket = new QTcpSocket(this);
    m_inputData = new QByteArray();
    m_outputData = new QByteArray();
    m_mode = mode;
    m_hostName = hostName;
    m_listeningPort = listening_port;
    m_connectingPort = connecting_port;
    m_inputConnection = false;
    m_outputConnection = false;
    m_protocol = protocol;
    this->setParent(parent);
    //qRegisterMetaType<QAbstractSocket::SocketState>("QAbstractSocket::SocketState");
    connect(this, &RlNetworkTranslator::newDataToSend, this, &RlNetworkTranslator::write);
    connect(m_udpSocket, &QUdpSocket::stateChanged, this, &RlNetworkTranslator::checkUdpSocket);
    connect(m_tcpSocket, &QTcpSocket::stateChanged, this, &RlNetworkTranslator::checkTcpSocket);
    connect(this, &RlNetworkTranslator::socketBound, this, &RlNetworkTranslator::checkConnection);
    connect(this, &RlNetworkTranslator::connectedToServer, this, &RlNetworkTranslator::checkConnection);
    //connect(this, &RlNetworkTranslator::runProgram, dynamic_cast<CMainWindow*>(this->parent()), &CMainWindow::runProgramFinish);
}

RlNetworkTranslator::~RlNetworkTranslator()
{
    disconnect();
    connect(this, &RlNetworkTranslator::connectedToServer, this, &RlNetworkTranslator::checkConnection);
    connect(this, &RlNetworkTranslator::socketBound, this, &RlNetworkTranslator::checkConnection);
    connect(m_tcpSocket, &QTcpSocket::stateChanged, this, &RlNetworkTranslator::checkTcpSocket);
    connect(m_udpSocket, &QUdpSocket::stateChanged, this, &RlNetworkTranslator::checkUdpSocket);
    connect(this, &RlNetworkTranslator::newDataToSend, this, &RlNetworkTranslator::write);
    delete m_outputData;
    delete m_inputData;
    delete m_tcpSocket;
    delete m_udpSocket;
}

bool RlNetworkTranslator::setupListener(quint16 listening_port, quint16 server_port)
{
    QMessageBox box;
    if(!m_udpSocket->bind(QHostAddress::LocalHost, listening_port, QUdpSocket::DontShareAddress) ||
                listening_port == server_port)
    {
        switch(m_mode)
        {
        case NORMAL:
            box.setText("Port " + QString::number(listening_port) + " is busy. Choose another one");
            box.setIcon(QMessageBox::Warning);
            box.setStandardButtons(QMessageBox::Ok);
            box.exec();
            break;
        case CONSOLE:
            qInfo() << "Port " + QString::number(listening_port) + " is busy. Choose another one";
            break;
        default: break;
        }
    return false;
    }
    connect(m_udpSocket, &QUdpSocket::readyRead, this, &RlNetworkTranslator::read);
#if defined (CONNECTION_DEBUG)
    qDebug() << "Gui UDP socket listening port is:" << listening_port;
#endif
    return true;
}

bool RlNetworkTranslator::setupConnector(const QString& hostName, quint16 connecting_port,
                                          QAbstractSocket::NetworkLayerProtocol protocol)
{
    emit sendListeningPort(m_listeningPort);
    m_tcpSocket->connectToHost(hostName, connecting_port, QIODevice::WriteOnly, protocol);
    switch(m_mode)
    {
    case NORMAL:
        m_errorTimer = new QBasicTimer();
        m_tryTimer = new QBasicTimer();
        m_errorTimer->start(CONNECTING_TIMER, this);
        m_tryTimer->start(CONNECTING_DELAY, this);
        m_box = new QMessageBox();
        m_box->setText("Connecting to " + hostName + "; port: " + QString::number(connecting_port));
        m_box->setIcon(QMessageBox::Information);
        m_box->setStandardButtons(QMessageBox::NoButton);
        m_box->exec();
        break;
    case CONSOLE:
        qInfo() << "Connecting to " + hostName + "; port: " + QString::number(connecting_port);
        m_outputConnection = m_tcpSocket->waitForConnected();
        m_outputConnection ? qInfo() << "Connected succesfully" :
                           qInfo() << "Warning!: Connection failed";
        break;
    default: break;
    }
#if defined (CONNECTION_DEBUG)
    qDebug() << "Gui TCP socket peer port is:" << m_tcpSocket->peerPort();
    qDebug() << "Gui TCP socket local port is:" << m_tcpSocket->localPort();
#endif
    return m_outputConnection;
}

bool RlNetworkTranslator::setupConnection(const QString &hostName, quint16 listening_port, quint16 connecting_port,
                                           QAbstractSocket::NetworkLayerProtocol protocol, quint16 server_port)
{
    QMessageBox box;
    switch(m_mode)
    {
    case NORMAL:
        if(hostName.simplified() == "")
        {
            box.setText("Insert valid connection address please");
            box.setIcon(QMessageBox::Warning);
            box.exec();
            return false;
        }
        if(listening_port == NO_PORT)
        {
            box.setText("Insert valid listening port please");
            box.setIcon(QMessageBox::Warning);
            box.exec();
            return false;
        }
        if(connecting_port == NO_PORT)
        {
            box.setText("Insert valid connection port please");
            box.setIcon(QMessageBox::Warning);
            box.exec();
            return false;
        }
        m_hostName = hostName;
        m_listeningPort = listening_port;
        m_connectingPort = connecting_port;
        m_protocol = protocol;
        break;
    case CONSOLE:
        if(hostName.simplified() == "")
        {
            qInfo() << "Insert valid connection address please";
            return false;
        }
        if(listening_port == NO_PORT)
        {
            qInfo() << "Insert valid listening port please";
            return false;
        }
        if(connecting_port == NO_PORT)
        {
            qInfo() << "Insert valid connection port please";
            return false;
        }
        m_hostName = hostName;
        m_listeningPort = listening_port;
        m_connectingPort = connecting_port;
        m_protocol = protocol;
        break;
    case SILENT:
        if(hostName.simplified() == "")
        {
            return false;
        }
        if(listening_port == NO_PORT)
        {
            return false;
        }
        if(connecting_port == NO_PORT)
        {
            return false;
        }
        m_hostName = hostName;
        m_listeningPort = listening_port;
        m_connectingPort = connecting_port;
        m_protocol = protocol;
    }
    return setupListener(m_listeningPort, server_port)
           &&
           setupConnector(m_hostName, m_connectingPort, m_protocol);
}

void RlNetworkTranslator::disconnect()
{
    QObject::disconnect(m_udpSocket, &QUdpSocket::readyRead, this, &RlNetworkTranslator::read);
    m_udpSocket->close();
    m_tcpSocket->disconnectFromHost();
    m_outputConnection = false;
}

quint16 RlNetworkTranslator::getIport()
{
    return m_listeningPort;
}

quint16 RlNetworkTranslator::getOport()
{
    return m_connectingPort;
}

QByteArray *RlNetworkTranslator::getInputData()
{
    return m_inputData;
}

QByteArray *RlNetworkTranslator::getOutputData()
{
    return m_outputData;
}

QString RlNetworkTranslator::getMsg()
{
    return m_msg;
}

int RlNetworkTranslator::getMode()
{
    return m_mode;
}

void RlNetworkTranslator::setMode(int mode)
{
    m_mode = mode;
}

//void RlNetworkTranslator::timerEvent(QTimerEvent *e)
//{
//    e->accept();
//    QMessageBox box;
//    if(e->timerId() == m_errorTimer->timerId())
//    {
//        m_outputConnection = false;
//        m_tryTimer->stop();
//        m_errorTimer->stop();
//        m_box->done(INCORRECT_EXIT_CODE);
//        disconnect();
//        box.setText("Warning!: Connection failed");
//        box.setIcon(QMessageBox::Warning);
//        box.setStandardButtons(QMessageBox::Ok);
//        box.exec();
//    }
//    else if(e->timerId() == m_tryTimer->timerId())
//    {
//        m_tryTimer->start(CONNECTING_INTERVAL, this);
//        if(m_udpSocket->state() == QAbstractSocket::BoundState
//                && m_tcpSocket->state() == QTcpSocket::ConnectedState)
//        {
//            m_outputConnection = true;
//            m_errorTimer->stop();
//            m_tryTimer->stop();
//            m_box->done(CORRECT_EXIT_CODE);
//            box.setText("Connected succesfully");
//            box.setIcon(QMessageBox::Information);
//            box.setStandardButtons(QMessageBox::Ok);
//            box.exec();
//            emit tcpStateChanged(QTcpSocket::ConnectedState);
//        }
//    }
//}

void RlNetworkTranslator::timerEvent(QTimerEvent *e)
{
    e->accept();
    QMessageBox box;
    if(e->timerId() == m_errorTimer->timerId())
    {
        m_errorTimer->stop();
        m_box->done(INCORRECT_EXIT_CODE);
        disconnect();
        box.setText("Warning!: Connection failed");
        box.setIcon(QMessageBox::Warning);
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
    }
}

void RlNetworkTranslator::read()
{
    if(m_udpSocket->state() == QUdpSocket::BoundState)
    {
        while(m_udpSocket->hasPendingDatagrams())
        {
            QVariant result;
            QNetworkDatagram datagram = m_udpSocket->receiveDatagram();
            *m_inputData = datagram.data();
            QDataStream in(m_inputData, QIODevice::ReadOnly);
            //in.setVersion(QDataStream::Qt_5_13);
            in >> result;
#if defined (CONNECTION_DEBUG)
            qDebug() << "UDP transported data is:" << result;
            if(result.typeName() == QString("int").toLocal8Bit())
            {
                int a = decodeData<int>(result);
                qDebug() << "UDP transported data encoded is:" << a;
            }
#endif
            if(result.typeName() == QString("QString").toLocal8Bit())
            {
                m_msg = decodeData<QString>(result);
#if defined (CONNECTION_DEBUG)
                qDebug() << "UDP transported data encoded is:" << m_msg;
#endif
            }
            emit dataDelivered();
//#endif
        }
#if defined (CONNECTION_DEBUG)
        encodeData("UDP transmission succesful");
#endif
    }
}

void RlNetworkTranslator::write()
{
#if defined (CONNECTION_DEBUG)
    qDebug() << "TCP write bytes:" << m_tcpSocket->write(*m_outputData);
#else
    m_tcpSocket->write(*m_outputData);
#endif
    m_tcpSocket->waitForBytesWritten(-1);
    m_outputData->clear();
}

void RlNetworkTranslator::encodeData(QVariant source)
{
    QDataStream out(m_outputData, QIODevice::WriteOnly);
    //out.setVersion(QDataStream::Qt_5_13);
    out << source;
#if defined (CONNECTION_DEBUG)
    qDebug() << "Initial GUI data is" << source;
    qDebug() << "Initial GUI data encoded is" << source;
#endif
    emit newDataToSend();
}

void RlNetworkTranslator::checkConnection()
{
    if(m_inputConnection && m_outputConnection)
    {   emit connectionEstablished();   }
}

void RlNetworkTranslator::checkUdpSocket(QAbstractSocket::SocketState state)
{
    if(state == QAbstractSocket::BoundState)
    {
        m_inputConnection = true;
        emit socketBound();
    }
    else if(m_inputConnection && state != QAbstractSocket::BoundState)
    {
        m_inputConnection = false;
        emit socketClosed();
    }
    else
    {   m_inputConnection = false;  }
}

void RlNetworkTranslator::checkTcpSocket(QAbstractSocket::SocketState state)
{
    if(state == QAbstractSocket::ConnectedState)
    {
        QMessageBox box;
        switch(m_mode)
        {
        case NORMAL:
            if(m_errorTimer)
            {
                m_errorTimer->stop();
                delete m_errorTimer;
            }
            if(m_errorTimer)
            {
                m_tryTimer->stop();
                delete m_tryTimer;
            }
            if(m_errorTimer)
            {
                m_box->done(CORRECT_EXIT_CODE);
                delete m_box;
            }
            box.setText("Connected succesfully");
            box.setIcon(QMessageBox::Information);
            box.setStandardButtons(QMessageBox::Ok);
            box.exec();
            break;
        case CONSOLE:
            qInfo() << "Connected succesfully";
            break;
        }
        m_outputConnection = true;
        emit connectedToServer();
    }
    else if(m_outputConnection && state != QAbstractSocket::ConnectedState)
    {
        m_outputConnection = false;
        emit disconnectedFromServer();
    }
    else
    {   m_outputConnection = false; }
}

//void RlNetworkTranslator::changeInputStatus(QAbstractSocket::SocketState state)
//{

//}

//void RlNetworkTranslator::changeOutputStatus(QAbstractSocket::SocketState state)
//{

//}

template<typename T>
T RlNetworkTranslator::decodeData(QVariant data)
{
    T input = NULL;
    if(data.canConvert<T>())
    {
        input = data.value<T>();
    }
    else
    {
#if defined (CONNECTION_DEBUG)
        qDebug() << "Cannot convert to T";
#endif
    }
    return input;
}
