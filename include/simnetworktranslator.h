#ifndef SIMNETWORKTRANSLATOR_H
#define SIMNETWORKTRANSLATOR_H

#include <QTcpServer>
#include <QInputDialog>
#include <QSettings>
#include <QThread>
#include "networktranslator.h"
#include "SimMessageDialog.h"
#include "StandProto.h"
#include "FltToStr.h"
#include <cmath>

#define STANDART_SERVER_LISTENING_PORT 6001
#define TEST_SERVER_LISTENING_PORT 601
#define STANDART_SIMULATOR_CONNECTION_PORT 6000
#define LOWEST_PORT_VALUE 1024
#define HIGHEST_PORT_VALUE 65535

enum MsgDialogType{
    QMsgBox,
    SimMsgDlg
};

class SimNetworkTranslator : public NetworkTranslator
{
    Q_OBJECT

    QTcpServer          *m_server;
    StandProto          *m_proto;
    QSettings           *m_factoryLimits;
    QString             m_oMsg;

public:
    explicit SimNetworkTranslator(QObject *parent = nullptr, int mode = MessageMode::NORMAL,
                                  const QString &hostName = QHostAddress(QHostAddress::LocalHost).toString(),
                                  quint16 listening_port = STANDART_SERVER_LISTENING_PORT,
                                  quint16 connecting_port = STANDART_SIMULATOR_CONNECTION_PORT,
                                  QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::IPv4Protocol);
    ~SimNetworkTranslator() override;
    quint16 getIport() override;
    quint16 getOport() override;
    QByteArray *getInputData() override;
    QByteArray *getOutputData() override;
    QString getMsg() override;
    int getMode() override;
    QString getOutputMsg();
    StandProto *getProto();

    void setMode(int mode) override;
    void setProto(StandProto* proto);
    void setSettings(QString filename, QSettings::Format format = QSettings::IniFormat);


    QTcpServer* getServer();
    QSettings *getSettings();

    void encodeData(QVariant source);
    template<typename T> T decodeData(QVariant data);

private:
    void doProtoAnswer();

signals:
    void newDataToSend();
    void connectionEstablished();
    void connectedToClt();
    void disconnectedFromClt();
    void cltConnected();
    void cltDisconnected();
    void valueSet();

    void sendMsgDialog(std::tuple<MsgDialogType, int, QString, QDialogButtonBox::StandardButtons>);
    void sendServerCode(int);

protected slots:
    virtual void checkConnection() override;
    virtual void checkUdpSocket(QAbstractSocket::SocketState state) override;
    virtual void checkTcpSocket(QAbstractSocket::SocketState state) override;

public slots:
    bool setupConnection(const QString &hostName = NO_ADDRESS, quint16 listening_port = NO_PORT, quint16 connecting_port = NO_PORT,
                         QAbstractSocket::NetworkLayerProtocol protocol = QAbstractSocket::IPv4Protocol,
                         quint16 server_port = NO_PORT) override;
    void disconnect() override;
    virtual void read() override;
    virtual void write() override;
    void setConnectingPort(quint16 port);
    bool setupServer(QHostAddress server_address, quint16 server_port);
    void stopServer();

    void connectSocket();
};

#endif // SIMNETWORKTRANSLATOR_H
