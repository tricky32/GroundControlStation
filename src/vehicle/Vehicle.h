#pragma once
#include <QObject>
#include <QElapsedTimer>
#include <QDateTime>
#include <QTimer>
#include <QtNetwork/QHostAddress>
#include "../mav/MavlinkCodec.h"

struct GeoPoint {
    Q_GADGET
    Q_PROPERTY(double latitude MEMBER latitude)
    Q_PROPERTY(double longitude MEMBER longitude)
    Q_PROPERTY(double altitude MEMBER altitude)
    Q_PROPERTY(bool   isValid  MEMBER isValid)
public:
    double latitude{0};
    double longitude{0};
    double altitude{0};
    bool   isValid{false};
};

class Vehicle : public QObject {
    Q_OBJECT
    Q_PROPERTY(int sysId READ sysId CONSTANT)
    Q_PROPERTY(QString status READ status NOTIFY telemetryChanged)
    Q_PROPERTY(uint32_t mode READ mode NOTIFY telemetryChanged)
    Q_PROPERTY(GeoPoint position READ position NOTIFY telemetryChanged)
    Q_PROPERTY(GeoPoint home READ home NOTIFY telemetryChanged)
    Q_PROPERTY(GeoPoint gotoPoint READ gotoPoint NOTIFY telemetryChanged)
    Q_PROPERTY(double altitudeAMSL READ altitudeAMSL NOTIFY telemetryChanged)
    Q_PROPERTY(double groundSpeed READ groundSpeed NOTIFY telemetryChanged)
    Q_PROPERTY(double airSpeed READ airSpeed NOTIFY telemetryChanged)
    Q_PROPERTY(double heading READ heading NOTIFY telemetryChanged)
    Q_PROPERTY(int batteryPct READ batteryPct NOTIFY telemetryChanged)
    Q_PROPERTY(bool armed READ armed NOTIFY telemetryChanged)
    Q_PROPERTY(bool inAir READ inAir NOTIFY telemetryChanged)

public:
    Vehicle(int sysId, const Endpoint& ep, QObject* parent=nullptr);

    int sysId() const { return m_sysId; }
    QString status() const { return m_status; }
    uint32_t mode() const { return m_mode; }
    GeoPoint position() const { return m_position; }
    GeoPoint home() const { return m_home; }
    GeoPoint gotoPoint() const { return m_goto; }
    double altitudeAMSL() const { return m_altAMSL; }
    double groundSpeed() const { return m_groundSpeed; }
    double airSpeed() const { return m_airSpeed; }
    double heading() const { return m_heading; }
    int batteryPct() const { return m_batteryPct; }
    bool armed() const { return m_armed; }
    bool inAir() const { return m_inAir; }

    void setEndpoint(const Endpoint& ep) { m_ep = ep; }
    qint64 msSinceHeartbeat() const { return m_lastHeartbeat.elapsed(); }

signals:
    void telemetryChanged();
    void sendBytes(const Endpoint& ep, const QByteArray& bytes);

public slots:
    void handleMsg(const mavlink_message_t& msg);
    void requestStreams();
    void arm(bool arm);
    void takeoff(double altRel);
    void land();
    void rtl();
    void changeAltitude(double altRel);
    void setModeText(const QString& mode);
    void gotoLatLonAlt(double lat, double lon, double altRel);

private:
    void updateStatus();
    void sendMessage(const mavlink_message_t& m);
    void sendCommandLong(uint16_t cmd, float p1=0,float p2=0,float p3=0,float p4=0,float p5=0,float p6=0,float p7=0);

private:
    int m_sysId{0};
    Endpoint m_ep;

    // state
    QString  m_status{"disarmed"};
    uint32_t m_mode{0};
    bool     m_armed{false};
    bool     m_inAir{false};

    GeoPoint m_position;
    GeoPoint m_home;
    GeoPoint m_goto;
    double   m_altAMSL{0};
    double   m_groundSpeed{0};
    double   m_airSpeed{0};
    double   m_heading{0};
    int      m_batteryPct{100};

    QElapsedTimer m_lastHeartbeat;
};
