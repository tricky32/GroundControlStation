#include "UdpLink.h"
#include <QNetworkDatagram>

UdpLink::UdpLink(QObject* p) : QObject(p) {
    connect(&m_sock, &QUdpSocket::readyRead, this, &UdpLink::onReadyRead);
}

bool UdpLink::bind(quint16 port) {
    close();
    if (!m_sock.bind(QHostAddress::AnyIPv4, port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
        return false;
    m_boundPort = port;
    return true;
}

void UdpLink::close() {
    if (m_sock.state() != QAbstractSocket::UnconnectedState) {
        m_sock.close();
    }
    m_boundPort = 0;
}

void UdpLink::send(const QHostAddress& addr, quint16 port, const QByteArray& bytes) {
    if (addr.isNull() || port == 0) return;
    m_sock.writeDatagram(bytes, addr, port);
}

void UdpLink::onReadyRead() {
    while (m_sock.hasPendingDatagrams()) {
        QNetworkDatagram d = m_sock.receiveDatagram();
        emit datagramReceived(d.data(), d.senderAddress(), d.senderPort());
    }
}
