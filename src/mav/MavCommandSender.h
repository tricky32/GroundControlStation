#pragma once
#include <QObject>
#include <QByteArray>
#include <QHash>

extern "C" {
#include <common/mavlink.h>
}

class MavRouter;

struct PendingCmd {
    QByteArray bytes;
    int        sysid = 0;
    int        command = 0;
    int        retries = 0;
};

class MavCommandSender : public QObject {
    Q_OBJECT
public:
    explicit MavCommandSender(MavRouter* router, QObject* parent = nullptr);

public slots:
    void arm(int sysid, bool force = false);
    void disarm(int sysid);
    void takeoff(int sysid, float alt);
    void land(int sysid);
    void changeAlt(int sysid, float alt);
    void setMode(int sysid, uint8_t baseMode, uint32_t customMode);
    void rtl(int sysid);
    void gotoLatLonAlt(int sysid, double lat, double lon, double alt);

signals:
    void commandSent(int sysid, int command);

private slots:
    void onMavMessage(const mavlink_message_t& msg);

private:
    void sendCommandLong_(int sysid, int command,
                          float p1, float p2, float p3, float p4,
                          float p5, float p6, float p7,
                          int targetComp);
    void scheduleRetry_(quint64 key);

    MavRouter* router_ = nullptr;
    QHash<quint64, PendingCmd> pending_; // key = (sysid<<32) | command
};
