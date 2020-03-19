/***************************************************************************
 *   MatSongProjector                                                      *
 *   Copyright (C) 2010-2020 by MatCraft, Bulgaria                         *
 *   matcraft.org@gmail.com                                                *
 *   https://www.matcraft.org/                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef EXTEXTWINDOW_H
#define EXTEXTWINDOW_H

#include <QtGui>
#include <QMainWindow>
#include <QToolBar>
#include <QLayout>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QTextStream>

class QToolBar;
class QPushButton;
class QPlainTextEdit;

class ExTextWindow : public QMainWindow
{
  Q_OBJECT

public:
  ExTextWindow(QString filePath, QString fileName, int Width, int Height, QWidget *parent = 0);
  ~ExTextWindow();

private:
  QString filePath; // Името на файла с пътя.
  QString fileName; // Само името на файла.
  bool    isUTF8;    // Запомня дали кодировката е UTF-8, за да знае с каква кодировка да го запише при Save.
  bool    isUTF8BOM; // Запомня дали кодировката е UTF-8, за да знае с каква кодировка да го запише при Save.
  bool    isWindows;
  QToolBar    *toolBar;
  QPushButton *btnSave;
  QPushButton *btnClose;
  QPlainTextEdit *editor;

  void closeEvent(QCloseEvent *event);
  void keyPressEvent(QKeyEvent *event);
  bool isUTF8TextCodec(QString fileName);
  bool isUTF8BOMTextCodec(QString fileName);
  bool maybeSave();
  bool fileSave();

private slots:
  void save();

signals:
  void textChanged(QString fileName);
};

#endif
