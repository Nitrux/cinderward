// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Nitrux Latinoamericana S.C. <hello@nxos.org>

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
    paths.append(QStringLiteral(":/icons"));
    QIcon::setThemeSearchPaths(paths);

    // 2. SETUP ORGANIZATION
    app.setOrganizationName(QStringLiteral("Nitrux"));
    app.setApplicationName(QStringLiteral("Cinderward"));
    
    // 3. SETUP WINDOW ICON
    QIcon appIcon = QIcon::fromTheme(QStringLiteral("cinderward"), QIcon(QStringLiteral(":/assets/cinderward.svg")));
    app.setWindowIcon(appIcon);

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("cinderward"));

    // 5. SETUP ABOUT DATA
    KAboutData about(QStringLiteral("cinderward"),
                     i18n("Cinderward"),
                     QStringLiteral("0.0.3"),
                     i18n("Simple firewall policy editor."),
                     KAboutLicense::BSD_3_Clause,
                     // "Maui" must be present for the footer logo to appear
                     i18n("© %1 Made by Nitrux | Built with MauiKit", QString::number(QDate::currentDate().year())),
                     QString::fromUtf8(GIT_BRANCH) + QStringLiteral("/") + QString::fromUtf8(GIT_COMMIT_HASH));

    about.addAuthor(QStringLiteral("Uri Herrera"), i18n("Developer"), QStringLiteral("uri_herrera@nxos.org"));
    about.setHomepage(QStringLiteral("https://nxos.org"));
    about.setProductName(QByteArrayLiteral("nitrux/cinderward"));
    about.setOrganizationDomain(QByteArrayLiteral("nxos.org"));    
    about.setDesktopFileName(QStringLiteral("org.nxos.cinderward"));
    
    // Set the logo for the About Dialog header
    about.setProgramLogo(app.windowIcon());

    KAboutData::setApplicationData(about);

    // 6. INITIALIZE MAUIKIT
    // Initializes the singleton and theming
    MauiApp::instance()->setIconName(QStringLiteral("qrc:/assets/cinderward.svg")); 

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
