#pragma once
#include <QObject>
#include <QThread>
#include <QHostAddress>

class UdpWorker;

class UdpLink : public QObject {
    Q_OBJECT
    Q_PROPERTY(quint16 listenPort READ listenPort WRITE setListenPort NOTIFY listenPortChanged)
public:
    explicit UdpLink(quint16 port, QObject* parent=nullptr);
    ~UdpLink() override;

    quint16 listenPort() const { return port_; }
    void setListenPort(quint16 p);

signals:
    void bytesReceived(const QByteArray& data, const QHostAddress& addr, quint16 port);
    void listenPortChanged(quint16);
    void sendBytesInternal(const QByteArray& data, const QHostAddress& addr, quint16 port);

public slots:
    void sendBytes(const QByteArray& data, const QHostAddress& addr, quint16 port);

private:
    quint16 port_;
    QThread thread_;
    UdpWorker* worker_ = nullptr;
};
