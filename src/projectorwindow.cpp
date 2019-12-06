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

#include "projectorwindow.h"

ProjectorWindow::ProjectorWindow(QWidget *parent) : QWidget(parent)
{
  setWindowTitle(QString("Projector screen - %1").arg(QCoreApplication::applicationName())); // Важно е заглавието на този прозорец да е различно от това на основния прозорец, за да може този прозорец да се избира за вграждане в софтуер за видеозапис.
  setWindowFlags(Qt::FramelessWindowHint); //Qt::SplashScreen
  setGeometry(QRect(0, 0, 256, 144));
  projectorMaximized = false;

  labelBlack = new QLabel(this);
  QPalette palette;
  QBrush brush(QColor(0, 0, 0));
  brush.setStyle(Qt::SolidPattern);
  palette.setBrush(QPalette::Window, brush);
  palette.setBrush(QPalette::Button, brush);
  palette.setBrush(QPalette::Base, brush);
  labelBlack->setAutoFillBackground(true);
  labelBlack->setPalette(palette);

  labelWallpaper = new QLabel(this);
  labelWallpaper->setAlignment(Qt::AlignCenter);
  labelText = new QLabel(this);
}

void ProjectorWindow::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);

  // Този слот се извиква многократно и затова се прави така, че сигналът за
  // максимизиране да се изпрати само веднъж - при първото извикване на този слот.
  if (!projectorMaximized)
  {
    if (isMaximized() || isFullScreen()) // Ако формата е максимизирана.
    {
      if (height() > 500) // Ако размерът ѝ е по-голям от начално заложения (тази проверка е заради Линукс).
      {
        projectorMaximized = true;
        emit projectorWindowMaximized(); // Излъчва сигнал, който ще се прихване от главната форма.
        QTimer::singleShot(200, this, SLOT(SlotHideWindow()));
      }
    }
  }
}

void ProjectorWindow::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);

  // Този слот се извиква няколко пъти, особено в Линукс и затова се прави така, че сигналът за
  // максимизиране да се изпрати само веднъж.
  if (!projectorMaximized)
  {
    if (isMaximized() || isFullScreen()) // Ако формата е максимизирана.
    {
      if (height() > 500) // Ако размерът ѝ е по-голям от начално заложения (тази проверка е заради Линукс).
      {
        projectorMaximized = true;
        emit projectorWindowMaximized(); // Излъчва сигнал, който ще се прихване от главната форма.
        QTimer::singleShot(200, this, SLOT(SlotHideWindow()));
      }
    }
  }
}

void ProjectorWindow::SlotHideWindow()
{
  // Това е някакво безумие, но не можах да реша този проблем (в Windows)...
  // След излъчване на горния сигнал, този прозорец се скрива, но поради някаква причина не се скрива, а става невидим, но си е там
  // и е над другите прозорци и практически блокира достъпа до компютъра. Ето защо се принудих да използвам таймер, който след 200 ms
  // извиква този слот, където прозорецът се скрива реално. Тъй като извикването на setVisible(false) няма ефект, защото официално
  // прозорецът се води скрит, се налага първо да се извика setVisible(true), което да покаже прозореца. Безумие...

  setVisible(true);  // Показва прозореца за проектора.
  setVisible(false); // Скрива прозореца за проектора.
}
