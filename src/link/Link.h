#pragma once
#include <QObject>
#include <QByteArray>
#include "../mav/MavlinkCodec.h"


class Link : public QObject {
    Q_OBJECT
public:
    explicit Link(QObject* parent=nullptr) : QObject(parent) {}

    void setEndpoint(const Endpoint& ep) { m_ep = ep; }
    const Endpoint& endpoint() const { return m_ep; }

signals:
    void outgoing(const Endpoint& ep, const QByteArray& payload);

public slots:
    void send(const QByteArray& payload) { emit outgoing(m_ep, payload); }

private:
    Endpoint m_ep;
};
