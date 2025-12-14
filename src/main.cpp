#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>
#include <QSurfaceFormat>
#include <QQuickWindow>
#include <QIcon>

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

    // 2. SETUP ORGANIZATION
    app.setOrganizationName("Nitrux");
    app.setOrganizationDomain("nxos.org");
    app.setApplicationName("Cinderward");
    
    // 3. SETUP WINDOW ICON
    QIcon appIcon = QIcon::fromTheme("security-high", QIcon(":/assets/cinderward.svg"));
    app.setWindowIcon(appIcon);

    KLocalizedString::setApplicationDomain("cinderward");

    // 4. SETUP ABOUT DATA
    KAboutData about(QStringLiteral("cinderward"),
                     i18n("Cinderward"),
                     "0.0.1",
                     i18n("Simple Firewall Policy Editor"),
                     KAboutLicense::BSD_3_Clause,
                     i18n("© 2025 Nitrux Latinoamericana S.C."));

    about.addAuthor(QStringLiteral("Uri Herrera"), i18n("Developer"), QStringLiteral("uri_herrera@nxos.org"));
    about.setHomepage("https://nxos.org");
    about.setProductName("nitrux/cinderward");
    about.setOrganizationDomain("nxos.org");
    
    // Explicitly set the logo for the About Dialog using the icon we resolved
    about.setProgramLogo(app.windowIcon());

    KAboutData::setApplicationData(about);

    // 3. INITIALIZE MAUIKIT
    MauiApp::instance()->setIconName("qrc:/assets/cinderward.svg"); 

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
