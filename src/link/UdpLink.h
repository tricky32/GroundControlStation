#pragma once
#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include "../mav/MavlinkCodec.h"

class UdpLink : public QObject {
    Q_OBJECT
public:
    explicit UdpLink(QObject* p=nullptr);

    bool bind(quint16 port);
    void close();

    void send(const QHostAddress& addr, quint16 port, const QByteArray& bytes);

signals:
    void datagramReceived(const QByteArray& bytes, QHostAddress from, quint16 port);

private slots:
    void onReadyRead();

private:
    QUdpSocket m_sock;
    quint16 m_boundPort{0};
};
