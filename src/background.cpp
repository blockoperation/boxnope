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

#include "utils.hpp"

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

WallpaperMode getWallpaperMode(QString s)
{
    if (s == QL1S("fill"))
        return WallpaperMode::Fill;
    else if (s == QL1S("scale"))
        return WallpaperMode::Scale;
    else if (s == QL1S("tile"))
        return WallpaperMode::Tile;

    return WallpaperMode::None;
}

BackgroundWindow::BackgroundWindow(QScreen* scr, QWidget* parent)
:   QMainWindow(parent),
    wallpaper_orig_(),
    wallpaper_(),
    wallpaper_mode_(WallpaperMode::None),
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

void BackgroundWindow::setWallpaper(const QPixmap& wallpaper, WallpaperMode mode)
{
    wallpaper_mode_ = mode;
    wallpaper_orig_ = wallpaper;
    updateWallpaper();
}

void BackgroundWindow::setMenu(QMenu* menu)
{
    menu_ = menu;
}

void BackgroundWindow::doResize(const QRect& g)
{
    setGeometry(g);
    updateWallpaper();
}

void BackgroundWindow::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    if (wallpaper_mode_ == WallpaperMode::Tile)
        painter.drawTiledPixmap(geometry(), wallpaper_);
    else
        painter.drawPixmap(0, 0, wallpaper_);
}

void BackgroundWindow::updateWallpaper()
{
    if (wallpaper_orig_.isNull())
        wallpaper_mode_ = WallpaperMode::None;

    switch (wallpaper_mode_)
    {
        case WallpaperMode::Fill:
            wallpaper_ = wallpaper_orig_.scaled(size(), Qt::KeepAspectRatioByExpanding,
                                                Qt::SmoothTransformation);
            break;

        case WallpaperMode::Scale:
            wallpaper_ = wallpaper_orig_.scaled(size(), Qt::IgnoreAspectRatio,
                                                Qt::SmoothTransformation);
            break;

        case WallpaperMode::Tile:
            wallpaper_ = wallpaper_orig_;
            break;

        case WallpaperMode::None:
            if (wallpaper_.size() != size())
                wallpaper_ = QPixmap(size());
            wallpaper_.fill(Qt::black);
            break;

        default:
            break;
    }
}
