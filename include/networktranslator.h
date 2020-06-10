#ifndef NETWORKTRANSLATOR_H
#define NETWORKTRANSLATOR_H

#include <QUdpSocket>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QHostAddress>
#include <QBasicTimer>
#include <QMessageBox>
#include <QTimerEvent>

#define STANDART_LISTENING_PORT 6000
#define STANDART_CONNECTION_PORT 6001
#define CORRECT_EXIT_CODE 0
#define INCORRECT_EXIT_CODE -1
#define NO_MAX_SIZE -1
#define NO_PORT 0
#define NO_DATA 0
#define NO_ADDRESS ""

enum TestingMode
{
    ON,
    OFF
};

enum MessageMode
{
    NORMAL,
    CONSOLE,
    SILENT
};

class NetworkTranslator : public QObject
{

    Q_OBJECT

protected:
    QUdpSocket  *m_udpSocket;
    QTcpSocket  *m_tcpSocket;
    QByteArray  *m_inputData;
    QByteArray  *m_outputData;

    QString     m_hostName;
    quint16     m_listeningPort;
    quint16     m_connectingPort;
    bool        m_outputConnection;
    bool        m_inputConnection;
    QAbstractSocket::NetworkLayerProtocol m_protocol;
    QString     m_msg;
    int         m_mode;

public:
    virtual ~NetworkTranslator();
    void returnState(QAbstractSocket::SocketState state);

    virtual quint16 getIport() = 0;
    virtual quint16 getOport() = 0;
    virtual QByteArray* getInputData() = 0;
    virtual QByteArray* getOutputData() = 0;
    virtual QString getMsg() = 0;
    virtual int getMode() = 0;

    virtual void setMode(int mode) = 0;

signals:
    void newDataToSend();
    void connectionEstablished();
    void dataDelivered();

private slots:
    virtual void checkConnection() = 0;
    virtual void checkUdpSocket(QAbstractSocket::SocketState state) = 0;
    virtual void checkTcpSocket(QAbstractSocket::SocketState state) = 0;

public slots:
    virtual bool setupConnection(const QString &hostName, quint16 listening_port, quint16 connecting_port,
                                 QAbstractSocket::NetworkLayerProtocol protocol, quint16 server_port) = 0;
    virtual void disconnect() = 0;
    virtual void read() = 0;
    virtual void write() = 0;

};

#endif // NETWORKTRANSLATOR_H
