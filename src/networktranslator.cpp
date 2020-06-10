#include "networktranslator.h"

void NetworkTranslator::returnState(QAbstractSocket::SocketState state)
{
    switch(state)
    {
        case QAbstractSocket::ClosingState:     qDebug() << "Socket on a closing state";     break;
        case QAbstractSocket::ConnectedState:   qDebug() << "Socket on a connected state";   break;
        case QAbstractSocket::ConnectingState:  qDebug() << "Socket on a connecting state";  break;
        case QAbstractSocket::UnconnectedState: qDebug() << "Socket on a unconnected state"; break;
        case QAbstractSocket::BoundState:       qDebug() << "Socket on a bound state";       break;
        case QAbstractSocket::ListeningState:   qDebug() << "Socket on a listening state";   break;
        case QAbstractSocket::HostLookupState:  qDebug() << "Socket on a host lookup state"; break;
    }
}

NetworkTranslator::~NetworkTranslator()
{

}
