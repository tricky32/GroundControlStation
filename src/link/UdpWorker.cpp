#include "UdpWorker.h"
#include <QNetworkDatagram>

UdpWorker::UdpWorker(QObject* p): QObject(p) {}

void UdpWorker::start(quint16 listenPort) {
    if (socket_) return;
    socket_ = new QUdpSocket(this);
    socket_->bind(QHostAddress::AnyIPv4, listenPort, QUdpSocket::ShareAddress);
    connect(socket_, &QUdpSocket::readyRead, this, &UdpWorker::onReadyRead_);
    emit started();
}

void UdpWorker::stop() {
    if (!socket_) return;
    socket_->close();
    socket_->deleteLater();
    socket_ = nullptr;
    emit stopped();
}

void UdpWorker::sendPacket(const QByteArray& bytes, const QHostAddress& host, quint16 port) {
    if (!socket_) return;
    socket_->writeDatagram(bytes, host, port);
}

void UdpWorker::onReadyRead_() {
    while (socket_ && socket_->hasPendingDatagrams()) {
        QNetworkDatagram d = socket_->receiveDatagram();
        emit packetReceived(d.data(), d.senderAddress(), d.senderPort());
    }
}
