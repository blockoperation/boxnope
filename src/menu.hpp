// Copyright (c) 2016, blockoperation. All rights reserved.
// boxnope is distributed under the terms of the BSD license.
// See LICENSE for details.

#ifndef BOXNOPE_MENU_HPP
#define BOXNOPE_MENU_HPP

#include <QApplication>
#include <QMenu>

QMenu* createMenuFromFile(QApplication* app, QWidget* parent, QString filename);

#endif
