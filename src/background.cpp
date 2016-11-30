// Copyright (c) 2016, blockoperation. All rights reserved.
// boxnope is distributed under the terms of the BSD license.
// See LICENSE for details.

#include "background.hpp"

#include <QScreen>
#include <QX11Info>
#include <QPainter>
#include <QMenu>
#include <QWindow>

#include <xcb/xcb.h>

namespace
{

void xcbSetDesktopWindow(xcb_connection_t* c, xcb_window_t wid)
{
    static const char* desktop = "_NET_WM_WINDOW_TYPE_DESKTOP";
    auto desktop_cookie = xcb_intern_atom(c, 0, strlen(desktop), desktop);

    static const char* type = "_NET_WM_WINDOW_TYPE";
    auto type_cookie = xcb_intern_atom(c, 0, strlen(type), type);

    auto desktop_reply = xcb_intern_atom_reply(c, desktop_cookie, nullptr);
    auto type_reply = xcb_intern_atom_reply(c, type_cookie, nullptr);

    if (desktop_reply && type_reply)
    {
        xcb_change_property(
            c, XCB_PROP_MODE_REPLACE, wid, type_reply->atom, XCB_ATOM_ATOM, 32,
            1, &desktop_reply->atom
        );
        xcb_flush(c);
    }

    free(desktop_reply);
    free(type_reply);
}

}

BackgroundWindow::BackgroundWindow(QScreen* scr, QWidget* parent)
:   QMainWindow(parent),
    wallpaper_(),
    wallpaper_scaled_(),
    menu_(nullptr)
{
    doResize(scr->virtualGeometry());
    QObject::connect(scr, &QScreen::virtualGeometryChanged, [this](const QRect& g)
    {
        doResize(g);
    });

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QMainWindow::customContextMenuRequested, [this](const QPoint& pos)
    {
        if (menu_)
            menu_->popup(mapToGlobal(pos));
    });

    xcbSetDesktopWindow(QX11Info::connection(), winId());
}

void BackgroundWindow::setWallpaper(const QPixmap& wallpaper)
{
    wallpaper_ = wallpaper;
    wallpaper_scaled_ = wallpaper.scaled(width(), height());
}

void BackgroundWindow::setMenu(QMenu* menu)
{
    menu_ = menu;
}

void BackgroundWindow::doResize(const QRect& g)
{
    setGeometry(g);

    if (!wallpaper_.isNull())
        wallpaper_scaled_ = wallpaper_.scaled(g.width(), g.height());
}

void BackgroundWindow::paintEvent(QPaintEvent*)
{
    if (!wallpaper_scaled_.isNull())
    {
        QPainter painter(this);
        painter.drawPixmap(0, 0, wallpaper_scaled_);
    }
}
