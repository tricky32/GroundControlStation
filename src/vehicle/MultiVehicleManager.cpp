#include "MultiVehicleManager.h"
#include <QMetaType>

MultiVehicleManager::MultiVehicleManager(QObject* p)
    : QObject(p)
{
    // Runtime registration is enough for queued connections
    qRegisterMetaType<mavlink_message_t>("mavlink_message_t");
    qRegisterMetaType<Endpoint>("Endpoint");

    connect(&m_codec, &MavlinkCodec::messageReceived,
            this, &MultiVehicleManager::onMavlinkMessage);

    // Prune vehicles with no heartbeat for >10s
    m_prune.setInterval(1000);
    connect(&m_prune, &QTimer::timeout, this, [this]{
        bool changed = false;
        for (auto it = m_bySys.begin(); it != m_bySys.end(); ) {
            Vehicle* v = it.value();
            if (v->msSinceHeartbeat() > 10000) {
                it = m_bySys.erase(it);
                v->deleteLater();
                changed = true;
            } else {
                ++it;
            }
        }
        if (changed) emit vehiclesChanged();
    });
    m_prune.start();
}

QVariantList MultiVehicleManager::vehicles() const {
    QVariantList list;
    list.reserve(m_bySys.size());
    for (auto* v : m_bySys)
        list << QVariant::fromValue(v);  // QVariant holds QObject*
    return list;
}

void MultiVehicleManager::onLinkBytes(const QByteArray& bytes, QHostAddress from, quint16 port) {
    m_codec.feed(bytes, Endpoint{from, port});
}

void MultiVehicleManager::onMavlinkMessage(mavlink_message_t msg, Endpoint ep) {
    const int sys = msg.sysid;

    auto it = m_bySys.find(sys);
    if (it == m_bySys.end()) {
        auto* v = new Vehicle(sys, ep, this);
        connect(v, &Vehicle::sendBytes, this, &MultiVehicleManager::sendBytes);
        m_bySys.insert(sys, v);
        emit vehiclesChanged();
        v->requestStreams();
    } else {
        it.value()->setEndpoint(ep);
    }

    m_bySys[sys]->handleMsg(msg);
}
