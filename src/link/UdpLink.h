#pragma once
#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QMutex>

class UdpLink : public QObject {
    Q_OBJECT
public:
    explicit UdpLink(QObject* parent=nullptr);
    ~UdpLink();

    Q_INVOKABLE bool bind(quint16 port);
    Q_INVOKABLE void close();
    Q_INVOKABLE quint16 boundPort() const { return m_port; }

    Q_INVOKABLE void send(const QHostAddress& addr, quint16 port, const QByteArray& bytes);

signals:
    void datagramReceived(const QByteArray& bytes, QHostAddress sender, quint16 senderPort);
    void errorText(const QString& msg);

private slots:
    void onReadyRead();

private:
    QUdpSocket* m_sock=nullptr;
    quint16 m_port=0;
    QMutex m_sendMutex;
};
