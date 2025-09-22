#include "UdpLink.h"
#include "UdpWorker.h"
#include <QMetaObject>

UdpLink::UdpLink(quint16 port, QObject* parent)
    : QObject(parent), port_(port) {
    worker_ = new UdpWorker();
    worker_->moveToThread(&thread_);
    connect(&thread_, &QThread::finished, worker_, &QObject::deleteLater);
    connect(this, &UdpLink::sendBytesInternal, worker_, &UdpWorker::sendPacket);
    connect(worker_, &UdpWorker::packetReceived, this, &UdpLink::bytesReceived);
    thread_.start();
    QMetaObject::invokeMethod(worker_, "start", Qt::QueuedConnection, Q_ARG(quint16, port_));
    emit listenPortChanged(port_);
}

UdpLink::~UdpLink() {
    if (worker_) QMetaObject::invokeMethod(worker_, "stop", Qt::BlockingQueuedConnection);
    thread_.quit();
    thread_.wait();
}

void UdpLink::setListenPort(quint16 p) {
    if (p == port_) return;
    if (worker_) QMetaObject::invokeMethod(worker_, "stop", Qt::BlockingQueuedConnection);
    port_ = p;
    if (worker_) QMetaObject::invokeMethod(worker_, "start", Qt::QueuedConnection, Q_ARG(quint16, port_));
    emit listenPortChanged(port_);
}

void UdpLink::sendBytes(const QByteArray& data, const QHostAddress& addr, quint16 port) {
    emit sendBytesInternal(data, addr, port);
}
