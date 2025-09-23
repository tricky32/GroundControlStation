#include "UdpLink.h"
#include <QNetworkDatagram>

UdpLink::UdpLink(QObject* parent): QObject(parent), m_sock(new QUdpSocket(this)) {
    connect(m_sock, &QUdpSocket::readyRead, this, &UdpLink::onReadyRead);
}

UdpLink::~UdpLink(){ close(); }

bool UdpLink::bind(quint16 port){
    if(m_sock->state() != QAbstractSocket::UnconnectedState) close();
    m_port = port;
    if(!m_sock->bind(QHostAddress::AnyIPv4, port, QUdpSocket::ShareAddress|QUdpSocket::ReuseAddressHint)){
        emit errorText(QString("UDP bind(%1) failed: %2").arg(port).arg(m_sock->errorString()));
        return false;
    }
    return true;
}

void UdpLink::close(){
    if(m_sock) m_sock->close();
}

void UdpLink::send(const QHostAddress& addr, quint16 port, const QByteArray& bytes){
    QMutexLocker lk(&m_sendMutex);
    m_sock->writeDatagram(bytes, addr, port);
}

void UdpLink::onReadyRead(){
    while(m_sock->hasPendingDatagrams()){
        QNetworkDatagram d = m_sock->receiveDatagram();
        emit datagramReceived(d.data(), d.senderAddress(), d.senderPort());
    }
}
