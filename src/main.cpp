#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>
#include <QSurfaceFormat>
#include <QQuickWindow>

#include <KLocalizedString>
#include <KLocalizedContext>
#include <MauiKit4/Core/mauiapp.h>
#include "firewallbackend.h"

int main(int argc, char *argv[])
{
    // 1. ENABLE WINDOW TRANSPARENCY
    // This tells the window manager we support transparency.
    QSurfaceFormat format;
    format.setAlphaBufferSize(8);
    QSurfaceFormat::setDefaultFormat(format);

    QGuiApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Nitrux");
    QCoreApplication::setOrganizationDomain("nxos.org");
    QCoreApplication::setApplicationName("Cinderward");

    KLocalizedString::setApplicationDomain("cinderward");
    MauiApp::instance()->setIconName("security-high");

    qmlRegisterType<FirewallBackend>("org.nitrux.firewall", 1, 0, "FirewallBackend");

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
