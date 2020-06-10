#include "simnetworktranslator.h"

SimNetworkTranslator::SimNetworkTranslator(QObject *parent, int mode, const QString &hostName, quint16 listening_port,
                                           quint16 connecting_port, QAbstractSocket::NetworkLayerProtocol protocol) :
    NetworkTranslator(), m_server(new QTcpServer(this)),  m_proto(nullptr), m_factoryLimits(nullptr)
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
    connect(m_udpSocket, &QUdpSocket::stateChanged, this, &SimNetworkTranslator::checkUdpSocket);
    connect(m_tcpSocket, &QTcpSocket::stateChanged, this, &SimNetworkTranslator::checkTcpSocket);
    connect(this, &SimNetworkTranslator::connectedToClt, this, &SimNetworkTranslator::checkConnection);
    connect(this, &SimNetworkTranslator::cltConnected, this, &SimNetworkTranslator::checkConnection);
}

SimNetworkTranslator::~SimNetworkTranslator()
{
    QObject::disconnect(this, &SimNetworkTranslator::cltConnected, this,
                        &SimNetworkTranslator::checkConnection);
    QObject::disconnect(this, &SimNetworkTranslator::connectedToClt, this,
                        &SimNetworkTranslator::checkConnection);
    QObject::disconnect(m_tcpSocket, &QTcpSocket::stateChanged, this,
                        &SimNetworkTranslator::checkConnection);
    QObject::disconnect(m_udpSocket, &QUdpSocket::stateChanged, this,
                        &SimNetworkTranslator::checkConnection);
    delete m_outputData;
    delete m_inputData;
    delete m_tcpSocket;
    delete m_proto;
    delete m_server;
    delete m_udpSocket;
}

bool SimNetworkTranslator::setupConnection(const QString &hostName, quint16 listening_port,
                                           quint16 connecting_port,
                                           QAbstractSocket::NetworkLayerProtocol protocol,
                                           quint16 server_port)
{
    m_protocol = protocol;
    if(hostName.simplified() != "")
    {
        m_hostName = hostName;
    }
    if(listening_port == server_port && listening_port != NO_PORT)
    {
        m_listeningPort = listening_port;
    }
    if(connecting_port != NO_PORT)
    {
        m_connectingPort = connecting_port;
    }

    m_udpSocket->connectToHost(m_hostName, m_connectingPort, QIODevice::WriteOnly, m_protocol);
    if(!m_udpSocket->waitForConnected())
    {
        switch(m_mode)
        {
        case NORMAL:
            emit sendMsgDialog(std::tuple<MsgDialogType, int, QString,
                               QDialogButtonBox::StandardButtons>(
                                   QMsgBox, QMessageBox::Information,
                                   "Server cannot connect to GUI",
                                   QDialogButtonBox::Ok));
            break;
        case CONSOLE:
            qInfo() << "Server cannot connect to GUI";
            break;
        default:
            break;
        }
        return false;
    }
    connect(this, &SimNetworkTranslator::newDataToSend, this,  &SimNetworkTranslator::write);
#if defined (CONNECTION_DEBUG)
    qDebug() << "Simulator UDP socket peer port is:" << m_udpSocket->peerPort();
    qDebug() << "Simulator UDP socket local port is:" << m_udpSocket->localPort();
#endif

    switch(m_mode)
    {
    case NORMAL:
        emit sendMsgDialog(std::tuple<MsgDialogType, int, QString,
                           QDialogButtonBox::StandardButtons>(
                               QMsgBox, QMessageBox::Information,
                               "Server succesfully connected to GUI",
                               QDialogButtonBox::Ok));
        break;
    case CONSOLE:
        qInfo() << "Server succesfully connected to GUI";
        break;
    default:
        break;
    }
    return true;
}

void SimNetworkTranslator::disconnect()
{
    QObject::disconnect(this, &SimNetworkTranslator::newDataToSend, this, &SimNetworkTranslator::write);
    m_udpSocket->disconnectFromHost();
}

QTcpServer *SimNetworkTranslator::getServer()
{
    return m_server;
}

QSettings *SimNetworkTranslator::getSettings()
{
    return m_factoryLimits;
}

quint16 SimNetworkTranslator::getIport()
{
    return m_listeningPort;
}

quint16 SimNetworkTranslator::getOport()
{
    return m_connectingPort;
}

QByteArray *SimNetworkTranslator::getInputData()
{
    return m_inputData;
}

QByteArray *SimNetworkTranslator::getOutputData()
{
    return m_outputData;
}

QString SimNetworkTranslator::getMsg()
{
    return m_msg;
}

int SimNetworkTranslator::getMode()
{
    return m_mode;
}

QString SimNetworkTranslator::getOutputMsg()
{
    return m_oMsg;
}

StandProto *SimNetworkTranslator::getProto()
{
    return m_proto;
}

void SimNetworkTranslator::setMode(int mode)
{
    m_mode = mode;
}

void SimNetworkTranslator::setProto(StandProto *proto)
{
    m_proto = proto;
}

void SimNetworkTranslator::setSettings(QString filename, QSettings::Format format)
{
    m_factoryLimits = new QSettings(filename, format);
}

bool SimNetworkTranslator::setupServer(QHostAddress server_address, quint16 server_port)
{
    int count = 0;
    switch (m_mode)
    {
    case NORMAL:
        while(!m_server->listen(server_address, server_port))
        {
            emit sendServerCode(INCORRECT_EXIT_CODE);
            emit sendMsgDialog(std::tuple<MsgDialogType, int, QString,
                               QDialogButtonBox::StandardButtons>(
                                   SimMsgDlg, QMessageBox::Warning, "Port " +
                                   QString::number(m_listeningPort) +
                                   " is unavailable. Choose another one\n(empty" +
                                   " input will lead to a random free port choice):",
                                   QDialogButtonBox::Ok | QDialogButtonBox::Cancel));
            return false;
        }
        emit sendMsgDialog(std::tuple<MsgDialogType, int, QString,
                           QDialogButtonBox::StandardButtons>(
                               QMsgBox, QMessageBox::Information, "Server is listening on " +
                               QString::number(m_server->serverPort()) + " port",
                               QDialogButtonBox::Ok));
        break;
    case CONSOLE:
        while(!m_server->listen(server_address, server_port) && count < 1000)
        {
            qInfo() << "Port " + QString::number(m_listeningPort) + " is unavailable";
            m_listeningPort = 0;
            count++;
        }
        if(m_server->isListening())
        {   qInfo() << "Server is listening on " + QString::number(m_server->serverPort()) +
                       " port";   }
        else
        {
            qInfo() << "Server is not listening";
            return false;
        }
        break;
    case SILENT:
        while(!m_server->listen(server_address, server_port) && count < 1000)
        {
            m_listeningPort = 0;
            count++;
        }
        if(!m_server->isListening())
        {   return false;   }
    }
    m_listeningPort = m_server->serverPort();
    m_proto = new StandProto(m_factoryLimits);
    connect(m_server, &QTcpServer::newConnection, this, &SimNetworkTranslator::connectSocket);
    emit sendServerCode(CORRECT_EXIT_CODE);
#if defined (CONNECTION_DEBUG)
    qDebug() << "Simulator server listening port is:" << m_server->serverPort();
#endif
    return true;
}

void SimNetworkTranslator::stopServer()
{
    disconnect();
    if(m_tcpSocket->isOpen())
    {
        if(m_tcpSocket->bytesAvailable() != NO_DATA)
        {   *m_inputData = m_tcpSocket->readAll();  }
        QObject::disconnect(m_tcpSocket, &QTcpSocket::readyRead, this,
                            &SimNetworkTranslator::read);
        m_tcpSocket->close();
    }
    QObject::disconnect(m_server, &QTcpServer::newConnection, this,
                        &SimNetworkTranslator::connectSocket);
    m_server->close();
}

void SimNetworkTranslator::connectSocket()
{
    m_tcpSocket = m_server->nextPendingConnection();
    if(m_tcpSocket->waitForConnected())
    {   connect(m_tcpSocket, &QTcpSocket::readyRead, this, &SimNetworkTranslator::read);    }
    setupConnection(m_hostName, m_listeningPort, m_connectingPort, QAbstractSocket::IPv4Protocol,
                    m_listeningPort);
#if defined (CONNECTION_DEBUG)
    qDebug() << "Simulator TCP socket state: ";
    returnState(m_tcpSocket->state());
#endif
}

void SimNetworkTranslator::read()
{
    if(m_tcpSocket->state() == QTcpSocket::ConnectedState)
    {
        while(m_tcpSocket->bytesAvailable() )
        {
            QVariant result;
            QDataStream in(m_inputData, QIODevice::ReadOnly);
            //in.setVersion(QDataStream::Qt_5_13);
            in.setDevice(m_tcpSocket);
            in.startTransaction();
            if(!in.commitTransaction())
            {   return;   }
            in >> result;
#if defined (CONNECTION_DEBUG)
            qDebug() << "TCP transported data is:" << result;
            qDebug() << "int typeid" << typeid(int).name();
            if(result.typeName() == QString("int").toLocal8Bit())
            {
                int a = decodeData<int>(result);
                qDebug() << "TCP transported data encoded is:" << a;
            }
#endif
            if(result.typeName() == QString("QString").toLocal8Bit())
            {
                m_msg = decodeData<QString>(result);
#if defined (CONNECTION_DEBUG)
                qDebug() << "TCP transported data encoded is:" << m_msg;
#endif
                doProtoAnswer();
            }
            emit dataDelivered();
//#endif
        }
#if defined (CONNECTION_DEBUG)
        //encodeData("TCP transmission succesful");
#endif
    }
}

void SimNetworkTranslator::write()
{
#if defined (CONNECTION_DEBUG)
    qDebug() << "UDP write bytes:" << m_udpSocket->write(*m_outputData);
#else
    m_udpSocket->write(*m_outputData);
#endif
    m_udpSocket->waitForBytesWritten(-1);
    m_outputData->clear();
}

void SimNetworkTranslator::setConnectingPort(quint16 port)
{
    m_connectingPort = port;
}

void SimNetworkTranslator::encodeData(QVariant source)
{
    if(source.canConvert<QString>())
    {   m_oMsg = source.value<QString>();   }
    QDataStream out(m_outputData, QIODevice::WriteOnly);
    //out.setVersion(QDataStream::Qt_5_13);
    out << source;
#if defined (CONNECTION_DEBUG)
    qDebug() << "Initial reply data is" << source;
    qDebug() << "Initial reply data encoded is" << source;
#endif
    emit newDataToSend();
}

void SimNetworkTranslator::doProtoAnswer()
{

    /* powerOn && powerOff */

    if(m_msg == "*ESE?")
    {   encodeData(QString::number(static_cast<int>(m_proto->getStandEventReg()))); }

    /* RTready */

    else if(m_msg == "*TST?")
    {   encodeData(QVariant(m_proto->getReady()).toString());   }

    /* RTgetPosition */

    else if(m_msg.startsWith(":Read:Position"))
    {
        if(m_msg.split(" ")[1].startsWith("1"))
        {   encodeData(FltToStr::toQString(m_proto->getPos().x()));    }
        else if(m_msg.split(" ")[1].startsWith("2"))
        {   encodeData(FltToStr::toQString(m_proto->getPos().y()));    }
        else if(m_msg.split(" ")[1].startsWith("3"))
        {   encodeData(FltToStr::toQString(m_proto->getPos().z()));    }
    }

    /* RTgetVelocity */

    else if(m_msg.startsWith(":Read:Rate"))
    {
        if(m_msg.split(" ")[1].startsWith("1"))
        {   encodeData(FltToStr::toQString(m_proto->getVel().x()));    }
        else if(m_msg.split(" ")[1].startsWith("2"))
        {   encodeData(FltToStr::toQString(m_proto->getVel().y()));    }
        else if(m_msg.split(" ")[1].startsWith("3"))
        {   encodeData(FltToStr::toQString(m_proto->getVel().z()));    }
    }

    /* RTgetAcceleration */

    else if(m_msg.startsWith(":Read:Acceleration"))
    {
        if(m_msg.split(" ")[1].startsWith("1"))
        {   encodeData(FltToStr::toQString(m_proto->getAcc().x()));    }
        else if(m_msg.split(" ")[1].startsWith("2"))
        {   encodeData(FltToStr::toQString(m_proto->getAcc().y()));    }
        else if(m_msg.split(" ")[1].startsWith("3"))
        {   encodeData(FltToStr::toQString(m_proto->getAcc().z()));    }
    }

    /* RTgetState */

    else if(m_msg.startsWith(":Read:Vector"))
    {
        if(m_msg.split(" ")[1].startsWith("1"))
        {
            encodeData(FltToStr::toQString(m_proto->getPos().x()) + "," +
                       FltToStr::toQString(m_proto->getVel().x()) + "," +
                       FltToStr::toQString(m_proto->getAcc().x()));
        }
        else if(m_msg.split(" ")[1].startsWith("2"))
        {
            encodeData(FltToStr::toQString(m_proto->getPos().y()) + "," +
                       FltToStr::toQString(m_proto->getVel().y()) + "," +
                       FltToStr::toQString(m_proto->getAcc().y()));
        }
        else if(m_msg.split(" ")[1].startsWith("3"))
        {
            encodeData(FltToStr::toQString(m_proto->getPos().z()) + "," +
                       FltToStr::toQString(m_proto->getVel().z()) + "," +
                       FltToStr::toQString(m_proto->getAcc().z()));
        }
    }

    /* RTgetMaxPositionLimit && RTgetMaxLimits */

    else if(m_msg.startsWith(":Limit:Hposition?"))
    {
        if(m_msg.split(",")[1].startsWith("1"))
        {   encodeData(FltToStr::toQString(m_proto->getMaxPosLimits().x())); }
        else if(m_msg.split(",")[1].startsWith("2"))
        {   encodeData(FltToStr::toQString(m_proto->getMaxPosLimits().y())); }
        else if(m_msg.split(",")[1].startsWith("3"))
        {   encodeData(FltToStr::toQString(m_proto->getMaxPosLimits().z())); }
    }

    /* RTgetMinPositionLimit && RTgetMinLimits */

    else if(m_msg.startsWith(":Limit:Lposition?"))
    {
        if(m_msg.split(",")[1].startsWith("1"))
        {   encodeData(FltToStr::toQString(m_proto->getMinPosLimits().x())); }
        else if(m_msg.split(",")[1].startsWith("2"))
        {   encodeData(FltToStr::toQString(m_proto->getMinPosLimits().y())); }
        else if(m_msg.split(",")[1].startsWith("3"))
        {   encodeData(FltToStr::toQString(m_proto->getMinPosLimits().z())); }
    }

    /* RTgetMaxAccelerationLimit && RTgetMinAccelerationLimit && RTgetMaxLimits &&
     * RTgetMinLimits */

    else if(m_msg.startsWith(":Limit:Acceleration?"))
    {
        if(m_msg.split(",")[1].startsWith("1"))
        {   encodeData(FltToStr::toQString(m_proto->getAccLimits().x())); }
        else if(m_msg.split(",")[1].startsWith("2"))
        {   encodeData(FltToStr::toQString(m_proto->getAccLimits().y())); }
        else if(m_msg.split(",")[1].startsWith("3"))
        {   encodeData(FltToStr::toQString(m_proto->getAccLimits().z())); }
    }

    /* RTgetMaxVelocityLimit && RTgetMinVelocityLimit && RTgetMaxLimits && RTgetMinLimits */

    else if(m_msg.startsWith(":Limit:Rate?"))
    {
        if(m_msg.split(",")[1].startsWith("1"))
        {   encodeData(FltToStr::toQString(m_proto->getVelLimits().x())); }
        else if(m_msg.split(",")[1].startsWith("2"))
        {   encodeData(FltToStr::toQString(m_proto->getVelLimits().y())); }
        else if(m_msg.split(",")[1].startsWith("3"))
        {   encodeData(FltToStr::toQString(m_proto->getVelLimits().z())); }
    }

    /* RTgetFactoryMaxLimits && RTgetFactoryMinLimits && RTgetFactoryMinAccelerationLimit &&
     * RTgetFactoryMaxAccelerationLimit && RTgetFactoryMinVelocityLimit &&
     * RTgetFactoryMaxVelocityLimit && RTgetFactoryMinPositionLimit &&
     * RTgetFactoryMaxPositionLimit */

    else if(m_msg.startsWith(":Limit:Absolute?"))
    {
        if(m_msg.split(" ")[1].startsWith("1"))
        {   encodeData(FltToStr::toQString(m_proto->getFactoryVelLimits().x()) + "," +
                       FltToStr::toQString(m_proto->getFactoryAccLimits().x()) + ", ," +
                       FltToStr::toQString(m_proto->getMinFactoryPosLimits().x()) + "," +
                       FltToStr::toQString(m_proto->getMaxFactoryPosLimits().x())); }
        else if(m_msg.split(" ")[1].startsWith("2"))
        {   encodeData(FltToStr::toQString(m_proto->getFactoryVelLimits().y()) + "," +
                       FltToStr::toQString(m_proto->getFactoryAccLimits().y()) + ", ," +
                       FltToStr::toQString(m_proto->getMinFactoryPosLimits().y()) + "," +
                       FltToStr::toQString(m_proto->getMaxFactoryPosLimits().y())); }
        else if(m_msg.split(" ")[1].startsWith("3"))
        {   encodeData(FltToStr::toQString(m_proto->getFactoryVelLimits().z()) + "," +
                       FltToStr::toQString(m_proto->getFactoryAccLimits().z()) + ", ," +
                       FltToStr::toQString(m_proto->getMinFactoryPosLimits().z()) + "," +
                       FltToStr::toQString(m_proto->getMaxFactoryPosLimits().z())); }
    }

    /* RTgetInterlock && setAxis */

    else if(m_msg.startsWith(":Interlock"))
    {
        if(m_msg.startsWith(":Interlock?"))
        {
            if(m_msg.split(" ")[1].startsWith("1"))
            {   m_proto->getActiveAxis() > 0 ? encodeData("Open") : encodeData("Closed"); }
            else if(m_msg.split(" ")[1].startsWith("2"))
            {   m_proto->getActiveAxis() > 1 ? encodeData("Open") : encodeData("Closed"); }
            else if(m_msg.split(" ")[1].startsWith("3"))
            {   m_proto->getActiveAxis() > 3 ? encodeData("Open") : encodeData("Closed"); }
        }
        else
        {
            for(auto num : m_msg.split(":", QString::SkipEmptyParts)[1].split(";"))
            {
                if(num.split(" ")[0] == "Open")
                {
                    if(num.split(" ")[1] == "ALL")
                    {
                        m_proto->setActiveAxis(7);
                    }
                    else
                    {
                        switch(num.split(" ")[1].toInt())
                        {
                        case 1:
                            if(m_proto->getActiveAxis() != 1 &&
                               m_proto->getActiveAxis() != 3 &&
                               m_proto->getActiveAxis() != 5 &&
                               m_proto->getActiveAxis() != 7)
                            {   m_proto->setActiveAxis(m_proto->getActiveAxis() + 1);   }
                            break;
                        case 2:
                            if(m_proto->getActiveAxis() != 2 &&
                               m_proto->getActiveAxis() != 3 &&
                               m_proto->getActiveAxis() != 6 &&
                               m_proto->getActiveAxis() != 7)
                            {   m_proto->setActiveAxis(m_proto->getActiveAxis() + 2);   }
                            break;
                        case 3:
                            if(m_proto->getActiveAxis() != 4 &&
                               m_proto->getActiveAxis() != 5 &&
                               m_proto->getActiveAxis() != 6 &&
                               m_proto->getActiveAxis() != 7)
                            {   m_proto->setActiveAxis(m_proto->getActiveAxis() + 4);   }
                            break;
                        }
                    }
                }
                else
                {
                    if(num.split(" ")[1] == "ALL")
                    {
                         m_proto->setActiveAxis(0);
                    }
                    else
                    {
                        switch(num.split(" ")[1].toInt())
                        {
                        case 1:
                            if(m_proto->getActiveAxis() == 1 ||
                               m_proto->getActiveAxis() == 3 ||
                               m_proto->getActiveAxis() == 5 ||
                               m_proto->getActiveAxis() == 7)
                            {   m_proto->setActiveAxis(m_proto->getActiveAxis() - 1);   }
                            break;
                        case 2:
                            if(m_proto->getActiveAxis() == 2 ||
                               m_proto->getActiveAxis() == 3 ||
                               m_proto->getActiveAxis() == 6 ||
                               m_proto->getActiveAxis() == 7)
                            {   m_proto->setActiveAxis(m_proto->getActiveAxis() - 2);   }
                            break;
                        case 3:
                            if(m_proto->getActiveAxis() == 4 ||
                               m_proto->getActiveAxis() == 5 ||
                               m_proto->getActiveAxis() == 6 ||
                               m_proto->getActiveAxis() == 7)
                            {   m_proto->setActiveAxis(m_proto->getActiveAxis() - 4);   }
                            break;
                        }
                    }
                }
            }
            valueSet();
        }
    }

    /* startRotation */

    else if(m_msg.startsWith(":Mode:Rate"))
    {
        if(m_msg.split(";")[1].startsWith(":Demand:Rate"))
        {
            if(m_msg.split(";")[1].split(" ")[1].split(",")[0] == "ALL")
            {
                m_proto->setVel(QVector3D(m_msg.split(";")[1].split(" ")[1]
                                                             .split(",")[1].toFloat(),
                                          m_msg.split(";")[1].split(" ")[1]
                                                             .split(",")[1].toFloat(),
                                          m_msg.split(";")[1].split(" ")[1]
                                                             .split(",")[1].toFloat()));
                if(m_proto->getMoving() != 7)
                {   m_proto->setMoving(7);  }
            }
            else
            {
                switch (m_msg.split(";")[1].split(" ")[1].split(",")[0].toInt())
                {
                case 1:
                    m_proto->setVel(QVector3D(m_msg.split(";")[1].split(" ")[1]
                                                                 .split(",")[1].toFloat(),
                                              m_proto->getVel().y(), m_proto->getVel().z()));
                    if(m_proto->getMoving() != 1 &&
                       m_proto->getMoving() != 3 &&
                       m_proto->getMoving() != 5 &&
                       m_proto->getMoving() != 7)
                    {   m_proto->setMoving(1);   }
                    break;
                case 2:
                    m_proto->setVel(QVector3D(m_proto->getVel().x(),
                                              m_msg.split(";")[1].split(" ")[1]
                                                                 .split(",")[1].toFloat(),
                                              m_proto->getVel().z()));
                    if(m_proto->getMoving() != 2 &&
                       m_proto->getMoving() != 3 &&
                       m_proto->getMoving() != 6 &&
                       m_proto->getMoving() != 7)
                    {   m_proto->setMoving(2);   }
                    break;
                case 3:
                    m_proto->setVel(QVector3D(m_proto->getVel().x(),
                                              m_proto->getVel().y(),
                                              m_msg.split(";")[1].split(" ")[1]
                                                   .split(",")[1].toFloat()));
                    if(m_proto->getMoving() != 4 &&
                       m_proto->getMoving() != 5 &&
                       m_proto->getMoving() != 6 &&
                       m_proto->getMoving() != 7)
                    {   m_proto->setMoving(4);   }
                    break;
                }
            }
            valueSet();
        }
    }

    /* RTsetMaxPositionLimit */

    else if(m_msg.startsWith(":Limit:Hposition"))
    {
        if(m_msg.split(" ")[1].split(",")[1] == "ALL")
        {
            m_proto->setMaxPosLimits(QVector3D(m_msg.split(" ")[1].split(",")[2].toFloat(),
                                               m_msg.split(" ")[1].split(",")[2].toFloat(),
                                               m_msg.split(" ")[1].split(",")[2].toFloat()));
        }
        else
        {
            switch (m_msg.split(" ")[1].split(",")[1].toInt())
            {
            case 1:
                m_proto->setMaxPosLimits(QVector3D(m_msg.split(" ")[1].split(",")[2].toFloat(),
                                                   m_proto->getMaxPosLimits().y(),
                                                   m_proto->getMaxPosLimits().z()));
                break;
            case 2:
                m_proto->setMaxPosLimits(QVector3D(m_proto->getMaxPosLimits().x(),
                                                   m_msg.split(" ")[1].split(",")[2].toFloat(),
                                                   m_proto->getMaxPosLimits().z()));
                break;
            case 3:
                m_proto->setMaxPosLimits(QVector3D(m_proto->getMaxPosLimits().x(),
                                                   m_proto->getMaxPosLimits().y(),
                                                   m_msg.split(" ")[1].split(",")[2].toFloat()));
                break;
            }
        }
        valueSet();
    }

    /* RTsetMinPositionLimit */

    else if(m_msg.startsWith(":Limit:Lposition"))
    {
        if(m_msg.split(" ")[1].split(",")[1] == "ALL")
        {
            m_proto->setMinPosLimits(QVector3D(m_msg.split(" ")[1].split(",")[2].toFloat(),
                                               m_msg.split(" ")[1].split(",")[2].toFloat(),
                                               m_msg.split(" ")[1].split(",")[2].toFloat()));
        }
        else
        {
            switch (m_msg.split(" ")[1].split(",")[1].toInt())
            {
            case 1:
                m_proto->setMinPosLimits(QVector3D(m_msg.split(" ")[1].split(",")[2].toFloat(),
                                                   m_proto->getMinPosLimits().y(),
                                                   m_proto->getMinPosLimits().z()));
                break;
            case 2:
                m_proto->setMinPosLimits(QVector3D(m_proto->getMinPosLimits().x(),
                                                   m_msg.split(" ")[1].split(",")[2].toFloat(),
                                                   m_proto->getMinPosLimits().z()));
                break;
            case 3:
                m_proto->setMinPosLimits(QVector3D(m_proto->getMinPosLimits().x(),
                                                   m_proto->getMinPosLimits().y(),
                                                   m_msg.split(" ")[1].split(",")[2].toFloat()));
                break;
            }
        }
        valueSet();
    }

    /* RTsetMaxVelocityLimit && RTsetMinVelocityLimit */

    else if(m_msg.startsWith(":Limit:Rate"))
    {
        if(m_msg.split(" ")[1].split(",")[1] == "ALL")
        {
            m_proto->setVelLimits(QVector3D(m_msg.split(" ")[1].split(",")[2].toFloat(),
                                               m_msg.split(" ")[1].split(",")[2].toFloat(),
                                               m_msg.split(" ")[1].split(",")[2].toFloat()));
        }
        else
        {
            switch (m_msg.split(" ")[1].split(",")[1].toInt())
            {
            case 1:
                m_proto->setVelLimits(QVector3D(m_msg.split(" ")[1].split(",")[2].toFloat(),
                                                m_proto->getVelLimits().y(),
                                                m_proto->getVelLimits().z()));
                break;
            case 2:
                m_proto->setVelLimits(QVector3D(m_proto->getVelLimits().x(),
                                                m_msg.split(" ")[1].split(",")[2].toFloat(),
                                                m_proto->getVelLimits().z()));
                break;
            case 3:
                m_proto->setVelLimits(QVector3D(m_proto->getVelLimits().x(),
                                                m_proto->getVelLimits().y(),
                                                m_msg.split(" ")[1].split(",")[2].toFloat()));
                break;
            }
        }
        valueSet();
    }

    /* RTsetMaxAccelerationLimit && RTsetMinAccelerationLimit */

    else if(m_msg.startsWith(":Limit:Acceleration"))
    {
        if(m_msg.split(" ")[1].split(",")[1] == "ALL")
        {
            m_proto->setAccLimits(QVector3D(m_msg.split(" ")[1].split(",")[2].toFloat(),
                                               m_msg.split(" ")[1].split(",")[2].toFloat(),
                                               m_msg.split(" ")[1].split(",")[2].toFloat()));
        }
        else
        {
            switch (m_msg.split(" ")[1].split(",")[1].toInt())
            {
            case 1:
                m_proto->setAccLimits(QVector3D(m_msg.split(" ")[1].split(",")[2].toFloat(),
                                                m_proto->getAccLimits().y(),
                                                m_proto->getAccLimits().z()));
                break;
            case 2:
                m_proto->setAccLimits(QVector3D(m_proto->getAccLimits().x(),
                                                m_msg.split(" ")[1].split(",")[2].toFloat(),
                                                m_proto->getAccLimits().z()));
                break;
            case 3:
                m_proto->setAccLimits(QVector3D(m_proto->getAccLimits().x(),
                                                m_proto->getAccLimits().y(),
                                                m_msg.split(" ")[1].split(",")[2].toFloat()));
                break;
            }
        }
        valueSet();
    }

    /* sinusMove && sinusRelMove */

    else if(m_msg.startsWith(":Mode:Synthesis"))
    {
        if(m_msg.split(" ")[1] == "ALL")
        {
            if(m_proto->getMoving() != 7)
            {   m_proto->setMoving(7);  }
            m_proto->setVel(QVector3D(50., 50., 50.));
        }
        else
        {
            switch(m_msg.split(" ")[1].toInt())
            {
            case 1:
                if(m_proto->getMoving() != 1 &&
                   m_proto->getMoving() != 3 &&
                   m_proto->getMoving() != 5 &&
                   m_proto->getMoving() != 7)
                {   m_proto->setMoving(1);   }
                m_proto->setVel(QVector3D(50., m_proto->getVel().y(), m_proto->getVel().z()));
                break;
            case 2:
                if(m_proto->getMoving() != 2 &&
                   m_proto->getMoving() != 3 &&
                   m_proto->getMoving() != 6 &&
                   m_proto->getMoving() != 7)
                {   m_proto->setMoving(2);   }
                m_proto->setVel(QVector3D(m_proto->getVel().x(), 50., m_proto->getVel().z()));
                break;
            case 3:
                if(m_proto->getMoving() != 4 &&
                   m_proto->getMoving() != 5 &&
                   m_proto->getMoving() != 6 &&
                   m_proto->getMoving() != 7)
                {   m_proto->setMoving(4);   }
                m_proto->setVel(QVector3D(m_proto->getVel().x(), m_proto->getVel().y(), 50.));
                break;
            }
        }
        QThread::currentThread()->msleep(1);
        valueSet();
    }

    /* RTsetPosition && RTsetRelPosition */

    else if(m_msg.startsWith(":Demand:Svector"))
    {
        switch(m_msg.split(" ")[1].split(",")[0].toInt())
        {
        case 1:
            m_proto->setPos(QVector3D(m_msg.split(" ")[1].split(",")[1].toFloat(),
                                      m_proto->getPos().y(), m_proto->getPos().z()));
            m_proto->setVel(QVector3D(m_msg.split(" ")[1].split(",")[2].toFloat(),
                                      m_proto->getVel().y(), m_proto->getVel().z()));
            break;
        case 2:
            m_proto->setPos(QVector3D(m_proto->getPos().x(),
                                      m_msg.split(" ")[1].split(",")[1].toFloat(),
                                      m_proto->getPos().z()));
            m_proto->setVel(QVector3D(m_proto->getVel().x(),
                                      m_msg.split(" ")[1].split(",")[2].toFloat(),
                                      m_proto->getVel().z()));
            break;
        case 3:
            m_proto->setPos(QVector3D(m_proto->getPos().x(),
                                      m_proto->getPos().y(),
                                      m_msg.split(" ")[1].split(",")[1].toFloat()));
            m_proto->setVel(QVector3D(m_proto->getVel().x(),
                                      m_proto->getVel().y(),
                                      m_msg.split(" ")[1].split(",")[2].toFloat()));
            break;
        }
        valueSet();
    }

    /* stopRotation */

    else if(m_msg.startsWith(":Mode:Off"))
    {
        if(m_msg.split(" ")[1] == "ALL")
        {
            m_proto->setMoving(m_proto->getMoving());
            m_proto->setAcc(QVector3D(.0, .0, .0));
        }
        else
        {
            switch(m_msg.split(" ")[1].toInt())
            {
            case 1:
                if(m_proto->getMoving() == 1 ||
                   m_proto->getMoving() == 3 ||
                   m_proto->getMoving() == 5 ||
                   m_proto->getMoving() == 7)
                {
                    m_proto->setMoving(1);
                    m_proto->setVel(QVector3D(.0, m_proto->getVel().y(),
                                              m_proto->getVel().z()));
                }
                break;
            case 2:
                if(m_proto->getMoving() == 2 ||
                   m_proto->getMoving() == 3 ||
                   m_proto->getMoving() == 6 ||
                   m_proto->getMoving() == 7)
                {
                    m_proto->setMoving(2);
                    m_proto->setVel(QVector3D(m_proto->getVel().x(),
                                              .0, m_proto->getVel().z()));
                }
                break;
            case 3:
                if(m_proto->getMoving() == 4 ||
                   m_proto->getMoving() == 5 ||
                   m_proto->getMoving() == 6 ||
                   m_proto->getMoving() == 7)
                {
                    m_proto->setMoving(4);
                    m_proto->setVel(QVector3D(m_proto->getVel().x(),
                                              m_proto->getVel().y(), .0));
                }
                break;
            }
        }
        valueSet();
    }

    else
    {   encodeData(m_msg);  }
}

template<typename T>
T SimNetworkTranslator::decodeData(QVariant data)
{
    T input = nullptr;
    if(data.canConvert<T>())
    {   input = data.value<T>();    }
#if defined (CONNECTION_DEBUG)
    else
    {   qDebug() << "Cannot convert to T";  }
#endif
    return input;
}

void SimNetworkTranslator::checkConnection()
{
    if(m_inputConnection && m_outputConnection)
    {   emit connectionEstablished();   }
}

void SimNetworkTranslator::checkUdpSocket(QAbstractSocket::SocketState state)
{
    if(state == QAbstractSocket::ConnectedState)
    {
        m_outputConnection = true;
        emit connectedToClt();
    }
    else if(m_outputConnection && state != QAbstractSocket::ConnectedState)
    {
        m_outputConnection = false;
        emit disconnectedFromClt();
    }
    else
    {   m_outputConnection = false; }
}

void SimNetworkTranslator::checkTcpSocket(QAbstractSocket::SocketState state)
{
    if(state == QAbstractSocket::ConnectedState)
    {
        m_inputConnection = true;
        emit cltConnected();
    }
    else if(m_inputConnection && state != QAbstractSocket::ConnectedState)
    {
        m_inputConnection = false;
        emit cltDisconnected();
    }
    else
    {   m_inputConnection = false;  }
}
