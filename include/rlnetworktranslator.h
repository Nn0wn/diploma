#ifndef RlNetworkTranslator_H
#define RlNetworkTranslator_H

#include "networktranslator.h"
#include "QTcpSocket"
#include "QNetworkDatagram"

#define TEST_CONNECTING_TIMER 100
#define CONNECTING_TIMER 30000
#define CONNECTING_DELAY 500
#define CONNECTING_INTERVAL 50
#define WRITING_TIMER 30000

class RlNetworkTranslator : public NetworkTranslator
{
    Q_OBJECT

    QBasicTimer *m_errorTimer;
    QBasicTimer *m_tryTimer;
    QMessageBox *m_box;

    bool setupListener(quint16 listener_port, quint16 server_port);
    bool setupConnector(const QString& hostName, quint16 connecting_port,
                        QAbstractSocket::NetworkLayerProtocol protocol);
    void timerEvent(QTimerEvent* e) override;

public:
    explicit RlNetworkTranslator(QObject *parent = nullptr, int mode = MessageMode::NORMAL,
                                 const QString &hostName = QHostAddress(QHostAddress::LocalHost).toString(),
                                 quint16 listener_port = STANDART_LISTENING_PORT, quint16 connecting_port = STANDART_CONNECTION_PORT,
                                 QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::IPv4Protocol);
    ~RlNetworkTranslator() override;
    quint16 getIport() override;
    quint16 getOport() override;
    QByteArray *getInputData() override;
    QByteArray *getOutputData() override;
    QString getMsg() override;
    int getMode() override;

    void setMode(int mode) override;
    void encodeData(QVariant source);
    template<typename T> T decodeData(QVariant data);

signals:
    void newDataToSend();
    void connectionEstablished();
    void connectedToServer();
    void disconnectedFromServer();
    void socketBound();
    void socketClosed();
    void sendListeningPort(quint16);

private slots:
    virtual void checkConnection() override;
    virtual void checkUdpSocket(QAbstractSocket::SocketState state) override;
    virtual void checkTcpSocket(QAbstractSocket::SocketState state) override;

public slots:
    bool setupConnection(const QString &hostName, quint16 listening_port, quint16 connecting_port,
                         QAbstractSocket::NetworkLayerProtocol protocol, quint16 server_port = NO_PORT) override;
    void disconnect() override;
    virtual void read() override;
    virtual void write() override;


    //void udpStateChanged();
};

#endif // RlNetworkTranslator_H
