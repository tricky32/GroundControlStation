#pragma once
#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>

class UdpWorker : public QObject {
    Q_OBJECT
public:
    explicit UdpWorker(QObject* p = nullptr);

public slots:
    void start(quint16 listenPort);
    void stop();
    void sendPacket(const QByteArray& bytes, const QHostAddress& host, quint16 port);

private slots:
    void onReadyRead_();

signals:
    void packetReceived(const QByteArray& data, const QHostAddress& addr, quint16 port);
    void started();
    void stopped();

private:
    QUdpSocket* socket_ = nullptr;
};
