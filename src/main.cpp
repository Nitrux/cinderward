#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>
#include <QSurfaceFormat>
#include <QQuickWindow>

#include <KLocalizedString>
#include <KLocalizedContext>
#include <KAboutData>
#include <MauiKit4/Core/mauiapp.h>
#include "firewallbackend.h"

int main(int argc, char *argv[])
{
    // 1. ENABLE WINDOW TRANSPARENCY
    QSurfaceFormat format;
    format.setAlphaBufferSize(8);
    QSurfaceFormat::setDefaultFormat(format);

    QGuiApplication app(argc, argv);

    app.setOrganizationName("Nitrux");
    app.setOrganizationDomain("nxos.org");
    app.setApplicationName("Cinderward");
    
    app.setWindowIcon(QIcon("://assets/cinderward_64.png"));

    KLocalizedString::setApplicationDomain("cinderward");

    // 2. SETUP ABOUT DATA
    KAboutData about(QStringLiteral("cinderward"),
                     i18n("Cinderward"),
                     "0.1.0",
                     i18n("Simple Firewall Policy Editor"),
                     KAboutLicense::BSD_3_Clause,
                     i18n("© 2025 Nitrux Latinoamericana S.C."));

    about.addAuthor(QStringLiteral("Nitrux"), i18n("Developer"), QStringLiteral("uri_herrera@nxos.org"));
    about.setHomepage("https://nxos.org");
    about.setProductName("nitrux/cinderward");
    about.setOrganizationDomain("nxos.org");
    about.setProgramLogo(app.windowIcon());

    KAboutData::setApplicationData(about);

    // 3. INITIALIZE MAUIKIT
    MauiApp::instance()->setIconName("preferences-security-firewall");

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