// Copyright (c) 2016, blockoperation. All rights reserved.
// boxnope is distributed under the terms of the BSD license.
// See LICENSE for details.

#ifndef BOXNOPE_BACKGROUND_HPP
#define BOXNOPE_BACKGROUND_HPP

#include <QMainWindow>

class BackgroundWindow : public QMainWindow
{
public:
    BackgroundWindow(QScreen* scr, QWidget* parent);

    void setWallpaper(const QPixmap& wallpaper);
    void setMenu(QMenu* menu);
    void doResize(const QRect& g);

protected:
    virtual void paintEvent(QPaintEvent*) override;

private:
    QPixmap wallpaper_;
    QPixmap wallpaper_scaled_;
    QMenu* menu_;

    void updateWallpaper();
};

#endif
