#pragma once
#include <QObject>
#include <QVariant>
#include <QHash>
#include <QSharedPointer>

class Vehicle;
struct MavMsg;

class MultiVehicleManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList vehicles READ vehicles NOTIFY vehiclesChanged)
public:
    explicit MultiVehicleManager(QObject* parent = nullptr);

    QVariantList vehicles() const;
    Q_INVOKABLE QObject* vehicleBySysId(int sysid) const;
    Vehicle* ensureVehicle(int sysid);

signals:
    void vehiclesChanged();

public slots:
    void onMavMessage(const MavMsg& m);

private:
    QHash<int, QSharedPointer<Vehicle>> map_;
};
