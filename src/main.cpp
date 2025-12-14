#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>
#include <QSurfaceFormat>
#include <QQuickWindow>
#include <QIcon>
#include <QDate>

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

    QStringList paths = QIcon::themeSearchPaths();
    paths.append(":/icons");
    QIcon::setThemeSearchPaths(paths);

    // 2. SETUP ORGANIZATION
    app.setOrganizationName("Nitrux");
    app.setApplicationName("Cinderward");
    
    // 3. SETUP WINDOW ICON
    QIcon appIcon = QIcon::fromTheme("preferences-security-firewall", QIcon(":/assets/cinderward.svg"));
    app.setWindowIcon(appIcon);

    KLocalizedString::setApplicationDomain("cinderward");

    // 4. SETUP VERSION (With Git Info)
    QString version = "0.0.3";
#ifdef GIT_COMMIT_HASH
    if (!QString(GIT_COMMIT_HASH).isEmpty()) {
        version += QString(" %1/%2").arg(GIT_BRANCH).arg(GIT_COMMIT_HASH);
    }
#endif

    // 5. SETUP ABOUT DATA
    KAboutData about(QStringLiteral("cinderward"),
                     i18n("Cinderward"),
                     version,
                     i18n("Simple firewall policy editor"),
                     KAboutLicense::BSD_3_Clause,
                     // "Maui" must be present for the footer logo to appear
                     i18n("© %1 Made by Nitrux | Built with MauiKit", QString::number(QDate::currentDate().year())));

    about.addAuthor(QStringLiteral("Uri Herrera"), i18n("Developer"), QStringLiteral("uri_herrera@nxos.org"));
    about.setHomepage("https://nxos.org");
    about.setProductName("nitrux/cinderward");
    about.setOrganizationDomain("nxos.org");    
    about.setDesktopFileName("org.nxos.cinderward");
    
    // Set the logo for the About Dialog header
    about.setProgramLogo(app.windowIcon());

    KAboutData::setApplicationData(about);

    // 6. INITIALIZE MAUIKIT
    // Initializes the singleton and theming
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
