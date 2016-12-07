// Copyright (c) 2016, blockoperation. All rights reserved.
// boxnope is distributed under the terms of the BSD license.
// See LICENSE for details.

#ifndef BOXNOPE_BACKGROUND_HPP
#define BOXNOPE_BACKGROUND_HPP

#include <QWidget>
#include <QMenu>

enum class WallpaperMode
{
    Fill,
    Scale,
    Tile,
    None
};

WallpaperMode getWallpaperMode(QString s);

class BackgroundWindow : public QWidget
{
public:
    BackgroundWindow(QScreen* scr, QWidget* parent);

    void setWallpaper(const QPixmap& wallpaper, WallpaperMode mode);
    void setMenu(QMenu* menu);

protected:
    virtual void paintEvent(QPaintEvent*) override;
    virtual void resizeEvent(QResizeEvent*) override;

private:
    QPixmap wallpaper_orig_;
    QPixmap wallpaper_;
    WallpaperMode wallpaper_mode_;
    QMenu* menu_;

    void updateWallpaper();
};

#endif
