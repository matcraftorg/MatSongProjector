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

#include <QApplication>
#include <QTranslator>
#include <QTextCodec>
#include "mainwindow.h"
#include "qsettings.h"

int main(int argc, char *argv[])
{
  QCoreApplication::setApplicationVersion("1.0.0");
  QCoreApplication::setApplicationName(QString("MatSongProjector"));

  QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8")); // Да не се коментира този ред! Програмата работи изцяло с UTF-8 кодировка, като новите файлове се записват като UTF-8-BOM под Windows и без BOM ако не е Windows.

  Q_INIT_RESOURCE(resources);
  QApplication app(argc, argv);
  app.setWindowIcon(QIcon(":/resources/images/logo128x128.png"));
  app.addLibraryPath(QCoreApplication::applicationDirPath() + "/Plugins"); //app.addLibraryPath("./Plugins");
  app.addLibraryPath(QCoreApplication::applicationDirPath() + "/Plugins/iconengines"); //app.addLibraryPath("./Plugins/iconengines");
  app.addLibraryPath(QCoreApplication::applicationDirPath() + "/Plugins/imageformats"); //app.addLibraryPath("./Plugins/imageformats");
  app.addLibraryPath(QCoreApplication::applicationDirPath() + "/Plugins/platforms"); //app.addLibraryPath("./Plugins/platforms"); // ???
  app.addLibraryPath(QCoreApplication::applicationDirPath() + "/Plugins/styles"); //app.addLibraryPath("./Plugins/styles");

  // Създаване на TS и QM файловете:
  // TS са XML базирани файлове, които съдържат всички стрингове от формите (.ui файловете) и всички стрингове от .h и .cpp файловете,
  // които са поставени в методите tr(). Те могат да се превеждат на съответния език или с текстова програма, работеща с UTF-8 или с Qt Linguist.
  // QM са бинарни файлове за бърз достъп до преведените стрингове. Генерират се от TS файловете.
  //
  // От терминала се влиза в директорията на .pro файла и се пише:
  //
  // C:\Qt\Qt5.5.1\5.5\mingw492_32\bin\lupdate MatSongProjector.pro
  //
  // Горният път ще е различен, според версията на Qt.
  // lupdate трябва да се зададе с пътя до него, а .pro файлът не трябва да се задава с пътя.
  // Това ще създаде TS файловете, описани в .pro файлът (TRANSLATIONS += ...).
  // TS файловете се превеждат и после по аналогичен начин в терминала се изпълнява lrelease (или от Qt Linguist се избира File|Release):
  //
  // C:\Qt\Qt5.5.1\5.5\mingw492_32\bin\lrelease MatSongProjector.pro
  //
  // Това създава QM файловете.
  // Всички тези файлове се копират на ръка в папка bin/Tr.
  // Един от QM файловете трябва да се избере и зареди:
  QTranslator translator;
  //translator.load(QLocale::system(), QString("%1/Tr/MatSongProjector").arg(QCoreApplication::applicationDirPath()), QString(), QString(), QString()); // Така не става.
  //translator.load(QLocale::system(), "MatSongProjector", ".", QString("%1/Tr").arg(QCoreApplication::applicationDirPath()), ".qm"); // Така не става.
  translator.load(QString("%1/Tr/MatSongProjector.%2").arg(QCoreApplication::applicationDirPath()).arg(QLocale::system().name()));
  app.installTranslator(&translator);
  
  //QFont font = app.font();
  //font.setPointSize(10);
  //app.setFont(font);

  app.setStyle("Fusion"); // Windows WindowsXP WindowsVista Fusion Macintosh

  MainWindow w;
  w.show();

  return app.exec();
}
