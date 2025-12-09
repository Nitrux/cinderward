#include "firewallbackend.h"
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusArgument>
#include <QDebug>

// Constants for Firewalld D-Bus

const QString FW_SERVICE = "org.fedoraproject.FirewallD1";
const QString FW_PATH = "/org/fedoraproject/FirewallD1";
const QString FW_INTERFACE = "org.fedoraproject.FirewallD1";
const QString FW_ZONE_INTERFACE = "org.fedoraproject.FirewallD1.zone";

// Permanent Config Constants

const QString FW_CONFIG_PATH = "/org/fedoraproject/FirewallD1/config";
const QString FW_CONFIG_INTERFACE = "org.fedoraproject.FirewallD1.config";
const QString FW_CONFIG_ZONE_INTERFACE = "org.fedoraproject.FirewallD1.config.zone";

FirewallBackend::FirewallBackend(QObject *parent)
    : QObject(parent)
{
    QDBusConnection bus = QDBusConnection::systemBus();
    if (bus.isConnected()) {
        QDBusInterface fw(FW_SERVICE, FW_PATH, FW_INTERFACE, bus);
        
        QDBusReply<QStringList> reply = fw.call("listServices");
        
        if (reply.isValid()) {
            m_knownServices = reply.value();
            m_knownServices.sort(); 
        } else {
            qWarning() << "Failed to load services:" << reply.error().message();
        }
    }
}

// === HELPER: Get Permanent Zone Path ===

QString FirewallBackend::getPermanentZonePath(const QString &zoneName)
{
    // 1. Connect to the Config interface
    QDBusInterface config(FW_SERVICE, FW_CONFIG_PATH, FW_CONFIG_INTERFACE, QDBusConnection::systemBus());
    
    // 2. Ask for the object path of the specific zone by name
    QDBusReply<QDBusObjectPath> reply = config.call("getZoneByName", zoneName);
    if (reply.isValid()) {
        return reply.value().path();
    }
    return QString();
}

// === GETTERS ===

QString FirewallBackend::state() const { return m_state; }
QString FirewallBackend::defaultZone() const { return m_defaultZone; }
QStringList FirewallBackend::services() const { return m_services; }
QStringList FirewallBackend::ports() const { return m_ports; }
QStringList FirewallBackend::forwardRules() const { return m_forwardRules; }
QStringList FirewallBackend::knownServices() const { return m_knownServices; }
QStringList FirewallBackend::sources() const { return m_sources; }
bool FirewallBackend::masquerade() const { return m_masquerade; }
bool FirewallBackend::logDenied() const { return m_logDenied; }
bool FirewallBackend::panic() const { return m_panic; }
bool FirewallBackend::stealthMode() const { return m_stealthMode; }
bool FirewallBackend::strictIcmp() const { return m_strictIcmp; }

// === SETTERS (Internal) ===
void FirewallBackend::setState(const QString &s) { if (m_state != s) { m_state = s; emit stateChanged(); } }
void FirewallBackend::setDefaultZone(const QString &z) { if (m_defaultZone != z) { m_defaultZone = z; emit defaultZoneChanged(); } }
void FirewallBackend::setServices(const QStringList &s) { if (m_services != s) { m_services = s; emit servicesChanged(); } }
void FirewallBackend::setPorts(const QStringList &p) { if (m_ports != p) { m_ports = p; emit portsChanged(); } }
void FirewallBackend::setForwardRules(const QStringList &r) { if (m_forwardRules != r) { m_forwardRules = r; emit forwardRulesChanged(); } }
void FirewallBackend::setMasqueradeState(bool enabled) { if (m_masquerade != enabled) { m_masquerade = enabled; emit masqueradeChanged(); } }
void FirewallBackend::setLogDeniedState(bool enabled) { if (m_logDenied != enabled) { m_logDenied = enabled; emit logDeniedChanged(); } }
void FirewallBackend::setPanicState(bool enabled) { if (m_panic != enabled) { m_panic = enabled; emit panicChanged(); } }
void FirewallBackend::setStealthModeState(bool enabled) { if (m_stealthMode != enabled) { m_stealthMode = enabled; emit stealthModeChanged(); } }
void FirewallBackend::setStrictIcmpState(bool enabled) { if (m_strictIcmp != enabled) { m_strictIcmp = enabled; emit strictIcmpChanged(); } }
void FirewallBackend::setSources(const QStringList &s) { if (m_sources != s) { m_sources = s; emit sourcesChanged(); } }

// === REFRESH LOGIC (Permanent-based) ===

void FirewallBackend::refresh(const QString &zone)
{
    QDBusConnection bus = QDBusConnection::systemBus();
    if (!bus.isConnected()) {
        emit operationError("Cannot connect to system bus");
        setState("error");
        return;
    }

    QDBusInterface fw(FW_SERVICE, FW_PATH, FW_INTERFACE, bus);

    // 1. Get State & Panic
    setState("running");

    QDBusReply<bool> panicReply = fw.call("queryPanicMode");
    if (panicReply.isValid()) setPanicState(panicReply.value());

    // 2. Determine the Zone to Query
    QDBusReply<QString> defZoneReply = fw.call("getDefaultZone");
    QString actualZone = zone;
    
    // If we passed an empty string (from startup), use the system default
    if (defZoneReply.isValid()) {
        setDefaultZone(defZoneReply.value());
        if (actualZone.isEmpty()) actualZone = defZoneReply.value(); 
    } else {
        if (actualZone.isEmpty()) actualZone = m_defaultZone;
    }

    // A. Permanent Path (REQUIRED for Services/Ports)
    QString permanentZonePath = getPermanentZonePath(actualZone);
    if (permanentZonePath.isEmpty()) {
        setServices({}); setPorts({}); setForwardRules({});
        return;
    }
    QDBusInterface permanentZoneIf(FW_SERVICE, permanentZonePath, FW_CONFIG_ZONE_INTERFACE, bus);

    // 4. Services (Always from Permanent)
    QDBusReply<QStringList> servicesReply = permanentZoneIf.call("getServices");
    if (servicesReply.isValid()) setServices(servicesReply.value());
    else setServices({});

    // 5. Ports (Always from Permanent)
    QDBusMessage portsMsg = permanentZoneIf.call("getPorts");
    if (portsMsg.type() == QDBusMessage::ReplyMessage) {
        const QDBusArgument &arg = portsMsg.arguments().at(0).value<QDBusArgument>();
        QStringList portList;
        arg.beginArray();
        while (!arg.atEnd()) {
            QString port, proto;
            arg.beginStructure();
            arg >> port >> proto;
            arg.endStructure();
            portList.append(port + "/" + proto);
        }
        arg.endArray();
        setPorts(portList);
    } else {
        setPorts({});
    }

    // 6. Sources

    QDBusReply<QStringList> sourcesReply = permanentZoneIf.call("getSources");
    setSources(sourcesReply.isValid() ? sourcesReply.value() : QStringList());

    // 7. Forward Ports (Always from Permanent)
    QDBusMessage fwdMsg = permanentZoneIf.call("getForwardPorts");
    if (fwdMsg.type() == QDBusMessage::ReplyMessage) {
        const QDBusArgument &arg = fwdMsg.arguments().at(0).value<QDBusArgument>();
        QStringList fwdList;
        arg.beginArray();
        while (!arg.atEnd()) {
            QString p, proto, toP, toAddr;
            arg.beginStructure();
            arg >> p >> proto >> toP >> toAddr;
            arg.endStructure();

            QString rule = QString("port=%1:proto=%2:toport=%3").arg(p, proto, toP);
            if (!toAddr.isEmpty()) rule += QString(":toaddr=%1").arg(toAddr);
            fwdList.append(rule);
        }
        arg.endArray();
        setForwardRules(fwdList);
    } else {
        setForwardRules({});
    }

    // 8. Masquerade (Always from Permanent)
    QDBusReply<bool> masqReply = permanentZoneIf.call("queryMasquerade");
    if (masqReply.isValid())
        setMasqueradeState(masqReply.value());
    else
        setMasqueradeState(false);

    // 9. Stealth Mode (Check if Target is DROP)
    QDBusReply<QString> targetReply = permanentZoneIf.call("getTarget");
    setStealthModeState(targetReply.isValid() && targetReply.value() == "DROP");

    QDBusReply<bool> icmpInvReply = permanentZoneIf.call("getIcmpBlockInversion");
    setStrictIcmpState(icmpInvReply.isValid() && icmpInvReply.value());

    // 10. Log Denied (Global)
    QDBusReply<QString> logDeniedReply = fw.call("getLogDenied");
    if (logDeniedReply.isValid()) {
        setLogDeniedState(logDeniedReply.value() != "off");
    } else {
        setLogDeniedState(false);
    }
}

// === MODIFICATION LOGIC (Permanent + Reload) ===

void FirewallBackend::addSource(const QString &source, const QString &zone)
{
    QString path = getPermanentZonePath(zone);
    if (path.isEmpty()) return;
    QDBusInterface zoneIf(FW_SERVICE, path, FW_CONFIG_ZONE_INTERFACE, QDBusConnection::systemBus());
    QDBusReply<void> reply = zoneIf.call("addSource", source);
    if (!reply.isValid()) emit operationError("Failed: " + reply.error().message());
    else reload();
}

void FirewallBackend::removeSource(const QString &source, const QString &zone)
{
    QString path = getPermanentZonePath(zone);
    if (path.isEmpty()) return;
    QDBusInterface zoneIf(FW_SERVICE, path, FW_CONFIG_ZONE_INTERFACE, QDBusConnection::systemBus());
    zoneIf.call("removeSource", source);
    reload();
}

void FirewallBackend::setStealthMode(bool enabled, const QString &zone)
{
    QString path = getPermanentZonePath(zone);
    if (path.isEmpty()) return;
    QDBusInterface zoneIf(FW_SERVICE, path, FW_CONFIG_ZONE_INTERFACE, QDBusConnection::systemBus());
    
    zoneIf.call("setTarget", enabled ? "DROP" : "default");
    reload();
}

void FirewallBackend::setStrictIcmp(bool enabled, const QString &zone)
{
    QString path = getPermanentZonePath(zone);
    if (path.isEmpty()) return;
    QDBusInterface zoneIf(FW_SERVICE, path, FW_CONFIG_ZONE_INTERFACE, QDBusConnection::systemBus());

    if (enabled) {
        zoneIf.call("addIcmpBlockInversion");
        zoneIf.call("addIcmpBlock", "destination-unreachable"); 
        zoneIf.call("addIcmpBlock", "time-exceeded"); 
        
    } else {
        zoneIf.call("removeIcmpBlockInversion");
        zoneIf.call("removeIcmpBlock", "destination-unreachable");
        zoneIf.call("removeIcmpBlock", "time-exceeded");
    }
    reload();
}

void FirewallBackend::addService(const QString &service, const QString &zone)
{
    QString path = getPermanentZonePath(zone);
    if (path.isEmpty()) {
        emit operationError("Could not find path for zone: " + zone);
        return;
    }

    QDBusInterface zoneIf(FW_SERVICE, path, FW_CONFIG_ZONE_INTERFACE, QDBusConnection::systemBus());
    
    QDBusReply<void> reply = zoneIf.call("addService", service);
    
    if (!reply.isValid()) {
        emit operationError("Failed to add service '" + service + "': " + reply.error().message());
    } else {
        reload();
    }
}

void FirewallBackend::removeService(const QString &service, const QString &zone)
{
    QString path = getPermanentZonePath(zone);
    if (path.isEmpty()) return;
    QDBusInterface zoneIf(FW_SERVICE, path, FW_CONFIG_ZONE_INTERFACE, QDBusConnection::systemBus());
    zoneIf.call("removeService", service);
    reload();
}

void FirewallBackend::addPort(const QString &port, const QString &protocol, const QString &zone)
{
    QString path = getPermanentZonePath(zone);
    if (path.isEmpty()) return;

    QDBusInterface zoneIf(FW_SERVICE, path, FW_CONFIG_ZONE_INTERFACE, QDBusConnection::systemBus());
    QDBusReply<void> reply = zoneIf.call("addPort", port, protocol);

    if (!reply.isValid()) {
        emit operationError("Failed to add port: " + reply.error().message());
    } else {
        reload();
    }
}

void FirewallBackend::removePort(const QString &port, const QString &protocol, const QString &zone)
{
    QString path = getPermanentZonePath(zone);
    if (path.isEmpty()) return;
    QDBusInterface zoneIf(FW_SERVICE, path, FW_CONFIG_ZONE_INTERFACE, QDBusConnection::systemBus());
    zoneIf.call("removePort", port, protocol);
    reload();
}

void FirewallBackend::addForwardRule(const QString &src, const QString &proto, const QString &destPort, const QString &destIP, const QString &zone)
{
    QString path = getPermanentZonePath(zone);
    if (path.isEmpty()) {
        emit operationError("Could not find path for zone: " + zone);
        return;
    }

    QDBusInterface zoneIf(FW_SERVICE, path, FW_CONFIG_ZONE_INTERFACE, QDBusConnection::systemBus());
    
    QDBusReply<void> reply = zoneIf.call("addForwardPort", src, proto, destPort, destIP);
    
    if (!reply.isValid()) {
        emit operationError("Failed to add forward rule: " + reply.error().message());
    } else {
        reload();
    }
}

void FirewallBackend::removeForwardRule(const QString &src, const QString &proto, const QString &destPort, const QString &destIP, const QString &zone)
{
    QString path = getPermanentZonePath(zone);
    if (path.isEmpty()) return;
    QDBusInterface zoneIf(FW_SERVICE, path, FW_CONFIG_ZONE_INTERFACE, QDBusConnection::systemBus());
    zoneIf.call("removeForwardPort", src, proto, destPort, destIP);
    reload();
}

void FirewallBackend::changeDefaultZone(const QString &zone)
{
    QDBusInterface fw(FW_SERVICE, FW_PATH, FW_INTERFACE, QDBusConnection::systemBus());
    fw.call("setDefaultZone", zone);
    refresh(zone); 
}

void FirewallBackend::setMasquerade(bool enabled, const QString &zone)
{
    QString path = getPermanentZonePath(zone);
    if (path.isEmpty()) return;
    QDBusInterface zoneIf(FW_SERVICE, path, FW_CONFIG_ZONE_INTERFACE, QDBusConnection::systemBus());
    if (enabled) zoneIf.call("addMasquerade");
    else zoneIf.call("removeMasquerade");
    reload();
}

void FirewallBackend::setLogDenied(bool enabled)
{
    QDBusInterface fw(FW_SERVICE, FW_PATH, FW_INTERFACE, QDBusConnection::systemBus());
    QString value = enabled ? "all" : "off";
    
    fw.call("setLogDenied", value); 
    
    setLogDeniedState(enabled);
}

// === GLOBAL ACTIONS ===

void FirewallBackend::reload()
{
    QDBusInterface fw(FW_SERVICE, FW_PATH, FW_INTERFACE, QDBusConnection::systemBus());
    fw.call("reload");
    refresh(m_defaultZone); 
}

void FirewallBackend::setPanic(bool enabled)
{
    QDBusInterface fw(FW_SERVICE, FW_PATH, FW_INTERFACE, QDBusConnection::systemBus());
    if (enabled)
        fw.call("enablePanicMode");
    else
        fw.call("disablePanicMode");
    
    refresh(m_defaultZone);
}
