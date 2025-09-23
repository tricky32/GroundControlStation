#pragma once
#include <QObject>
#include <QVariant>
#include <QHash>
#include <QTimer>
#include "../mav/MavlinkCodec.h"
#include "Vehicle.h"

class MultiVehicleManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList vehicles READ vehicles NOTIFY vehiclesChanged)
public:
    explicit MultiVehicleManager(QObject* p=nullptr);

    QVariantList vehicles() const;

signals:
    void vehiclesChanged();
    void sendBytes(const Endpoint& ep, const QByteArray& bytes);

public slots:
    void onMavlinkMessage(mavlink_message_t msg, Endpoint ep);
    void onLinkBytes(const QByteArray& bytes, QHostAddress from, quint16 port);

private:
    MavlinkCodec m_codec;
    QHash<int, Vehicle*> m_bySys;   // sysid -> vehicle
    QTimer m_prune;                  // prune stale vehicles
};
