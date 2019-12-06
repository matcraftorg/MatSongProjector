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

#ifndef CIRCLEWIDGET_H
#define CIRCLEWIDGET_H

#include <QLabel>

#define CIRCLE_COUNT 3

class CircleWidget : public QLabel
{
Q_OBJECT

private:
  struct CIRCLE // Преоразмерени картинки.
  {
    int X;
    int Y;
    double diameter;
  };

  QTimer *timer;
  int circleBrightness; // Яркостта на кръговете.
  double maxDiameter;
  CIRCLE circles[CIRCLE_COUNT];

  int RandomInt(int from, int to);

public:
  bool enabledCircles; // Флаг, указващ дали е разрешено показването на кръговете.

  CircleWidget(QWidget *parent = 0);
  void enableCircles(bool enable); // Чрез този метод ще разрешава или забранява показването на кръговете.
  void setCircleBrightness(int circleBrightness = 0);

private slots:
  void nextStep();

protected:
  void resizeEvent(QResizeEvent *event);
  void paintEvent(QPaintEvent *event);
};

#endif
