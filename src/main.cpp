// Copyright (c) 2016-2017, blockoperation. All rights reserved.
// boxnope is distributed under the terms of the BSD license.
// See LICENSE for details.

#include <QApplication>
#include <QCommandLineParser>
#include <QProcess>

#if WITH_KGLOBALACCEL
#include <kglobalaccel.h>
#endif

#include "background.hpp"
#include "menu.hpp"
#include "utils.hpp"

namespace
{

template <typename F>
void addShortcut(QObject* parent, QString name, QKeySequence keys, F f)
{
#if WITH_KGLOBALACCEL
    QAction* action = new QAction(name, parent);
    action->setObjectName(name);
    action->setProperty("componentName", QL1S("boxnope"));
    QObject::connect(action, &QAction::triggered, f);
    KGlobalAccel::setGlobalShortcut(action, keys);
#endif
}

struct Arguments
{
    Arguments()
    :   wm{
            {QSL("w"), QSL("wm")},
            QSL("Path to window manager"),
            "file"
        },
        wm_args{
            {QSL("a"), QSL("wm-arg")},
            QSL("Argument to window manager (may be specified multiple times)"),
            "arg",
            QS()
        },
        autostart{
            {QSL("s"), QSL("autostart")},
            QSL("Path to autostart script"),
            "file"
        },
        autostart_args{
            {QSL("g"), QSL("autostart-arg")},
            QSL("Argument to autostart script (may be specified multiple times)"),
            "arg",
            QS()
        },
        wallpaper{
            {QSL("b"), QSL("wallpaper")},
            QSL("Path to wallpaper"),
            "file"
        },
        wallpaper_mode{
            {QSL("d"), QSL("wallpaper-mode")},
            QSL("Wallpaper mode: fill, scale (default) or tile"),
            QSL("mode"),
            QSL("scale")
        },
        menu{
            {QSL("m"), QSL("menu")},
            QSL("Path to menu configuration"),
            "file"
        }
    {
    };

    QCommandLineOption wm;
    QCommandLineOption wm_args;
    QCommandLineOption autostart;
    QCommandLineOption autostart_args;
    QCommandLineOption wallpaper;
    QCommandLineOption wallpaper_mode;
    QCommandLineOption menu;
};

}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QSL("boxnope"));
    QApplication::setApplicationVersion(QSL("0.3"));
    QObject::connect(&app, &QApplication::lastWindowClosed, [&app]()
    {
        app.quit();
    });

    QCommandLineParser arg_parser;
    arg_parser.setApplicationDescription(QSL("A simple X11 session/launcher"));
    arg_parser.addHelpOption();
    arg_parser.addVersionOption();

    Arguments args;
    arg_parser.addOptions({
        args.wm, args.wm_args,
        args.autostart, args.autostart_args,
        args.wallpaper, args.wallpaper_mode,
        args.menu
    });
    arg_parser.process(app);

    // Set up background window
    BackgroundWindow win(QGuiApplication::primaryScreen());
    if (arg_parser.isSet(args.wallpaper))
        win.setWallpaper(QPixmap(arg_parser.value(args.wallpaper)),
                         getWallpaperMode(arg_parser.value(args.wallpaper_mode)));
    win.show();

    // Start window manager
    QProcess wm;
    if (arg_parser.isSet(args.wm))
    {
        wm.start(
            arg_parser.value(args.wm),
            arg_parser.values(args.wm_args)
        );
        wm.waitForStarted();
    }

    // Start background apps
    QProcess autostart;
    if (arg_parser.isSet(args.autostart))
    {
        autostart.startDetached(
            arg_parser.value(args.autostart),
            arg_parser.values(args.autostart_args)
        );
        autostart.waitForStarted();
    }

    QObject::connect(&app, &QApplication::aboutToQuit, [&wm, &autostart]()
    {
        autostart.close();
        autostart.waitForFinished();
        wm.close();
        wm.waitForFinished();
    });

    // Set up menu
    QMenu* menu = nullptr;
    if (arg_parser.isSet(args.menu))
        menu = createMenuFromFile(&win, arg_parser.value(args.menu));

    if (menu)
    {
        win.setMenu(menu);
        addShortcut(menu, QSL("Open launcher menu"), {Qt::META + Qt::Key_Space}, [menu]()
        {
            menu->popup(QCursor::pos());
        });
    }

    return app.exec();
}
