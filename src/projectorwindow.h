/***************************************************************************
 *   MatSongProjector                                                      *
 *   Copyright (C) 2010-2020 by MatCraft, Bulgaria                         *
 *   matcraft.org@gmail.com                                                *
 *   http://www.matcraft.org/                                              *
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
 
/***************************************************************************
 *   Идеи и части от кода са взети от програмата проектор на Ненчо Ненов   *
 ***************************************************************************/

#ifndef PROJECTORWINDOW_H
#define PROJECTORWINDOW_H

#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QTimer>

class ProjectorWindow : public QWidget
{
Q_OBJECT

private:
  bool projectorMaximized; // Флаг чрез който ивентът за максимизиране се извиква само веднъж. Използва се заради Убунту Линукс.

public:
  QLabel *labelBlack;
  QLabel *labelWallpaper;
  QLabel *labelText;
  ProjectorWindow(QWidget *parent = 0);

protected:
  void showEvent(QShowEvent *event); // Този ивент се използва за предварително премащабиране на картинките за формата за проектора. Заради Windows.
  void resizeEvent(QResizeEvent *event); // Този ивент се използва за предварително премащабиране на картинките за формата за проектора. Заради Linux.

private slots:
  void SlotHideWindow();

signals:
  void projectorWindowMaximized();
};

#endif // PROJECTORWINDOW_H
