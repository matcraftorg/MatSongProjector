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

#include <QtGui>
#include "circlewidget.h"
//#include <QMessageBox> // QMessageBox::information(this, QCoreApplication::applicationName(), QString(""));

CircleWidget::CircleWidget(QWidget *parent) : QLabel(parent)
{
  circleBrightness = 40;
  maxDiameter = width() * 2;
  for (int i = 0; i < CIRCLE_COUNT; i++)
  {
    circles[i].X = RandomInt(-width(), width());
    circles[i].Y = RandomInt(-width(), width());
    circles[i].diameter = (double)RandomInt(0, (int)maxDiameter);
  }

  enabledCircles = false;
  
  QPalette pal = palette();
  pal.setBrush(QPalette::Base, QColor(0, 0, 0)); // 000000 // 191, 142, 74 (bf8e4a), ако се използва оранжевият фон.
  pal.setBrush(QPalette::Window, QColor(0, 0, 0)); // 000000 // 191, 142, 74 (bf8e4a), ако се използва оранжевият фон.
  setPalette(pal);
  setAutoFillBackground(true);

  setBackgroundRole(QPalette::Base);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(nextStep()));
}

int CircleWidget::RandomInt(int from, int to)
{
  //qrand() returns a value between 0 and RAND_MAX.
  int interval = to - from;
  double randIntD = (double)qrand() / (double)RAND_MAX * (double)interval;
  int randInt = (int)randIntD + from;
  if (randInt < from) randInt = from;
  if (randInt > to) randInt = to;
  return randInt;
}

void CircleWidget::enableCircles(bool enable)
{
  enabledCircles = enable;
  if (enable)
  {
    maxDiameter = width() * 2;
    for (int i = 0; i < CIRCLE_COUNT; i++)
    {
      circles[i].X = RandomInt(-width(), width());
      circles[i].Y = RandomInt(-width(), width());
      circles[i].diameter = (double)RandomInt(0, (int)maxDiameter);
    }

    timer->start(70);
    setVisible(true);
  }
  else
  {
    timer->stop();
    setVisible(false);
  }
}

void CircleWidget::setCircleBrightness(int circleBrightness)
{
  if (circleBrightness > 0 && this->circleBrightness <= 255) this->circleBrightness = circleBrightness;
}

void CircleWidget::nextStep()
{
  maxDiameter = width() * 2;
  for (int i = 0; i < CIRCLE_COUNT; i++)
  {
    circles[i].diameter += (width() > 600) ? 5.0 : 2.0;
    if (circles[i].diameter > maxDiameter)
    {
      circles[i].X = RandomInt(-width(), width());
      circles[i].Y = RandomInt(-width(), width());
      circles[i].diameter = 1.0; // Не трябва да е 0!
    }
  }
  update();
}

void CircleWidget::resizeEvent(QResizeEvent *event)
{
  QLabel::resizeEvent(event);

  // Този метод е излишен, но го оставям за всеки случай.
  maxDiameter = width() * 2;
  for (int i = 0; i < CIRCLE_COUNT; i++)
  {
    circles[i].X = RandomInt(-width(), width());
    circles[i].Y = RandomInt(-width(), width());
    circles[i].diameter = (double)RandomInt(0, (int)maxDiameter);
  }
}

void CircleWidget::paintEvent(QPaintEvent */*event*/)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, false); // Това (true) заглажда кръговете, но в случая няма значение дали са груби, защото са на заден план.
  painter.translate(width() / 2, height() / 2); // Това премества началото на координатната система в средата на екрана.

  for (int i = 0; i < CIRCLE_COUNT; i++)
  {
    double radius = circles[i].diameter / 2.0;
    double w = width() / 5;

    double delta1 = circles[i].diameter / w;   if (delta1 > 1) delta1 = 1; // Тази делта ще направи невидимо (избледнено) създаването на кръга (защото рязкото появяване на нов кръг, макар и да започва с размер нула, е някак дразнещо).
    double delta2 = circles[i].diameter / maxDiameter; // Тази делта ще направи невидимо (избледнено) изчезването на кръга (защото рязкото изчезване на кръга, е дразнещо).

    int alpha = 255.0
              - (255.0 - delta1 * 255.0) // Това причинява избледняване на кръга в началото. Става постепенно.
              - (delta2 * delta2 * delta2 * delta2 * 255.0); // Това причинява избледняване на кръга в края. По този начин кръгът избледнява в последния момент (заради повдигането на 4-та степен), а не плавно.

    if (alpha > 0 && alpha < 256)
    {
      //painter.setPen(QPen(QColor(20, 20, 20, alpha), (radius > w) ? w : radius)); // Едноцветно. // 141414 // 207, 159, 90 (cf9f5a), ако се използва оранжевият фон.
      painter.setPen(QPen(QColor::fromHsl((int)((double)alpha*1.407), 128, circleBrightness, alpha), (radius > w) ? w : radius)); // Цветно. H съставката (hue - цвят, нюанс) сменя всички цветове (приема стойности от 0 до 359, затова alpha, която тук се използва не по предназначение, се умножава по 1.407 - 255*1.407=358). S съставката (saturation) определя наситеността на цветовете (255 - най-ярки, 128 - пастелени). L съставката (lightness) е яркостта (circleBrightness=40 - бледи цветове).
      painter.drawEllipse(QRectF(-radius + circles[i].X, -radius + circles[i].Y, circles[i].diameter, circles[i].diameter)); // По този начин кръговете се разширяват.
    }
  }
}
