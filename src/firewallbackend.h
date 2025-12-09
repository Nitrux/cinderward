#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDBusInterface>

class FirewallBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString defaultZone READ defaultZone NOTIFY defaultZoneChanged)
    Q_PROPERTY(QStringList services READ services NOTIFY servicesChanged)
    Q_PROPERTY(QStringList ports READ ports NOTIFY portsChanged)
    Q_PROPERTY(QStringList sources READ sources NOTIFY sourcesChanged)
    Q_PROPERTY(QStringList forwardRules READ forwardRules NOTIFY forwardRulesChanged)
    Q_PROPERTY(bool masquerade READ masquerade NOTIFY masqueradeChanged)
    Q_PROPERTY(bool logDenied READ logDenied NOTIFY logDeniedChanged)
    Q_PROPERTY(bool panic READ panic NOTIFY panicChanged)
    Q_PROPERTY(QStringList knownServices READ knownServices NOTIFY knownServicesChanged)
    Q_PROPERTY(bool stealthMode READ stealthMode NOTIFY stealthModeChanged)
    Q_PROPERTY(bool strictIcmp READ strictIcmp NOTIFY strictIcmpChanged)

public:
    explicit FirewallBackend(QObject *parent = nullptr);

    QString state() const;
    QString defaultZone() const;
    QStringList services() const;
    QStringList ports() const;
    QStringList sources() const;
    QStringList forwardRules() const;
    QStringList knownServices() const;
    bool masquerade() const;
    bool logDenied() const;
    bool panic() const;
    bool stealthMode() const;
    bool strictIcmp() const;

    Q_INVOKABLE void refresh(const QString &zone);
    Q_INVOKABLE void addService(const QString &service, const QString &zone);
    Q_INVOKABLE void removeService(const QString &service, const QString &zone);
    Q_INVOKABLE void addPort(const QString &port, const QString &protocol, const QString &zone);
    Q_INVOKABLE void removePort(const QString &port, const QString &protocol, const QString &zone);
    Q_INVOKABLE void addSource(const QString &source, const QString &zone);
    Q_INVOKABLE void removeSource(const QString &source, const QString &zone);
    Q_INVOKABLE void reload();
    Q_INVOKABLE void addForwardRule(const QString &sourcePort, const QString &protocol, const QString &destPort, const QString &destIP, const QString &zone);
    Q_INVOKABLE void removeForwardRule(const QString &sourcePort, const QString &protocol, const QString &destPort, const QString &destIP, const QString &zone);
    Q_INVOKABLE void changeDefaultZone(const QString &zone);
    Q_INVOKABLE void setMasquerade(bool enabled, const QString &zone);
    Q_INVOKABLE void setLogDenied(bool enabled);
    Q_INVOKABLE void setPanic(bool enabled);
    Q_INVOKABLE void setStealthMode(bool enabled, const QString &zone);
    Q_INVOKABLE void setStrictIcmp(bool enabled, const QString &zone);

signals:
    void stateChanged();
    void defaultZoneChanged();
    void servicesChanged();
    void operationOutput(const QString &output);
    void operationError(const QString &error);
    void panicChanged();
    void portsChanged();
    void sourcesChanged();
    void forwardRulesChanged();
    void masqueradeChanged();
    void logDeniedChanged();
    void knownServicesChanged();
    void stealthModeChanged();
    void strictIcmpChanged();

private:
    QString getPermanentZonePath(const QString &zoneName);

    void setState(const QString &s);
    void setDefaultZone(const QString &z);
    void setServices(const QStringList &s);
    void setPanicState(bool enabled);
    void setPorts(const QStringList &p);
    void setSources(const QStringList &s);
    void setForwardRules(const QStringList &r);
    void setMasqueradeState(bool enabled);
    void setLogDeniedState(bool enabled);
    void setStealthModeState(bool enabled);
    void setStrictIcmpState(bool enabled);

    QString m_state;
    QString m_defaultZone;
    QStringList m_services;
    QStringList m_ports;
    QStringList m_sources;
    QStringList m_forwardRules;
    QStringList m_knownServices;
    bool m_panic = false;
    bool m_masquerade = false;
    bool m_logDenied = false;
    bool m_stealthMode = false;
    bool m_strictIcmp = false;
};
