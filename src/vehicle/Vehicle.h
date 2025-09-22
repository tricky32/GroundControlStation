#pragma once
#include <QObject>
#include <QString>
#include <QGeoCoordinate>

class Vehicle : public QObject {
    Q_OBJECT
    Q_PROPERTY(int sysId READ sysId CONSTANT)
    Q_PROPERTY(QString status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(QString mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(QGeoCoordinate position READ position WRITE setPosition NOTIFY positionChanged)
    Q_PROPERTY(double relAlt READ relAlt WRITE setRelAlt NOTIFY relAltChanged)
    Q_PROPERTY(double amslAlt READ amslAlt WRITE setAmslAlt NOTIFY amslAltChanged)
    Q_PROPERTY(double groundSpeed READ groundSpeed WRITE setGroundSpeed NOTIFY groundSpeedChanged)
    Q_PROPERTY(double airSpeed READ airSpeed WRITE setAirSpeed NOTIFY airSpeedChanged)
    Q_PROPERTY(double heading READ heading WRITE setHeading NOTIFY headingChanged)
    Q_PROPERTY(int batteryPct READ batteryPct WRITE setBatteryPct NOTIFY batteryPctChanged)
    Q_PROPERTY(QGeoCoordinate home READ home WRITE setHome NOTIFY homeChanged)
public:
    explicit Vehicle(int sysid, QObject* parent=nullptr) : QObject(parent), sysId_(sysid) {}

    int sysId() const { return sysId_; }

    const QString& status() const { return status_; }
    void setStatus(const QString& s) { if (status_==s) return; status_=s; emit statusChanged(); }

    const QString& mode() const { return mode_; }
    void setMode(const QString& m) { if (mode_==m) return; mode_=m; emit modeChanged(); }

    QGeoCoordinate position() const { return pos_; }
    void setPosition(const QGeoCoordinate& c) { if (pos_==c) return; pos_=c; emit positionChanged(); }

    double relAlt() const { return relAlt_; }
    void setRelAlt(double v) { if (relAlt_==v) return; relAlt_=v; emit relAltChanged(); }

    double amslAlt() const { return amslAlt_; }
    void setAmslAlt(double v) { if (amslAlt_==v) return; amslAlt_=v; emit amslAltChanged(); }

    double groundSpeed() const { return gs_; }
    void setGroundSpeed(double v) { if (gs_==v) return; gs_=v; emit groundSpeedChanged(); }

    double airSpeed() const { return as_; }
    void setAirSpeed(double v) { if (as_==v) return; as_=v; emit airSpeedChanged(); }

    double heading() const { return hdg_; }
    void setHeading(double v) { if (hdg_==v) return; hdg_=v; emit headingChanged(); }

    int batteryPct() const { return batt_; }
    void setBatteryPct(int v) { if (batt_==v) return; batt_=v; emit batteryPctChanged(); }

    QGeoCoordinate home() const { return home_; }
    void setHome(const QGeoCoordinate& h) { if (home_==h) return; home_=h; emit homeChanged(); }

signals:
    void statusChanged();
    void modeChanged();
    void positionChanged();
    void relAltChanged();
    void amslAltChanged();
    void groundSpeedChanged();
    void airSpeedChanged();
    void headingChanged();
    void batteryPctChanged();
    void homeChanged();

private:
    int sysId_ = -1;
    QString status_ = "disarmed";
    QString mode_ = "--";
    QGeoCoordinate pos_;
    double relAlt_ = 0.0;
    double amslAlt_ = 0.0;
    double gs_ = 0.0;
    double as_ = 0.0;
    double hdg_ = 0.0;
    int batt_ = -1;
    QGeoCoordinate home_;
};
