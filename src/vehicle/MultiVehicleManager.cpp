#include "MultiVehicleManager.h"
#include <QMetaType>
#include <algorithm>

MultiVehicleManager::MultiVehicleManager(QObject* p)
    : QObject(p)
{
    qRegisterMetaType<mavlink_message_t>("mavlink_message_t");
    qRegisterMetaType<Endpoint>("Endpoint");

    connect(&m_codec, &MavlinkCodec::messageReceived,
            this, &MultiVehicleManager::onMavlinkMessage);

    // per-vehicle link wrappers forward to our sendBytes (QmlGlobals then to UdpLink)
    // created on demand in onMavlinkMessage

    // prune vehicles with no heartbeat for >10s
    m_prune.setInterval(1000);
    connect(&m_prune, &QTimer::timeout, this, [this]{
        bool changed = false;
        for (auto it = m_bySys.begin(); it != m_bySys.end(); ) {
            Vehicle* v = it.value();
            if (v->msSinceHeartbeat() > 10000) {
                const int sys = it.key();
                it = m_bySys.erase(it);
                if (m_links.contains(sys)) {
                    m_links[sys]->deleteLater();
                    m_links.remove(sys);
                }
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
    QList<int> keys = m_bySys.keys();
    std::sort(keys.begin(), keys.end());
    list.reserve(keys.size());
    for (int k : keys)
        list << QVariant::fromValue(m_bySys[k]);
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
        auto* link = new VehicleLink(this);
        connect(v,    &Vehicle::sendBytes, link, &VehicleLink::forwardFromVehicle);
        connect(link, &VehicleLink::linkBytes, this, &MultiVehicleManager::sendBytes);

        m_bySys.insert(sys, v);
        m_links.insert(sys, link);
        emit vehiclesChanged();
        v->requestStreams();
    } else {
        Vehicle* v = it.value();
        const bool epDiffers = (v->endpoint().addr != ep.addr) || (v->endpoint().port != ep.port);
        if (epDiffers) {
            if (v->msSinceHeartbeat() > 2000) {
                v->setEndpoint(ep);
            } else {
                // duplicate SYSID on another endpoint -> ignore switch to prevent flapping
            }
        }
    }

    m_bySys[sys]->handleMsg(msg);
}
