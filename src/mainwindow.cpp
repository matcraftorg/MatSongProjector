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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSettings>
#include <QDir>
#include <QPainter>
#include <QStringList>
#include <QTextStream>
#include <QFontDialog>
#include <QColor>
#include <QColorDialog>
#include <QFileDialog>
#include <QCloseEvent>
#include <QTextCodec>
#include <QProgressBar>
#include <QInputDialog>
#include <QMessageBox> // QMessageBox::information(this, QCoreApplication::applicationName(), QString(""));

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  mainWindowSize = size();
  mainWindowPos = pos();

  QFont font = QApplication::font();
  font.setPointSize(8);
  QApplication::setFont(font);

  titlePanelHeight = 92;
  songListWidgetZoom = 10;
  textListWidgetZoom = 10;
  customTextEditZoom = 10;
  currentCouplet = "";
  currentCouplets.clear();
  rowsInCouplet.clear();
  wallpapersPath = "";
  songsPath = "";
  pictures1Path = "";
  pictures2Path = "";
  pictures1FilesCurrentItem = -1;
  pictures2FilesCurrentItem = -1;
  lineSpacing = 100;
  textHeight = 0;
  textOutline = 0;
  textUpper = false;
  embeddedWallpaperCircleBrightness = 40;
  
  ui->frameCustomWallpaper->setVisible(false);
  ui->frameSongsFile->setVisible(false);

  ui->leTabPictures1Interval->setValidator(new QIntValidator(1, 16777215));
  ui->leTabPictures2Interval->setValidator(new QIntValidator(1, 16777215));

  QPalette palette = ui->lwText->palette();
  lwTextColorB = palette.brush(QPalette::Inactive, QPalette::Window).color(); // Може да се използва QPalette::Inactive, QPalette::Highlight, но в Линукс цветът на Inactive може да е като на Active и става все едно всички са селектирани. Затова се взима Window вместо Highlight.
  QColor colorHighlight = palette.brush(QPalette::Active, QPalette::Highlight).color();
  palette.setBrush(QPalette::Highlight, colorHighlight); // Прави цвета на селектиране да е един и същ за Active, Inactive и Disabled.
  ui->lwText->setPalette(palette);
  
  PW = new ProjectorWindow; // Формата за проектора.
  connect(PW, SIGNAL(projectorWindowMaximized()), this, SLOT(projectorWindowMaximized())); // Този слот ще се изпълни само веднъж, при първото максимизиране на формата на проектора.

  circleWidgetPreview = new CircleWidget(ui->labelBlack);
  circleWidgetProjector = new CircleWidget(PW->labelBlack);

  loadSettings(); // Прочита предходната сесия.

  if (projectorScreenWidth > 10 && projectorScreenHeight > 10) // Ако в INI файла са зададени стойности от потребителя (на ръка).
  {
    PW->move(projectorScreenX, projectorScreenY);
    PW->setMinimumSize(QSize(projectorScreenWidth, projectorScreenHeight));
    PW->setMaximumSize(QSize(projectorScreenWidth, projectorScreenHeight));

    resizeGeometry();
  }
  else // Ако размерите на проектора трябва да се изчислят автоматично.
  {
    PW->move(projectorScreenXAuto, 0); // Отмества формата на projectorScreenXAuto точки за да се покаже на другия екран (т.е. на проектора). Ако компютъра е само с един екран (без проектор), формата ще се покаже на този екран.
    PW->showFullScreen(); //PW->showMaximized(); // Показва формата максимизирана, но почти веднага след това (в слота projectorWindowMaximized) ще я скрие. Идеята е да се вземе размера ѝ след като се максимизира.

    //resizeGeometry(); - този метод се извиква в слота projectorWindowMaximized.
  }
  //auto const full_screen_rect = QApplication::desktop()->screenGeometry();

  if (ui->cbWallpaper->currentIndex() == 0) // Embedded Wallpaper.
  {
    circleWidgetPreview->setCircleBrightness(embeddedWallpaperCircleBrightness);
    circleWidgetProjector->setCircleBrightness(embeddedWallpaperCircleBrightness);
    circleWidgetPreview->enableCircles(true);
    circleWidgetProjector->enableCircles(true);
  }

  if (ui->lwSongsFavourites->count() > 0)
  {
    ui->lwSongsFavourites->setCurrentRow(0);
    on_lwSongsFavourites_itemClicked(ui->lwSongsFavourites->currentItem()); // Зарежда текста.
  }
  else if (ui->lwSongs->count() > 0)
  {
    ui->lwSongs->setCurrentRow(0);
    on_lwSongs_itemClicked(ui->lwSongs->currentItem()); // Зарежда текста.
  }
  //else
  //{
  //  ui->lwSongsFavourites->setCurrentRow(-1); // Премахва селекцията.
  //  ui->lwSongs->setCurrentRow(-1); // Премахва селекцията.
  //}
  ui->lwSongs->setFocus();
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
  QMainWindow::closeEvent(e);

  if (e->type() == QCloseEvent::Close)
  {
    saveSettings(); // Записва текущата сесия.
    if (SDPersonalWallpaperChanged) savePersonalWallpaperSet(); // Записва текущите настройки за собствените картинки за фона на песните (но само ако са били направени някакви промени).
    PW->close();
  }
}

void MainWindow::keyPressEvent(QKeyEvent * event)
{
  QMainWindow::keyPressEvent(event);

  if (event->modifiers() == Qt::ControlModifier) // Ако е натиснат Ctrl.
  {
  }
  else if (event->modifiers() == Qt::ShiftModifier) // Ако е натиснат Shift.
  {
  }
  else if (event->modifiers() == Qt::AltModifier) // Ако е натиснат Alt.
  {
  }
  else // Ако НЕ е натиснат Ctrl или Shift или Alt.
  {
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)
    {
      if (ui->tabWidget->currentIndex() == 0 && !ui->leTabSongsFilter->hasFocus()) // Ако е показан раздел "Songs".
      {
        ui->leTabSongsFilter->clear();
      }
    }
    else if (event->key() == Qt::Key_Left)
    {
      if (ui->tabWidget->currentIndex() == 0 && ui->lwText->hasFocus() && ui->lwText->count() > 0) // Ако е показан раздел "Songs".
      {
        int row = ui->lwText->currentRow() - 1;
        if (row >= 0)
        {
          ui->lwText->setCurrentRow(row);
          on_lwText_itemClicked(ui->lwText->currentItem());
        }
      }
    }
    else if (event->key() == Qt::Key_Right)
    {
      if (ui->tabWidget->currentIndex() == 0 && ui->lwText->hasFocus() && ui->lwText->count() > 0) // Ако е показан раздел "Songs".
      {
        int row = ui->lwText->currentRow() + 1;
        if (row < ui->lwText->count())
        {
          ui->lwText->setCurrentRow(row);
          on_lwText_itemClicked(ui->lwText->currentItem());
        }
      }
    }
  }
}

void MainWindow::resizeEvent(QResizeEvent * event)
{
  QMainWindow::resizeEvent(event);
  if ( !(windowState() & Qt::WindowMaximized) && !(windowState() & Qt::WindowMinimized) && !(windowState() & Qt::WindowFullScreen) ) // Ако не е максимизиран или минимизиран или FullScreen.
  {
    mainWindowSize = size();
  }
}

void MainWindow::moveEvent(QMoveEvent *event)
{
  QMainWindow::moveEvent(event);
  if ( !(windowState() & Qt::WindowMaximized) && !(windowState() & Qt::WindowMinimized) && !(windowState() & Qt::WindowFullScreen) ) // Ако не е максимизиран или минимизиран или FullScreen.
  {
    mainWindowPos = pos();
  }
}

void MainWindow::loadSettings()
{
  ui->chbDisable->setChecked(true); // Това ще забрани опресняването на малкия екран (проекторът не е включен в този момент) докато се сетнат всички настройки.

  QString iniFile = QString("%1/MatSongProjector.ini").arg(QCoreApplication::applicationDirPath());
  QSettings settings(iniFile, QSettings::IniFormat, this);
  settings.setIniCodec(QTextCodec::codecForName("UTF-8"));
  
  settings.beginGroup("MainWindow");
    move(settings.value("pos", QPoint(0, 0)).toPoint());
    resize(settings.value("size", QSize(1007, 640)).toSize());
    mainWindowPos = pos();
    mainWindowSize = size();
    if ( settings.value("Maximized", false).toBool() ) // Ако прозорецът трябва да се максимизира.
    {
      QPoint mainWindowPosH = mainWindowPos;  // Цялата тази глупост с помощните променливи, е защото извикването на setWindowState(Qt::WindowMaximized)
      QSize mainWindowSizeH = mainWindowSize; // вдига ивентите resizeEvent и moveEvent преди да е сменило състоянието на Qt::WindowMaximized т.е.
                                              // когато слотовете resizeEvent и moveEvent се изпълняват, състоянието си стой все-още на Qt::WindowNoState
      setWindowState(Qt::WindowMaximized);    // и mainWindowPos и mainWindowSize приемат максималните стойности, смятайки че прозорецът не е максимизиран.
                                              // В следващия момент състоянието вече е променено на Qt::WindowMaximized, но mainWindowPos и mainWindowSize
      mainWindowPos = mainWindowPosH;         // са с максимални стойности. Затова след извикване на setWindowState(Qt::WindowMaximized), те връщат отново
      mainWindowSize = mainWindowSizeH;       // нормалните си стойности (чрез помощните променливи).
    }

    songListWidgetZoom = settings.value("SongListWidgetZoom", 10).toInt();
    textListWidgetZoom = settings.value("TextListWidgetZoom", 10).toInt();
    customTextEditZoom = settings.value("CustomTextEditZoom", 10).toInt();

    QFont font = QApplication::font();
    font = ui->lwSongs->font();
    font.setPointSize(songListWidgetZoom);
    ui->lwSongs->setFont(font);
    ui->lwSongsFavourites->setFont(font);
    
    font = ui->lwText->font();
    font.setPointSize(textListWidgetZoom);
    ui->lwText->setFont(font);
    
    font = ui->teCustomText1->font();
    font.setPointSize(customTextEditZoom);
    ui->teCustomText1->setFont(font);
    ui->teCustomText2->setFont(font);
    ui->teCustomText3->setFont(font);

    songsPath = settings.value("SongsPath", "").toString();
    if (!QDir(songsPath).exists()) songsPath = QDir(QCoreApplication::applicationDirPath()).path();

    textColor.setNamedColor( settings.value("TextColor", "").toString() );
    if (!textColor.isValid()) textColor = QColor(255,255,255); // Бял.

    int index = settings.value("TextAlignH", 1).toInt();
    if (index < 0 || index > 2) index = 1;
    ui->cbTextAlignH->setCurrentIndex(index);

    index = settings.value("TextAlignV", 1).toInt();
    if (index < 0 || index > 2) index = 1;
    ui->cbTextAlignV->setCurrentIndex(index);

    textOutline = settings.value("TextOutline", 4).toInt();
    if (textOutline < 0 || textOutline > 7) textOutline = 4;
    ui->cbTextOutline->setCurrentIndex(textOutline);
    textOutlineColor.setNamedColor( settings.value("TextOutlineColor", "").toString() );
    if (!textOutlineColor.isValid()) textOutlineColor = QColor(68,68,68); // Сив.

    index = settings.value("TitleBarPosition", 1).toInt();
    if (index < 0 || index > 4) index = 1;
    ui->cbTitleBarPosition->setCurrentIndex(index);

    index = settings.value("TitleBarAlpha", 0).toInt();
    if (index < 0 || index > 1) index = 0;
    ui->cbTitleBarAlpha->setCurrentIndex(index);

    textFont = this->font();
    textFont.setFamily(settings.value("TextFontFamily", this->font().family()).toString());
    textFont.setPointSize(settings.value("TextPointSize", 80).toInt());
    textFont.setBold(settings.value("TextFontBold", true).toBool());
    textFont.setItalic(settings.value("TextFontItalic", true).toBool());
    textUpper = settings.value("TextUpper", false).toBool();
    loadSongs(false);
    if (SD.count() > 0)
    {
      loadCouplets(0); // Зарежда стиховете на първата песен от списъка.
    }

    ui->chbTabSongsShowTitle->setChecked( settings.value("SongsShowTitle", true).toBool() );
    QStringList songsFavourites = settings.value("SongsFavourites", QStringList()).toStringList(); // Списък с избраните песни.
    for (int i = 0; i < songsFavourites.count(); i++)
    {
      bool exists = false;
      for (int c = 0; c < SD.count(); c++)
      {
        if (SD[c].fileName == songsFavourites[i])
        {
          exists = true;
          break;
        }
      }
      if (exists) ui->lwSongsFavourites->addItem(songsFavourites[i]);
    }

    index = settings.value("WallpaperPosition", 1).toInt();
    if (index < 0 || index > 3) index = 1; // 1 - Fit Ex
    ui->cbWallpaperPosition->setCurrentIndex(index);

    index = settings.value("WallpaperType", 0).toInt();
    if (index < 0 || index > 3 || index == 2) index = 0; // Индекс 2 се забранява умишлено, защото при първоначално пускане няма да има заредена картинка, която да се покаже на екрана.
    ui->cbWallpaper->setCurrentIndex(index);
    ui->cbWallpaperPosition->setEnabled(index == 1);

    embeddedWallpaperCircleBrightness = settings.value("WallpaperEmCircleBrightness", 60).toInt();
	if (embeddedWallpaperCircleBrightness < 20 || embeddedWallpaperCircleBrightness > 120 || (embeddedWallpaperCircleBrightness % 10) != 0) embeddedWallpaperCircleBrightness = 60;
    ui->btnWallpaperEmbedded->setText(QString("%1").arg(embeddedWallpaperCircleBrightness));

    wallpapersPath = settings.value("WallpapersPath", "").toString();
    if (!QDir(wallpapersPath).exists()) wallpapersPath = QDir(QCoreApplication::applicationDirPath()).path();

    wallpaperFile = settings.value("WallpaperFile", "").toString();

    wallpaperColor.setNamedColor( settings.value("WallpaperColor", "").toString() );
    if (!wallpaperColor.isValid()) wallpaperColor = QColor(0,0,0); // Черен.


    pictures1Path = settings.value("Pictures1Path", "").toString();
    if (!QDir(pictures1Path).exists()) pictures1Path = QDir(QCoreApplication::applicationDirPath()).path();

    index = settings.value("Pictures1Position", 0).toInt();
    if (index < 0 || index > 4) index = 0; // 0 - Auto (Fit or 1:1)
    ui->cbTabPictures1Position->setCurrentIndex(index);

    ui->leTabPictures1Interval->setText( QString("%1").arg(settings.value("Pictures1Interval", 1000).toInt()) );


    pictures2Path = settings.value("Pictures2Path", "").toString();
    if (!QDir(pictures2Path).exists()) pictures2Path = QDir(QCoreApplication::applicationDirPath()).path();

    pictures2Files = settings.value("Pictures2Files", QStringList()).toStringList();

    index = settings.value("Pictures2Position", 0).toInt();
    if (index < 0 || index > 4) index = 0; // 0 - Auto (Fit or 1:1)
    ui->cbTabPictures2Position->setCurrentIndex(index);

    ui->leTabPictures2Interval->setText( QString("%1").arg(settings.value("Pictures2Interval", 1000).toInt()) );


    QStringList customText = settings.value("CustomText1", QStringList()).toStringList();
    for (int i = 0; i < customText.count(); i++)
    {
      ui->teCustomText1->appendPlainText(customText[i]);
    }
    customText = settings.value("CustomText2", QStringList()).toStringList();
    for (int i = 0; i < customText.count(); i++)
    {
      ui->teCustomText2->appendPlainText(customText[i]);
    }
    customText = settings.value("CustomText3", QStringList()).toStringList();
    for (int i = 0; i < customText.count(); i++)
    {
      ui->teCustomText3->appendPlainText(customText[i]);
    }


    // Следващите са само за четене (програмата не променя стойностите им - потребителят може да ги промени само на ръка от файла).
    projectorScreenXAuto = settings.value("projectorScreenXAuto", -10000).toInt();
    if (projectorScreenXAuto == -10000) settings.setValue("projectorScreenXAuto", 2000); // За да се появи в ini-файла, за да може потребителят да смени стойността му от там.
    projectorScreenX = settings.value("projectorScreenX", -10000).toInt();
    if (projectorScreenX == -10000) settings.setValue("projectorScreenX", 0);// За да се появи в ini-файла, за да може потребителят да смени стойността му от там.
    projectorScreenY = settings.value("projectorScreenY", -10000).toInt();
    if (projectorScreenY == -10000) settings.setValue("projectorScreenY", 0);// За да се появи в ini-файла, за да може потребителят да смени стойността му от там.
    projectorScreenWidth = settings.value("projectorScreenWidth", -10000).toInt();
    if (projectorScreenWidth == -10000) settings.setValue("projectorScreenWidth", 0);// За да се появи в ini-файла, за да може потребителят да смени стойността му от там.
    projectorScreenHeight = settings.value("projectorScreenHeight", -10000).toInt();
    if (projectorScreenHeight == -10000) settings.setValue("projectorScreenHeight", 0);// За да се появи в ini-файла, за да може потребителят да смени стойността му от там.
  settings.endGroup();
  
  QPixmap pixmap(16, 16);
  pixmap.fill(QColor(0,0,0));
  customWallpaperPreviewScreen = pixmap;
  customWallpaperProjectorScreen = pixmap;
  // Текст: Цвят
  pixmap.fill(textColor);
  ui->btnTextColor->setIcon(pixmap);
  // Текст, сянка: Цвят
  pixmap.fill(textOutlineColor);
  ui->btnTextOutlineColor->setIcon(pixmap);
  // Фон: Цвят
  pixmap.fill(wallpaperColor);
  ui->btnWallpaperColor->setIcon(pixmap);

  // Текст: Главни/малки букви
  ui->btnTextUpper->setChecked(textUpper);
  // Шрифт
  ui->cbFont->setCurrentIndex(ui->cbFont->findText(textFont.family()));
  // Шрифт: Bold
  ui->btnTextBold->setChecked(textFont.bold());
  // Шрифт: Italic
  ui->btnTextItalic->setChecked(textFont.italic());

  ui->chbDisable->setChecked(false);
}

void MainWindow::saveSettings()
{
  if ( !(windowState() & Qt::WindowMaximized) && !(windowState() & Qt::WindowMinimized) && !(windowState() & Qt::WindowFullScreen) ) // Ако не е максимизиран или минимизиран или FullScreen.
  {
    mainWindowPos = pos();
    mainWindowSize = size();
  }

  QString iniFile = QString("%1/MatSongProjector.ini").arg(QCoreApplication::applicationDirPath());
  QSettings settings(iniFile, QSettings::IniFormat, this);
  settings.setIniCodec(QTextCodec::codecForName("UTF-8"));

  settings.beginGroup("MainWindow");
    settings.setValue("pos", mainWindowPos);
    settings.setValue("size", mainWindowSize);
    settings.setValue("Maximized", (windowState() & Qt::WindowMaximized)? true : false); // true ако е максимизиран, false ако не е.;
    settings.setValue("SongListWidgetZoom", songListWidgetZoom);
    settings.setValue("TextListWidgetZoom", textListWidgetZoom);
    settings.setValue("CustomTextEditZoom", customTextEditZoom);
    settings.setValue("SongsPath", songsPath);
    settings.setValue("TextColor", textColor.name());
    settings.setValue("TextAlignH", ui->cbTextAlignH->currentIndex());
    settings.setValue("TextAlignV", ui->cbTextAlignV->currentIndex());
    settings.setValue("TextOutline", textOutline);
    settings.setValue("TextOutlineColor", textOutlineColor.name());
    settings.setValue("TitleBarPosition", ui->cbTitleBarPosition->currentIndex());
    settings.setValue("TitleBarAlpha", ui->cbTitleBarAlpha->currentIndex());
    settings.setValue("TextFontFamily", textFont.family());
    settings.setValue("TextPointSize", textFont.pointSize());
    settings.setValue("TextFontBold", textFont.bold());
    settings.setValue("TextFontItalic", textFont.italic());
    settings.setValue("TextUpper", textUpper);
    settings.setValue("WallpaperPosition", ui->cbWallpaperPosition->currentIndex());
    settings.setValue("WallpaperType", ui->cbWallpaper->currentIndex());
    settings.setValue("WallpaperEmCircleBrightness", embeddedWallpaperCircleBrightness);
    settings.setValue("WallpapersPath", wallpapersPath);
    settings.setValue("WallpaperFile", wallpaperFile);
    settings.setValue("WallpaperColor", wallpaperColor.name());

    settings.setValue("SongsShowTitle", ui->chbTabSongsShowTitle->isChecked());
    QStringList songsFavourites; // Списък с избраните песни.
    for (int i = 0; i < ui->lwSongsFavourites->count(); i++)
    {
      songsFavourites << ui->lwSongsFavourites->item(i)->text();
    }
    settings.setValue("SongsFavourites", songsFavourites);

    settings.setValue("Pictures1Path", pictures1Path);
    settings.setValue("Pictures1Position", ui->cbTabPictures1Position->currentIndex());
    settings.setValue("Pictures1Interval", ui->leTabPictures1Interval->text().toInt());

    settings.setValue("Pictures2Path", pictures2Path);
    settings.setValue("Pictures2Files", pictures2Files);
    settings.setValue("Pictures2Position", ui->cbTabPictures2Position->currentIndex());
    settings.setValue("Pictures2Interval", ui->leTabPictures2Interval->text().toInt());

    QStringList customText;
    for (QTextBlock it = ui->teCustomText1->document()->begin(); it != ui->teCustomText1->document()->end(); it = it.next())
    {
      customText << it.text();
    }
    settings.setValue("CustomText1", customText);
    customText.clear();
    for (QTextBlock it = ui->teCustomText2->document()->begin(); it != ui->teCustomText2->document()->end(); it = it.next())
    {
      customText << it.text();
    }
    settings.setValue("CustomText2", customText);
    customText.clear();
    for (QTextBlock it = ui->teCustomText3->document()->begin(); it != ui->teCustomText3->document()->end(); it = it.next())
    {
      customText << it.text();
    }
    settings.setValue("CustomText3", customText);
  settings.endGroup();
}

void MainWindow::savePersonalWallpaperSet()
{
  // Записва информацията за собствените картинки, за песните, които имат зададени собствени картинки.
  // Всеки ред от файла съдържа следната информация:
  //
  // Име на файла на песента без пътя^Име на файла на картинката с пътя^Индексът на комбобоксът за позицията на картинката^Флаг, указващ дали се използва собствената картинка
  
  QFileInfo fi(songsPath, ".MatSongProjector.PersonalSongsWallpaper");
  QFile file( fi.filePath() ); // Име на файлът с пътя.
  if (file.exists()) // Ако файлът съществува.
  {
    file.remove(); // Първо го изтрива, защото ако няма записи, той няма (не трябва) да бъде създаден по-долу.
  }
  
  QStringList allLines;
  for (int i = 0; i < SD.count(); i++)
  {
    if (!SD[i].personalWallpaperPath.isEmpty()) // Ако има зададена собствена картинка за тази песен.
    {
      allLines << QString("%1^%2^%3^%4\n").arg(SD[i].fileName).arg(SD[i].personalWallpaperPath).arg(SD[i].personalWallpaperPosition).arg(SD[i].personalWallpaper);
    }
  }

  if (!allLines.isEmpty())
  {
    file.open(QIODevice::WriteOnly);
    for (int i = 0; i < allLines.count(); i++)
    {
      file.write(allLines[i].toUtf8());
    }
    file.close();
  }

  SDPersonalWallpaperChanged = false;
}

void MainWindow::on_btnProjector_clicked()
{
  if (ui->btnProjector->isChecked())
  {
    showWallpaper();
    showText();

    PW->setVisible(true); // Показва прозореца на проектора.
    PW->raise();     PW->activateWindow();   // Това ще изкара на преден план екрана на проектора над другите екрани (на други програми-проектори).
    this->raise();   this->activateWindow(); // Това ще изкара на преден план прозореца на програмата (това е много важно иначе може програмата да остане зад екрана на проектора) .

    // Зелен цвят на текста на бутон "ПРОЕКТОР".
    QPalette palette;
    palette.setBrush(QPalette::ButtonText, QColor(0, 170, 170));
    ui->btnProjector->setPalette(palette);
  }
  else
  {
    PW->lower(); // Това ще постави на заден план екрана на проектора под другите екрани (на други програми-проектори).
    PW->setVisible(false); // Скрива прозореца на проектора.

    // Цвят по подразбиране на текста на бутон "ПРОЕКТОР".
    QPalette palette;
    palette.setBrush(QPalette::ButtonText, ui->btnAbout->palette().brush(QPalette::Active, QPalette::ButtonText).color());
    ui->btnProjector->setPalette(palette);
  }
}

void MainWindow::on_lblTitle_linkActivated(const QString &/*link*/)
{
  on_lwText_itemClicked(new QListWidgetItem(ui->lblTitle->windowTitle())); // Взима името в чистия му вид (без html таговете за форматиране).
}
void MainWindow::on_lwText_itemActivated(QListWidgetItem *item)
{
  on_lwText_itemClicked(item);
}
void MainWindow::on_lwText_itemClicked(QListWidgetItem *item)
{
  if (item == 0) return;
  if (ui->chbDisable->isChecked()) return;

  PW->labelText->setVisible(true);
  ui->labelText->setVisible(true);

  rowsInCouplet.clear();
  if (currentCouplets.length() == 0) return;
  currentCouplet = item->text();
  rowsInCouplet = item->text().trimmed().split('\n', QString::KeepEmptyParts); // Разделя стринга на списък от редове. За разделител се взима символът за нов ред. Функцията trimmed() е изтрила последния символ за нов ред, затова се използва KeepEmptyParts - за да може да се запазят вътрешните празни редове (при сливане на куплети).

  showText();
}

void MainWindow::on_lwSongsFavourites_itemActivated(QListWidgetItem *item)
{
  on_lwSongs_itemClicked(item);
}
void MainWindow::on_lwSongsFavourites_itemClicked(QListWidgetItem *item)
{
  on_lwSongs_itemClicked(item);
}
void MainWindow::on_lwSongs_itemActivated(QListWidgetItem *item)
{
  on_lwSongs_itemClicked(item);
}
void MainWindow::on_lwSongs_itemClicked(QListWidgetItem *item)
{
  if (ui->lwSongs->count() == 0) return;
  if (item == 0) return;

  // Синхронизира двата списъка (селектира едно и също име и в двата списъка):
  bool selected = false;
  for (int i = 0; i < ui->lwSongsFavourites->count(); i++)
  {
    if (ui->lwSongsFavourites->item(i)->text() == item->text())
    {
      ui->lwSongsFavourites->setCurrentRow(i);
      selected = true;
      break;
    }
  }
  if (!selected) ui->lwSongsFavourites->setCurrentRow(-1); // Ако не е намерено такова име, премахва селекцията.
  selected = false;
  for (int i = 0; i < ui->lwSongs->count(); i++)
  {
    if (ui->lwSongs->item(i)->text() == item->text())
    {
      ui->lwSongs->setCurrentRow(i);
      selected = true;
      break;
    }
  }
  if (!selected) ui->lwSongs->setCurrentRow(-1); // Ако не е намерено такова име, премахва селекцията.

  ui->lblTitle->setWindowTitle(QFileInfo(item->text()).completeBaseName().trimmed()); // Записва заглавието (името на файла без разширението) като чист текст. Полето WindowTitle се използва за да съхранява заглавието в чистия му вид (без html таговете за форматиране).
  ui->lblTitle->setText( QString("<a style=\"text-decoration:none;\" href=\"%1\"><b><font color=\"#000000\">%2&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</font></b></a>").arg(ui->lblTitle->windowTitle()).arg(ui->lblTitle->windowTitle()) ); // Показва името на файла без разширението като линк в текстовото поле.
  loadCouplets(item->text());

  // Текст
  ui->lwText->clear();
  //ui->lwText->setSpacing(1);
  for (int i = 0; i < currentCouplets.length(); i++)
  {
    QListWidgetItem *newItem = new QListWidgetItem(currentCouplets[i]);
    newItem->setBackground(lwTextColorB);
    //newItem->setTextAlignment(Qt::AlignCenter); // Qt::AlignCenter   Qt::AlignLeft Qt::AlignRigh Qt::AlignHCenter Qt::AlignJustify    Qt::AlignTop Qt::AlignBottom Qt::AlignVCenter
    ui->lwText->addItem(newItem);
  }

  showText();
}

void MainWindow::loadCouplets(QString fileName)
{
  // Идеята на този метод е не само да зареди текста на песента (куплетите), а да направи и прочистване на текста.
  // Изчистват се както началните и крайните интервали на всеки ред, така и празните редове, ако са повече от един,
  // разположени последователно, така че да остане само по един празен ред между куплетите. Също така, премахва
  // празните редове от началото и края на текста.

  currentCouplet = "";
  rowsInCouplet.clear();
  currentCouplets.clear();
  fileName = QFileInfo(songsPath, fileName).filePath(); // Име на файла с пътя.
  QFile file(fileName);
  if (!file.exists()) return;
  QString line, couplet = "";
  QString cleanRow;
  QStringList allRows; // Съдържа всички редове от файла.
  QStringList cleanRows; // Съдържа само редовете с текст плюс най-много по един празен ред между куплетите.

  // Предварително автоматично разпознаване на енкодинга и отваряне на файла с правилния енкодинг (ако не е UTF-8 или UTF-8-BOM, взима системния енкодинг и каквото излезе - това е):
  bool isUTF8 = isUTF8TextCodec(fileName);
  file.open(QIODevice::ReadOnly | QIODevice::Text);
  QTextStream in(&file);
  if (isUTF8)
    in.setCodec(QTextCodec::codecForLocale()); // В main.cpp това е установено на UTF-8.
  else
    in.setCodec(QTextCodec::codecForName("System")); // Взима системния енкодинг.
  in.setAutoDetectUnicode(true); // Това май е излишно. Без значение какъв кодек е избран по-горе, това може да разпознае UTF-8-BOM, но не и стандартния UTF-8 (без BOM байтовете).

  // Чете всички редове от файла:
  do
  {
    line = in.readLine();
    cleanRow = line.trimmed(); // Изчиства началните и крайни интервали.
    if (!line.isEmpty() && cleanRow.isEmpty()) // Ако преди почистването на реда в него има нещо, а след почистването е празен, значи че е съдържал само интервал(и). Приема се, че такъв ред не е разделител, а е част от куплета.
      allRows << " "; // Добавя се ред с един интервал в него и той ще бъде част от куплета.
    else
      allRows << cleanRow; // Добавя изчистения ред, който може да бъде и празен, ако е разделител между куплетите.
  }
  while (!line.isNull());
  file.close();

  // Прочиства празните редове:
  cleanRows << ""; // Прибавя един празен ред в началото (за да се опрости долната логика).
  for (int i = 0; i < allRows.count(); i++)
  {
    if (allRows[i].isEmpty()) // Ако редът е празен. Ред, съдържащ един или повече интервали не се счита за празен, а е част от куплета.
    {
      if (!cleanRows[cleanRows.count()-1].isEmpty()) cleanRows << allRows[i]; // Ако последният е бил празен, не добавя новия, за да не се получат два последователни празни редове.
    }
    else // Ако редът не е празен (има текст или поне един интервал).
    {
      cleanRows << allRows[i];
    }
  }
  cleanRows.removeAt(0); // Премахва празния ред в началото.

  for (int i = 0; i < cleanRows.count(); i++)
  {
    if (!cleanRows[i].isEmpty()) // Ако искаме два куплета да се показват едновременно, празния ред между тях (във файла) трябва да съдържа някакъв знак.
    {
      couplet = couplet + cleanRows[i]/*.trimmed()*/ + "\n"; // Това ще постави и един празен ред след куплета.
    }
    else
    {
      //couplet =  "\n" + couplet; // Това ще постави един празен ред над куплета, но така става много голямо разстоянието между куплетите.
      currentCouplets.append(couplet);
      couplet = "";
    }
  }

  // Прави настройките за собствена картинка за тази песен:
  int SDPersonalWallpaperChangedH = SDPersonalWallpaperChanged; // Запомня състоянието на флага, защото в долните настройки ще се сетнат комбобоксовете и ще вдигнат този флаг, а не трябва.
  int currentIndex = ui->lwSongs->currentRow();
  if (currentIndex < 0) currentIndex = 0; // Някаква крайна мярка.
  ui->cbTabSongsWallpaperPosition->setCurrentIndex( SD[currentIndex].personalWallpaperPosition );
  ui->cbTabSongsWallpaper->setCurrentIndex( SD[currentIndex].personalWallpaper );
  setCurrentTextsWallpaper();
  showWallpaper();
  if (SD[currentIndex].personalWallpaperPath.isEmpty())
  {
    QPixmap pixmap(16, 16);
    pixmap.fill(QColor(0,0,0,0)); // Прозрачност.
    ui->btnTabSongsWallpaper->setIcon(pixmap);
  }
  else
  {
    QPixmap pixmap(SD[currentIndex].personalWallpaperPath);
    if (!pixmap.isNull()) // Ако е картинка.
    {
      ui->btnTabSongsWallpaper->setIcon(pixmap.scaled(16, 16));
    }
    else
    {
      QPixmap pixmap(16, 16);
      pixmap.fill(QColor(0,0,0,0)); // Прозрачност.
      ui->btnTabSongsWallpaper->setIcon(pixmap);
    }
  }
  SDPersonalWallpaperChanged = SDPersonalWallpaperChangedH; // Възстановява състоянието на флага отпреди настройките.
}

void MainWindow::on_btnTextZoomIn_clicked()
{
  int ps = textFont.pointSize();
  ps++;
  textFont.setPointSize(ps);
  showText();
}

void MainWindow::on_btnTextZoomOut_clicked()
{
  int ps = textFont.pointSize();
  ps--;
  textFont.setPointSize(ps);
  showText();
}

void MainWindow::on_btnTextBold_clicked()
{
  textFont.setBold(ui->btnTextBold->isChecked());
  showText();
}

void MainWindow::on_btnTextItalic_clicked()
{
  textFont.setItalic(ui->btnTextItalic->isChecked());
  showText();
}

void MainWindow::on_btnTextUpper_clicked()
{
  textUpper = ui->btnTextUpper->isChecked();
  showText();
}

void MainWindow::on_btnTextColor_clicked()
{
  QColor color = QColorDialog::getColor(textColor, this);
  if (color.isValid())
  {
    textColor = color;
    QPixmap pixmap(16, 16);
    pixmap.fill(color);
    ui->btnTextColor->setIcon(pixmap);

    showText();
  }
}

void MainWindow::on_cbFont_currentFontChanged(const QFont &font)
{
  bool isBold = textFont.bold(); // Взима текущата настройка за Bold преди шрифта да бъде сменен.
  bool isItalic = textFont.italic(); // Взима текущата настройка за Italic преди шрифта да бъде сменен.
  textFont.setFamily(font.family());
  //TextFont = font; // Запомня новия шрифт.
  textFont.setBold(isBold); // Възстановява настройката за Bold.
  textFont.setItalic(isItalic); // Възстановява настройката за Italic.
  showText();
}

void MainWindow::on_cbTextAlignH_currentIndexChanged(int /*index*/)
{
  showText();
}

void MainWindow::on_cbTextAlignV_currentIndexChanged(int /*index*/)
{
  showText();
}

void MainWindow::on_cbTextOutline_currentIndexChanged(int index)
{
  textOutline = index;
  showText();
}

void MainWindow::on_btnTextOutlineColor_clicked()
{
  QColor color = QColorDialog::getColor(textOutlineColor, this);
  if (color.isValid())
  {
    textOutlineColor = color;
    QPixmap pixmap(16, 16);
    pixmap.fill(color);
    ui->btnTextOutlineColor->setIcon(pixmap);

    showText();
  }
}

void MainWindow::on_cbTitleBarPosition_currentIndexChanged(int index)
{
  if (index == 0) // Left
  {
    textArea.setX(20 + titlePanelHeight);
    textArea.setY(15);
    textArea.setWidth(projectorScreenWidth - textArea.x() - 20); // Това е максималната дължина на един ред. По нея ще се определя колко думи да влязат в един ред.
    textArea.setHeight(projectorScreenHeight - textArea.y() - 15); // Това е височината на целия текст. По нея ще се пресмята разстоянието между редовете, така че да се съберат всички.
  }
  else if (index == 2) // Right
  {
    textArea.setX(20);
    textArea.setY(15);
    textArea.setWidth(projectorScreenWidth - textArea.x() - 20 - titlePanelHeight); // Това е максималната дължина на един ред. По нея ще се определя колко думи да влязат в един ред.
    textArea.setHeight(projectorScreenHeight - textArea.y() - 15); // Това е височината на целия текст. По нея ще се пресмята разстоянието между редовете, така че да се съберат всички.
  }
  else if (index == 1) // Top
  {
    textArea.setX(20);
    textArea.setY(15 + titlePanelHeight);
    textArea.setWidth(projectorScreenWidth - textArea.x() - 20); // Това е максималната дължина на един ред. По нея ще се определя колко думи да влязат в един ред.
    textArea.setHeight(projectorScreenHeight - textArea.y() - 15); // Това е височината на целия текст. По нея ще се пресмята разстоянието между редовете, така че да се съберат всички.
  }
  else if (index == 3) // Bottom
  {
    textArea.setX(20);
    textArea.setY(15);
    textArea.setWidth(projectorScreenWidth - textArea.x() - 20); // Това е максималната дължина на един ред. По нея ще се определя колко думи да влязат в един ред.
    textArea.setHeight(projectorScreenHeight - textArea.y() - 15 - titlePanelHeight); // Това е височината на целия текст. По нея ще се пресмята разстоянието между редовете, така че да се съберат всички.
  }
  else
  {
    textArea.setX(20);
    textArea.setY(15);
    textArea.setWidth(projectorScreenWidth - textArea.x() - 20); // Това е максималната дължина на един ред. По нея ще се определя колко думи да влязат в един ред.
    textArea.setHeight(projectorScreenHeight - textArea.y() - 15); // Това е височината на целия текст. По нея ще се пресмята разстоянието между редовете, така че да се съберат всички.
  }
  showText();
}

void MainWindow::on_cbTitleBarAlpha_currentIndexChanged(int /*index*/)
{
  showText();
}

void MainWindow::on_cbWallpaperPosition_currentIndexChanged(int index)
{
  if (customWallpaperPreviewScreen.isNull()) return; // Това е важно при първоначално инициализиране на комбобокса.

  if (index == 0) // Fit
  {
    customWallpaperPreviewScreen = scalePixmapFit(false, false, previewScreenWidth, previewScreenHeight, wallpaperFile);
    customWallpaperProjectorScreen = scalePixmapFit(false, false, projectorScreenWidth, projectorScreenHeight, wallpaperFile);
  }
  else if (index == 1) // Fit Expanding
  {
    customWallpaperPreviewScreen = scalePixmapFit(true, false, previewScreenWidth, previewScreenHeight, wallpaperFile);
    customWallpaperProjectorScreen = scalePixmapFit(true, false, projectorScreenWidth, projectorScreenHeight, wallpaperFile);
  }
  else if (index == 2) // Stretch
  {
    customWallpaperPreviewScreen = scalePixmapStretch(previewScreenWidth, previewScreenHeight, wallpaperFile);
    customWallpaperProjectorScreen = scalePixmapStretch(projectorScreenWidth, projectorScreenHeight, wallpaperFile);
  }
  else if (index == 3) // 1:1
  {
    customWallpaperPreviewScreen = scalePixmapCenter(true, wallpaperFile);
    customWallpaperProjectorScreen = scalePixmapCenter(false, wallpaperFile);
  }

  if (ui->cbWallpaper->currentIndex() == 1) // Custom Wallpaper.
  {
    currentWallpaperProjectorScreen = customWallpaperProjectorScreen;
    currentWallpaperPreviewScreen = customWallpaperPreviewScreen;
    showWallpaper();
  }
}

void MainWindow::on_cbWallpaper_currentIndexChanged(int index)
{
  if (index == 0) // Embedded Wallpaper.
  {
    ui->btnTabPictures1Auto->setChecked(false); // Спира автоматичното показване на картинки, ако е пуснато.
    ui->btnTabPictures2Auto->setChecked(false); // Спира автоматичното показване на картинки, ако е пуснато.

    currentWallpaperProjectorScreen = embeddedWallpaperProjectorScreen;
    currentWallpaperPreviewScreen = embeddedWallpaperPreviewScreen;
    ui->cbWallpaperPosition->setEnabled(false);
    ui->btnWallpaperEmbedded->setVisible(true);
    ui->btnWallpaperCustom->setVisible(false);
    ui->btnWallpaperColor->setVisible(false);
    showWallpaper();

    circleWidgetPreview->setCircleBrightness(embeddedWallpaperCircleBrightness);
    circleWidgetProjector->setCircleBrightness(embeddedWallpaperCircleBrightness);
    circleWidgetPreview->enableCircles(true);
    circleWidgetProjector->enableCircles(true);
  }
  else if (index == 1) // Custom Wallpaper.
  {
    ui->btnTabPictures1Auto->setChecked(false); // Спира автоматичното показване на картинки, ако е пуснато.
    ui->btnTabPictures2Auto->setChecked(false); // Спира автоматичното показване на картинки, ако е пуснато.

    if (ui->tabWidget->currentIndex() == 0 && SD.size() > 0 && ui->lwSongs->currentRow() >= 0 && SD[ui->lwSongs->currentRow()].personalWallpaper == 1) // Ако е избран табът с песните и текущата песен има собствена картинка.
    {
      on_cbTabSongsWallpaperPosition_currentIndexChanged(SD[ui->lwSongs->currentRow()].personalWallpaperPosition); // Тук ще зареди собствената картинка.
    }
    else // Ако е избран табът с песните, но текущата песен няма собствена картинка или ако е избран друг таб.
    {
      currentWallpaperProjectorScreen = customWallpaperProjectorScreen;
      currentWallpaperPreviewScreen = customWallpaperPreviewScreen;
    }
    ui->cbWallpaperPosition->setEnabled(true);
    ui->btnWallpaperEmbedded->setVisible(false);
    ui->btnWallpaperCustom->setVisible(true);
    ui->btnWallpaperColor->setVisible(false);
    showWallpaper();

    circleWidgetPreview->enableCircles(false);
    circleWidgetProjector->enableCircles(false);
  }
  else if (index == 2) // From Pictures 1,2.
  {
    currentWallpaperProjectorScreen = colorWallpaperProjectorScreen;
    currentWallpaperPreviewScreen = colorWallpaperPreviewScreen;
    ui->cbWallpaperPosition->setEnabled(false);
    ui->btnWallpaperEmbedded->setVisible(false);
    ui->btnWallpaperCustom->setVisible(false);
    ui->btnWallpaperColor->setVisible(false);
    showWallpaper();

    circleWidgetPreview->enableCircles(false);
    circleWidgetProjector->enableCircles(false);
  }
  else if (index == 3) // Color.
  {
    ui->btnTabPictures1Auto->setChecked(false); // Спира автоматичното показване на картинки, ако е пуснато.
    ui->btnTabPictures2Auto->setChecked(false); // Спира автоматичното показване на картинки, ако е пуснато.

    currentWallpaperProjectorScreen = colorWallpaperProjectorScreen;
    currentWallpaperPreviewScreen = colorWallpaperPreviewScreen;
    ui->cbWallpaperPosition->setEnabled(false);
    ui->btnWallpaperEmbedded->setVisible(false);
    ui->btnWallpaperCustom->setVisible(false);
    ui->btnWallpaperColor->setVisible(true);
    showWallpaper();

    circleWidgetPreview->enableCircles(false);
    circleWidgetProjector->enableCircles(false);
  }
}

void MainWindow::on_btnWallpaperEmbedded_clicked()
{
  embeddedWallpaperCircleBrightness += 10;
  if (embeddedWallpaperCircleBrightness > 120) embeddedWallpaperCircleBrightness = 20;
  ui->btnWallpaperEmbedded->setText(QString("%1").arg(embeddedWallpaperCircleBrightness));
  
  circleWidgetPreview->setCircleBrightness(embeddedWallpaperCircleBrightness);
  circleWidgetProjector->setCircleBrightness(embeddedWallpaperCircleBrightness);
}

void MainWindow::on_btnWallpaperCustom_clicked()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("Wallpaper"), wallpapersPath, QString("Wallpaper (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm *.tiff *.mng *.svg *.tga);;All Files (*.*)"));
  if (!fileName.isEmpty())
  {
    QPixmap pixmap(fileName);
    
    if (!pixmap.isNull()) // Ако е картинка.
    {
      wallpapersPath = QFileInfo(fileName).absolutePath(); // Запомня текущата директория.
      wallpaperFile = fileName; // Запомня файла.

      // Преоразмерява картинката, за да е готова за употреба.
      customWallpaperProjectorScreen = pixmap.scaled(projectorScreenWidth, projectorScreenHeight);
      customWallpaperPreviewScreen = pixmap.scaled(previewScreenWidth, previewScreenHeight);
      ui->btnWallpaperCustom->setIcon(pixmap.scaled(16, 16));

      if (ui->cbWallpaper->currentIndex() == 1)
      {
        on_cbWallpaper_currentIndexChanged(1);
      }
    }
    else
    {
      QMessageBox::information(this, QCoreApplication::applicationName(), tr("File Read Error!"));
    }
  }
}

void MainWindow::on_btnWallpaperColor_clicked()
{
  QColor color = QColorDialog::getColor(wallpaperColor, this);
  if (color.isValid())
  {
    wallpaperColor = color;

    QPixmap pmcolorPj(projectorScreenWidth, projectorScreenHeight);
    QPixmap pmcolorPv(previewScreenWidth, previewScreenHeight);
    pmcolorPj.fill(wallpaperColor);
    pmcolorPv.fill(wallpaperColor);
    colorWallpaperProjectorScreen = pmcolorPj;
    colorWallpaperPreviewScreen = pmcolorPv;

    QPixmap pmColor(16, 16);
    pmColor.fill(wallpaperColor);
    ui->btnWallpaperColor->setIcon(pmColor);

    if (ui->cbWallpaper->currentIndex() == 3)
    {
      on_cbWallpaper_currentIndexChanged(3);
    }
    else if (ui->cbWallpaper->currentIndex() == 4)
    {
      on_cbWallpaper_currentIndexChanged(4);
    }
  }
}

void MainWindow::on_btnAbout_clicked()
{
  // Това е нещо като помощна информация (About/Help), но май няма какво да се напише освен версията на програмата, затова този стринг не се превежда (използва се QString, а не tr)...
  QMessageBox::about(this, QCoreApplication::applicationName(), QString("<p><b>%1</b><br />Ver %2</p><p><a href=\"http://www.matcraft.org\">www.matcraft.org</a></p>").arg(QCoreApplication::applicationName()).arg(QCoreApplication::applicationVersion()));
  //QMessageBox::about(this, QCoreApplication::applicationName(), tr("%1 %2\nwww.matcraft.org\n\n\nYou need to use UTF-8 character encoding for the text files.").arg(QCoreApplication::applicationName()).arg(QCoreApplication::applicationVersion()));
}

void MainWindow::on_btnQt_clicked()
{
  qApp->aboutQt(); // Отваря стандартен прозорец с версията на Qt библиотеката която се ползва. Добре е такъв прозорец да има във всяко Qt приложение, за да се казва на програмиста коя е текущата версия на Qt в случай, че потребителят има проблем. Освен това, при Open-source Qt, това май е задължително.
}

void MainWindow::on_chbTabSongsShowTitle_clicked()
{
  showText();
}

void MainWindow::on_btnTabSongsAddToFavourites_clicked()
{
  if (ui->lwSongs->currentItem() == 0) return;
  ui->lwSongsFavourites->addItem( ui->lwSongs->currentItem()->text() );
}

void MainWindow::on_btnTabSongsDeleteFromFavourites_clicked()
{
  if (ui->lwSongsFavourites->currentItem() == 0) return;
  int currentItem = ui->lwSongsFavourites->currentRow();
  ui->lwSongsFavourites->takeItem(currentItem);
}

void MainWindow::on_btnTabSongsFilterSearchInText_clicked()
{
  on_leTabSongsFilter_textChanged(ui->leTabSongsFilter->text());
}

void MainWindow::on_leTabSongsFilter_textChanged(const QString &text)
{
  QString current = "~~~"; // Такъв елемент няма да има.
  if (ui->lwSongs->currentItem() != 0) current = ui->lwSongs->currentItem()->text(); // Запомня текущото име, за да го селектира отново, ако го има във филтрирания списък.

  ui->lwSongs->clear();

  if (text.isEmpty())
  {
    for (int i = 0; i < SD.count(); i++)
    {
      ui->lwSongs->addItem(SD[i].fileName);
    }

    QPalette palette = ui->lwSongs->palette();
    palette.setBrush(QPalette::Text, ui->lwSongsFavourites->palette().brush(QPalette::Text)); // Взима цвета на текста от другия списък.
    ui->lwSongs->setPalette(palette);
  }
  else
  {
    QStringList searchWordsList = text.split(" ", QString::SkipEmptyParts);
    int c = 0;

    if (!ui->btnTabSongsFilterSearchInText->isChecked()) // Търсене в имената.
    {
      for (int i = 0; i < SD.count(); i++)
      {
        c = 0;
        foreach (QString word, searchWordsList)
        {
          if ( SD[i].fileName.contains(word, Qt::CaseInsensitive) )
            c++; // Указва, че тази дума се съдържа.
          else
            break; // Ако не съдържа тази дума, няма смисъл да се проверява за останалите.
        }
        if (c == searchWordsList.count()) // Ако името съдържа всички търсени думи.
        {
          ui->lwSongs->addItem(SD[i].fileName);
        }
      }

      QPalette palette = ui->lwSongs->palette();
      palette.setBrush(QPalette::Text, QColor(85, 170, 255));
      ui->lwSongs->setPalette(palette);
    }
    else // Търсене в текста на песните.
    {
      QFile file;
      QString allText;

      for (int i = 0; i < SD.count(); i++)
      {
        file.setFileName(SD[i].filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
          QTextStream in(&file);
          if (SD[i].UTF8TextCodec)
            in.setCodec(QTextCodec::codecForLocale()); // В main.cpp това е установено на UTF-8.
          else
            in.setCodec(QTextCodec::codecForName("System")); // Взима системния енкодинг.
          in.setAutoDetectUnicode(true); // Това май е излишно. Без значение какъв кодек е избран по-горе, това може да разпознае UTF-8-BOM, но не и стандартния UTF-8 (без BOM байтовете).
          allText = in.readAll();
          file.close();

          c = 0;
          foreach (QString word, searchWordsList)
          {
            if ( allText.contains(word, Qt::CaseInsensitive) )
              c++; // Указва, че тази дума се съдържа.
            else
              break; // Ако не съдържа тази дума, няма смисъл да се проверява за останалите.
          }
          if (c == searchWordsList.count()) // Ако името съдържа всички търсени думи.
          {
            ui->lwSongs->addItem(SD[i].fileName);
          }
        }
      }

      QPalette palette = ui->lwSongs->palette();
      palette.setBrush(QPalette::Text, QColor(0, 170, 0));
      ui->lwSongs->setPalette(palette);
    }
  }

  // Селектира запомненото име, ако го има:
  QList<QListWidgetItem *> listWidgets = ui->lwSongs->findItems(current, Qt::MatchCaseSensitive);
  if (listWidgets.count() > 0)
  {
    ui->lwSongs->setCurrentItem(listWidgets[0]);
  }
  else
  {
    if (ui->lwSongs->count() > 0) ui->lwSongs->setCurrentRow(0);
  }
}

void MainWindow::on_btnTabSongsClearFilter_clicked()
{
  ui->leTabSongsFilter->clear();
}

void MainWindow::on_btnTabSongsFolder_clicked()
{
  if (SDPersonalWallpaperChanged) savePersonalWallpaperSet(); // Първо записва текущите настройки за собствените картинки за фона на песните (но само ако са били направени някакви промени).
  
  QString folder = QFileDialog::getExistingDirectory(this, tr("Open Directory"), songsPath, QFileDialog::DontUseNativeDialog); // ...However, the native Windows file dialog does not support displaying files in the directory chooser. You need to pass DontUseNativeDialog to display files using a QFileDialog.
  if (folder == "") return;
  songsPath = folder;
  loadSongs(true);
}

void MainWindow::loadSongs(bool refreshFavourites)
{
  // Зарежда списъка с имена на файловете с песните.
  ui->leTabSongsFilter->clear(); // Това трябва да е първи ред, за да работи всичко коректно.
  ui->lblTitle->setText("");
  if (refreshFavourites) ui->lwText->clear();
  ui->lwSongs->clear();
  SD.clear();
  setCurrentTextsWallpaper(); // Това ще инициализира фона, защото може да е бил собствена картинка на някоя песен.
  showWallpaper();

  SongData sData;
  QStringList allFileNames = QDir(songsPath).entryList(QStringList() << "*.txt", QDir::Files, QDir::IgnoreCase); // QDir::Name QDir::IgnoreCase QDir::Time QDir::Unsorted
  for (int i = 0; i < allFileNames.count(); i++)
  {
    //if (allFileNames[i].toLower().contains(".txt")) // Това е излишно, защото горния списък съдържа само .txt файлове.
    //{
      sData.filePath = "";
      sData.fileName = allFileNames[i];
      sData.personalWallpaperPath = "";
      sData.personalWallpaper = 0;
      sData.personalWallpaperPosition = 1; // 1 - Fit Ex
      sData.UTF8TextCodec = true;
      SD << sData;
    //}
  }
  for (int i = 0; i < SD.count(); i++)
  {
    QFileInfo fi(songsPath, SD[i].fileName);
    if (!SD[i].fileName.isEmpty()) // Ако файлът съществува.
    {
      SD[i].filePath = fi.filePath(); // Име на файла с пътя.
      SD[i].UTF8TextCodec = isUTF8TextCodec(fi.filePath());
      ui->lwSongs->addItem(SD[i].fileName);
    }
  }
  if (ui->lwSongs->count() > 0) ui->lwSongs->setCurrentRow(0);

  if (refreshFavourites)
  {
    QStringList songsFavourites; // Списък с фаворитите в момента.
    for (int i = 0; i < ui->lwSongsFavourites->count(); i++)
    {
      songsFavourites << ui->lwSongsFavourites->item(i)->text();
    }
    ui->lwSongsFavourites->clear();
    for (int i = 0; i < songsFavourites.count(); i++)
    {
      bool exists = false;
      for (int c = 0; c < SD.count(); c++)
      {
        if (SD[c].fileName == songsFavourites[i])
        {
          exists = true;
          break;
        }
      }
      if (exists) ui->lwSongsFavourites->addItem(songsFavourites[i]); // Ако фаворитът съществува като файл и в новоизбраната директория, остава в списъка с фаворитите.
    }
  }
  
  // Зарежда информация за собствените картинки, за песните, които имат зададени собствени картинки:
  // Файлът с тази информация се намира в директорията с песните и се казва .MatSongProjector.PersonalSongsWallpaper
  // Всеки ред от него съдържа следната информация:
  //
  // Име на файла на песента без пътя^Име на файла на картинката с пътя^Индексът на комбобокса за позицията на картинката^Флаг указващ дали се използва собствената картинка
  //
  // Песните, за които липсва такъв ред, нямат собствена картинка. Ако за дадена песен никога не е задавана собствена картинка,
  // тя няма да има ред в този файл. Ако обаче някога е зададена собствена картинка, тя ще се помни винаги в този файл, а дали
  // ще се използва или не се разбира от флага.
  QFileInfo fi(songsPath, ".MatSongProjector.PersonalSongsWallpaper");
  QFile file( fi.filePath() ); // Име на файла с пътя.
  if (file.exists()) // Ако файлът съществува.
  {
    SDPersonalWallpaperChanged = false;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream in(&file);
      //in.setCodec(QTextCodec::codecForLocale()); // В main.cpp това е установено на UTF-8.
      QString line;
      QStringList atts;
      do
      {
        line = in.readLine().trimmed();
        if (!line.isEmpty())
        {
          atts = line.split("^", QString::KeepEmptyParts);
          if (atts.size() == 4)
          {
            atts[0] = atts[0].trimmed();
            atts[1] = atts[1].trimmed();
            atts[2] = atts[2].trimmed();
            atts[3] = atts[3].trimmed();
            if (atts[0].isEmpty() || atts[1].isEmpty())
            {
              SDPersonalWallpaperChanged = true;
            }
            else
            {
              int index = -1;
              for (int i = 0; i < SD.count(); i++)
              {
                if (SD[i].fileName == atts[0])
                {
                  index = i;
                  break;
                }
              }

              if (index >= 0 && QFile(atts[1]).exists()) // Ако е намерена такава песен в списъка и ако файлът с картинката за нея съществува реално на диска.
              {
                SD[index].personalWallpaperPath = atts[1];

                if (atts[2] != "0" && atts[2] != "1" && atts[2] != "2" && atts[2] != "3") SDPersonalWallpaperChanged = true;
                SD[index].personalWallpaperPosition = atts[2].toUShort();   if (SD[index].personalWallpaperPosition > 3) SD[index].personalWallpaperPosition = 1; // 1 - Fit Ex

                if (atts[3] != "0" && atts[3] != "1") SDPersonalWallpaperChanged = true;
                SD[index].personalWallpaper = (atts[3] == "1") ? 1 : 0;
              }
              else
              {
                SDPersonalWallpaperChanged = true;
              }
            }
          }
          else // Ако редът не е в очаквания формат.
          {
            SDPersonalWallpaperChanged = true;
          }
        }
      }
      while (!line.isNull());
      file.close();

    }
    else
    {
      SDPersonalWallpaperChanged = true; // Ако има проблем при отварянето на файла, който съществува. Това ще доведе до опит за записването му отново, по-долу.
    }
  }
  
  if (SDPersonalWallpaperChanged) savePersonalWallpaperSet(); // Ако по време на зареждането на настройките от файла е възникнала някаква грешка или има информация за песен, която не съществува вече, това ще запише файла наново, актуален.
}

void MainWindow::on_btnTabSongsFile_clicked()
{
  ui->frameCustomWallpaper->setVisible(ui->btnTabSongsFile->isChecked());
  ui->frameSongsFile->setVisible(ui->btnTabSongsFile->isChecked());
  //ui->lwSongs->setFocus(); // Това причинява дразнещо премигване.
}

void MainWindow::on_cbTabSongsWallpaper_currentIndexChanged(int index)
{
  int currentIndex = ui->lwSongs->currentRow();
  if (currentIndex < 0) return;
  SD[currentIndex].personalWallpaper = index;
  SDPersonalWallpaperChanged = true;

  if (index == 0) // Common Wallpaper.
  {
    ui->btnTabPictures1Auto->setChecked(false); // Спира автоматичното показване на картинки, ако е пуснато.
    ui->btnTabPictures2Auto->setChecked(false); // Спира автоматичното показване на картинки, ако е пуснато.

    ui->cbTabSongsWallpaperPosition->setEnabled(false);
    ui->btnTabSongsWallpaper->setEnabled(false);
    
    if (ui->cbWallpaper->currentIndex() == 1) // Custom Wallpaper.
    {
      currentWallpaperProjectorScreen = customWallpaperProjectorScreen;
      currentWallpaperPreviewScreen = customWallpaperPreviewScreen;
      showWallpaper();
    }
  }
  else if (index == 1) // Personal Wallpaper.
  {
    ui->cbTabSongsWallpaperPosition->setEnabled(true);
    ui->btnTabSongsWallpaper->setEnabled(true);
    
    if (ui->cbWallpaper->currentIndex() == 1) // Custom Wallpaper.
    {
      on_cbTabSongsWallpaperPosition_currentIndexChanged(SD[currentIndex].personalWallpaperPosition); // Извиква този метод тук, за да не се повтаря кода.
    }
  }
}

void MainWindow::on_cbTabSongsWallpaperPosition_currentIndexChanged(int index)
{
  int currentIndex = ui->lwSongs->currentRow();
  if (currentIndex < 0) return;
  SD[currentIndex].personalWallpaperPosition = index;
  SDPersonalWallpaperChanged = true;

  if (ui->cbWallpaper->currentIndex() == 1 && ui->cbTabSongsWallpaper->currentIndex() == 1) // Custom Wallpaper.
  {
    if (currentWallpaperPreviewScreen.isNull()) return; // Това е важно при първоначално инициализиране на комбобокса.

    ui->btnTabPictures1Auto->setChecked(false); // Спира автоматичното показване на картинки, ако е пуснато.
    ui->btnTabPictures2Auto->setChecked(false); // Спира автоматичното показване на картинки, ако е пуснато.

    if (index == 0) // Fit
    {
      currentWallpaperPreviewScreen = scalePixmapFit(false, false, previewScreenWidth, previewScreenHeight, SD[currentIndex].personalWallpaperPath);
      currentWallpaperProjectorScreen = scalePixmapFit(false, false, projectorScreenWidth, projectorScreenHeight, SD[currentIndex].personalWallpaperPath);
    }
    else if (index == 1) // Fit Expanding
    {
      currentWallpaperPreviewScreen = scalePixmapFit(true, false, previewScreenWidth, previewScreenHeight, SD[currentIndex].personalWallpaperPath);
      currentWallpaperProjectorScreen = scalePixmapFit(true, false, projectorScreenWidth, projectorScreenHeight, SD[currentIndex].personalWallpaperPath);
    }
    else if (index == 2) // Stretch
    {
      currentWallpaperPreviewScreen = scalePixmapStretch(previewScreenWidth, previewScreenHeight, SD[currentIndex].personalWallpaperPath);
      currentWallpaperProjectorScreen = scalePixmapStretch(projectorScreenWidth, projectorScreenHeight, SD[currentIndex].personalWallpaperPath);
    }
    else if (index == 3) // 1:1
    {
      currentWallpaperPreviewScreen = scalePixmapCenter(true, SD[currentIndex].personalWallpaperPath);
      currentWallpaperProjectorScreen = scalePixmapCenter(false, SD[currentIndex].personalWallpaperPath);
    }

    showWallpaper();
  }
}

void MainWindow::on_btnTabSongsWallpaper_clicked()
{
  int currentIndex = ui->lwSongs->currentRow();
  if (currentIndex < 0) return;
  QString fileName = QFileDialog::getOpenFileName(this, tr("Wallpaper"), wallpapersPath, QString("Wallpaper (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm *.tiff *.mng *.svg *.tga);;All Files (*.*)"));
  if (!fileName.isEmpty())
  {
    QPixmap pixmap(fileName);
    
    if (!pixmap.isNull()) // Ако е картинка.
    {
      SD[currentIndex].personalWallpaperPath = fileName; // Запомня файла.

      ui->btnTabSongsWallpaper->setIcon(pixmap.scaled(16, 16));

      on_cbTabSongsWallpaperPosition_currentIndexChanged(SD[currentIndex].personalWallpaperPosition); // Извиква този метод тук, за да не се повтаря кода.
    }
    else
    {
      QMessageBox::information(this, QCoreApplication::applicationName(), tr("File Read Error!"));
    }
  }
}

void MainWindow::on_btnTabSongsNew_clicked()
{
  bool ok;
  QString fileName = QInputDialog::getText(this, tr("Create New Text File"), tr("Enter file name (without extension):"), QLineEdit::Normal, "", &ok);
  if (ok && !fileName.isEmpty())
  {
    int extIndex = fileName.lastIndexOf(".txt", -1, Qt::CaseInsensitive);
    if (extIndex == -1 || extIndex < (fileName.length()-4)) fileName += ".txt";

    QFileInfo fi(songsPath, fileName);
    QFile file( fi.filePath() ); // Име на файла с пътя.
    if (!file.exists()) // Ако файлът не съществува.
    {
      file.open(QIODevice::WriteOnly); // Създава файла и веднага го затваря.
      file.close();

      loadSongs(false);
      QList<QListWidgetItem *> listWidgets = ui->lwSongs->findItems(fi.fileName(), Qt::MatchCaseSensitive);
      if (listWidgets.count() > 0)
      {
        ui->lwSongs->setCurrentItem(listWidgets[0]);
        on_lwSongs_itemClicked(ui->lwSongs->currentItem()); // Презарежда текста.
      }
      else if (ui->lwSongs->count() > 0)
      {
        ui->lwSongs->setCurrentRow(0);
        on_lwSongs_itemClicked(ui->lwSongs->currentItem()); // Презарежда текста.
      }
    }
    else // Ако файлът съществува.
    {
      QMessageBox::information(this, QCoreApplication::applicationName(), tr("File with the name '%1' already exists!").arg(fi.fileName()));
    }
  }
}

void MainWindow::on_btnTabSongsDelete_clicked()
{
  if (ui->lwSongs->count() == 0) return; // Ако няма никакви песни в избраната директория.

  QFileInfo fi(songsPath, ui->lwSongs->currentItem()->text());

  QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("Delete File"), tr("Are you sure you want to permanently delete this file?") + QString("\n\n%1").arg(fi.fileName()), QMessageBox::Yes | QMessageBox::No);
  if (ret == QMessageBox::Yes && QFile::remove(fi.filePath())) // Ако файлът е изтрит успешно.
  {
    loadSongs(true);
    ui->lwSongsFavourites->setCurrentRow(-1); // Премахва селекцията.
    ui->lwSongs->setCurrentRow(-1); // Премахва селекцията.
    ui->lblTitle->setWindowTitle("");
    ui->lblTitle->setText("");
    ui->lwText->clear();
	on_btnTabSongsHideText_clicked(); // Скрива текста.
    setCurrentTextsWallpaper(); // Това ще инициализира фона, защото може да е бил собствена картинка на някоя песен.
    showWallpaper();
  }
}

void MainWindow::on_btnTabSongsRename_clicked()
{
  if (ui->lwSongs->count() == 0) return; // Ако няма никакви песни в избраната директория.

  QFileInfo fiOldName(songsPath, ui->lwSongs->currentItem()->text());
  bool ok;
  QString fileName = QInputDialog::getText(this, tr("Rename Text File"), tr("Enter new file name (without extension):"), QLineEdit::Normal, fiOldName.completeBaseName(), &ok);
  if (ok && !fileName.isEmpty())
  {
    int extIndex = fileName.lastIndexOf(".txt", -1, Qt::CaseInsensitive);
    if (extIndex == -1 || extIndex < (fileName.length()-4)) fileName += ".txt";

    QFileInfo fi(songsPath, fileName);
    //if (fi.fileName().toLower() == fiOldName.fileName().toLower()) return; // Ако този ред се разкоментира, няма да може да преименува файла, ако например е сменена само първата му буква от малка в главна.

    if ( QFile::rename(fiOldName.filePath(), fi.filePath()) ) // Ако не съществува файл с такова име и файлът е преименуван успешно.
    {
      SD[ui->lwSongs->currentRow()].fileName = fi.fileName();
      SDPersonalWallpaperChanged = true;
      savePersonalWallpaperSet(); // Записва текущите настройки за собствените картинки за фона на песните (вече с промененото име на файла).
      loadSongs(false);
      QList<QListWidgetItem *> listWidgets = ui->lwSongs->findItems(fi.fileName(), Qt::MatchCaseSensitive);
      if (listWidgets.count() > 0)
      {
        ui->lwSongs->setCurrentItem(listWidgets[0]);
      }
      else if (ui->lwSongs->count() > 0)
      {
        ui->lwSongs->setCurrentRow(0);
      }

      for (int i = 0; i < ui->lwSongsFavourites->count(); i++)
      {
        if (ui->lwSongsFavourites->item(i)->text() == fiOldName.fileName()) ui->lwSongsFavourites->item(i)->setText(fi.fileName());
      }

      ui->lblTitle->setWindowTitle(fi.completeBaseName().trimmed()); // Записва заглавието (името на файла без разширението), като чист текст. Полето WindowTitle се използва, за да съхранява заглавието в чистия му вид (без html таговете за форматиране).
      ui->lblTitle->setText( QString("<a style=\"text-decoration:none;\" href=\"%1\"><b><font color=\"#000000\">%2&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</font></b></a>").arg(ui->lblTitle->windowTitle()).arg(ui->lblTitle->windowTitle()) ); // Показва името на файла без разширението, като линк в текстовото поле.
    }
    else // Ако съществува файл с такова име.
    {
      QMessageBox::information(this, QCoreApplication::applicationName(), tr("File with the name '%1' already exists!").arg(fi.fileName()));
    }
  }
}

void MainWindow::on_btnTabSongsEdit_clicked()
{
  if (ui->lwSongs->count() == 0) return; // Ако няма никакви песни в избраната директория.

  ExTextWindow *pad = new ExTextWindow(QFileInfo(songsPath, ui->lwSongs->currentItem()->text()).filePath(), ui->lwSongs->currentItem()->text(), 640, 512, this);
  pad->setWindowTitle(QString("%1 - ").arg(ui->lwSongs->currentItem()->text()) + tr("Text Editor") + QString(", %2").arg(QCoreApplication::applicationName()));
  connect(pad, SIGNAL(textChanged(QString)), this, SLOT(songTextChanged(QString)));
  pad->show();
}

void MainWindow::songTextChanged(QString fileName)
{
  if (ui->lwSongs->currentItem()->text() == fileName) on_lwSongs_itemClicked(ui->lwSongs->currentItem()); // Презарежда текста.
}

void MainWindow::on_btnTabSongsHideText_clicked()
{
  rowsInCouplet.clear();
  currentCouplet = "";
  showText();
}

void MainWindow::on_btnTabSongsSizeInS_clicked()
{
  songListWidgetZoom++;

  QFont font = ui->lwSongs->font();
  font.setPointSize(songListWidgetZoom);
  ui->lwSongs->setFont(font);
  ui->lwSongsFavourites->setFont(font);
}

void MainWindow::on_btnTabSongsSizeOutS_clicked()
{
  songListWidgetZoom--;

  QFont font = ui->lwSongs->font();
  font.setPointSize(songListWidgetZoom);
  ui->lwSongs->setFont(font);
  ui->lwSongsFavourites->setFont(font);
}

void MainWindow::on_btnTabSongsSizeInT_clicked()
{
  textListWidgetZoom++;

  QFont font = ui->lwText->font();
  font.setPointSize(textListWidgetZoom);
  ui->lwText->setFont(font);
}

void MainWindow::on_btnTabSongsSizeOutT_clicked()
{
  textListWidgetZoom--;

  QFont font = ui->lwText->font();
  font.setPointSize(textListWidgetZoom);
  ui->lwText->setFont(font);
}

void MainWindow::on_btnTabPictures1Folder_clicked()
{
  QString folder = QFileDialog::getExistingDirectory(this, tr("Open Directory"), pictures1Path, QFileDialog::DontUseNativeDialog); // ...However, the native Windows file dialog does not support displaying files in the directory chooser. You need to pass DontUseNativeDialog to display files using a QFileDialog.
  if (folder == "") return;
  pictures1Path = folder;

  //QString("Wallpaper (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm *.tiff *.mng *.svg *.tga);;All Files (*.*)") );

  // Зарежда списъка с имена на файловете с картинките.
  ui->btnTabPictures1Auto->setChecked(false);
  ui->lwPictures1->clear();
  loadPictures1();
}

void MainWindow::on_cbTabPictures1Position_currentIndexChanged(int /*index*/)
{
  if (ui->lwPictures1->count() == 0) return; // Това е важно при първоначално инициализиране на комбобокса.
  if (ui->btnTabPictures1Auto->isChecked()) return;
  on_lwPictures1_itemClicked(ui->lwPictures1->currentItem()); // Презарежда текущата картинка в желаните размери.
}

void MainWindow::on_btnTabPictures1Auto_clicked()
{
  if (!ui->btnTabPictures1Auto->isChecked()) return;
  if (ui->chbDisable->isChecked() || ui->lwPictures1->count() == 0)
  {
    ui->btnTabPictures1Auto->setChecked(false);
    return;
  }

  PW->labelText->setVisible(false);
  ui->labelText->setVisible(false);
  ui->btnTabPictures2Auto->setChecked(false);

  pictures1FilesCurrentItem = ui->lwPictures1->currentRow() - 1; // Това е за да тръгне от селектираната картинка.
  if (pictures1FilesCurrentItem < -1) pictures1FilesCurrentItem = -1; // Ако още няма селектирана картинка.
  pictures1_Timer_singleShot();
}

void MainWindow::pictures1_Timer_singleShot()
{
  if (!ui->btnTabPictures1Auto->isChecked()) return;
  if (ui->lwPictures1->count() == 0)
  {
    ui->btnTabPictures1Auto->setChecked(false);
    return;
  }

  pictures1FilesCurrentItem++;
  if (pictures1FilesCurrentItem >= ui->lwPictures1->count()) pictures1FilesCurrentItem = 0;

  QFileInfo fi(pictures1Path, ui->lwPictures1->item(pictures1FilesCurrentItem)->text());

  if (ui->cbTabPictures1Position->currentIndex() == 0) // Auto (Fit or 1:1)
  {
    currentWallpaperPreviewScreen = scalePixmapFit(false, true, previewScreenWidth, previewScreenHeight, fi.filePath());
    currentWallpaperProjectorScreen = scalePixmapFit(false, true, projectorScreenWidth, projectorScreenHeight, fi.filePath());
  }
  else if (ui->cbTabPictures1Position->currentIndex() == 1) // Fit
  {
    currentWallpaperPreviewScreen = scalePixmapFit(false, false, previewScreenWidth, previewScreenHeight, fi.filePath());
    currentWallpaperProjectorScreen = scalePixmapFit(false, false, projectorScreenWidth, projectorScreenHeight, fi.filePath());
  }
  else if (ui->cbTabPictures1Position->currentIndex() == 2) // Fit Expanding
  {
    currentWallpaperPreviewScreen = scalePixmapFit(true, false, previewScreenWidth, previewScreenHeight, fi.filePath());
    currentWallpaperProjectorScreen = scalePixmapFit(true, false, projectorScreenWidth, projectorScreenHeight, fi.filePath());
  }
  else if (ui->cbTabPictures1Position->currentIndex() == 3) // Stretch
  {
    currentWallpaperPreviewScreen = scalePixmapStretch(previewScreenWidth, previewScreenHeight, fi.filePath());
    currentWallpaperProjectorScreen = scalePixmapStretch(projectorScreenWidth, projectorScreenHeight, fi.filePath());
  }
  else if (ui->cbTabPictures1Position->currentIndex() == 4) // 1:1
  {
    currentWallpaperPreviewScreen = scalePixmapCenter(true, fi.filePath());
    currentWallpaperProjectorScreen = scalePixmapCenter(false, fi.filePath());
  }
  showWallpaper();

  if (circleWidgetPreview->enabledCircles) // Ако кръговете се движат.
  {
    circleWidgetPreview->enableCircles(false);
    circleWidgetProjector->enableCircles(false);
  }

  QTimer::singleShot(ui->leTabPictures1Interval->text().toInt(), this, SLOT(pictures1_Timer_singleShot()));
}

void MainWindow::loadPictures1()
{
  QStringList extFilters;
  extFilters << "*.bmp" << "*.gif" << "*.jpg" << "*.jpeg" << "*.png" << "*.pbm" << "*.pgm" << "*.ppm" << "*.xbm" << "*.xpm" << "*.tiff" << "*.mng" << "*.svg" << "*.tga";
  QDir dir;
  dir.setSorting(QDir::Name);
  dir.setNameFilters(extFilters);
  dir.setPath(pictures1Path);

  //QStringList allFileNames = QDir(Pictures1Path).entryList(QDir::Files, QDir::Name); // QDir::Name QDir::Time QDir::Unsorted
  QStringList allFileNames = dir.entryList();

  QProgressBar *progressBar = new QProgressBar(ui->lwPictures1);
  progressBar->setMinimum(0);
  progressBar->setMaximum(allFileNames.count());
  progressBar->resize(ui->lwPictures1->size().width(), 16);
  progressBar->setAlignment(Qt::AlignHCenter);
  progressBar->setFormat(tr("Loading...")); // Loading %p%
  progressBar->show();

  for (int i = 0; i < allFileNames.count(); i++)
  {
    //if (allFileNames[i].toLower().contains(".png"))
    //{
      QFileInfo fi(pictures1Path, allFileNames[i]);
      //if (allFileNames[i].isEmpty()) // Ако файлът съществува.
      //{
        //ui->lwPictures1->addItem(QString("%1").arg(fi.fileName()));
        QIcon icon(QPixmap(fi.filePath()));
        ui->lwPictures1->addItem(new QListWidgetItem(icon, QString("%1").arg(allFileNames[i])));
        progressBar->setValue(i+1);
      //}
    //}
  }

  delete progressBar;
}

void MainWindow::on_lwPictures1_itemActivated(QListWidgetItem *item)
{
  on_lwPictures1_itemClicked(item);
}
void MainWindow::on_lwPictures1_itemClicked(QListWidgetItem *item)
{
  if (ui->lwPictures1->count() == 0) return;
  if (ui->lwPictures1->currentRow() < 0) return;
  if (item == 0) return;
  if (ui->chbDisable->isChecked()) return;

  ui->btnTabPictures1Auto->setChecked(false); // Спира автоматичното показване на картинки, ако е пуснато.
  ui->btnTabPictures2Auto->setChecked(false); // Спира автоматичното показване на картинки, ако е пуснато.
  PW->labelText->setVisible(false);
  ui->labelText->setVisible(false);

  QFileInfo fi(pictures1Path, item->text());

  if (ui->cbTabPictures1Position->currentIndex() == 0) // Auto (Fit or 1:1)
  {
    currentWallpaperPreviewScreen = scalePixmapFit(false, true, previewScreenWidth, previewScreenHeight, fi.filePath());
    currentWallpaperProjectorScreen = scalePixmapFit(false, true, projectorScreenWidth, projectorScreenHeight, fi.filePath());
  }
  else if (ui->cbTabPictures1Position->currentIndex() == 1) // Fit
  {
    currentWallpaperPreviewScreen = scalePixmapFit(false, false, previewScreenWidth, previewScreenHeight, fi.filePath());
    currentWallpaperProjectorScreen = scalePixmapFit(false, false, projectorScreenWidth, projectorScreenHeight, fi.filePath());
  }
  else if (ui->cbTabPictures1Position->currentIndex() == 2) // Fit Expanding
  {
    currentWallpaperPreviewScreen = scalePixmapFit(true, false, previewScreenWidth, previewScreenHeight, fi.filePath());
    currentWallpaperProjectorScreen = scalePixmapFit(true, false, projectorScreenWidth, projectorScreenHeight, fi.filePath());
  }
  else if (ui->cbTabPictures1Position->currentIndex() == 3) // Stretch
  {
    currentWallpaperPreviewScreen = scalePixmapStretch(previewScreenWidth, previewScreenHeight, fi.filePath());
    currentWallpaperProjectorScreen = scalePixmapStretch(projectorScreenWidth, projectorScreenHeight, fi.filePath());
  }
  else if (ui->cbTabPictures1Position->currentIndex() == 4) // 1:1
  {
    currentWallpaperPreviewScreen = scalePixmapCenter(true, fi.filePath());
    currentWallpaperProjectorScreen = scalePixmapCenter(false, fi.filePath());
  }

  showWallpaper();

  if (circleWidgetPreview->enabledCircles) // Ако кръговете се движат.
  {
    circleWidgetPreview->enableCircles(false);
    circleWidgetProjector->enableCircles(false);
  }
}

void MainWindow::on_btnTabPictures2Add_clicked()
{
  QString fileName = QFileDialog::getOpenFileName(this, tr("Wallpaper"), pictures2Path, QString("Wallpaper (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm *.tiff *.mng *.svg *.tga);;All Files (*.*)"));
  if (!fileName.isEmpty())
  {
    QPixmap pixmap(fileName);
    
    if (!pixmap.isNull()) // Ако е картинка.
    {
      pictures2Path = QFileInfo(fileName).absolutePath(); // Запомня текущата директория.
      QFileInfo fi(pictures2Path, fileName);
      pictures2Files << fi.filePath();
      Pictures2Set p2s;
      if (ui->cbTabPictures2Position->currentIndex() == 0) // Auto (Fit or 1:1)
      {
        p2s.picturesSmall = scalePixmapFit(false, true, previewScreenWidth, previewScreenHeight, fi.filePath());
        p2s.picturesBig = scalePixmapFit(false, true, projectorScreenWidth, projectorScreenHeight, fi.filePath());
      }
      else if (ui->cbTabPictures2Position->currentIndex() == 1) // Fit
      {
        p2s.picturesSmall = scalePixmapFit(false, false, previewScreenWidth, previewScreenHeight, fi.filePath());
        p2s.picturesBig = scalePixmapFit(false, false, projectorScreenWidth, projectorScreenHeight, fi.filePath());
      }
      else if (ui->cbTabPictures2Position->currentIndex() == 2) // Fit Expanding
      {
        p2s.picturesSmall = scalePixmapFit(true, false, previewScreenWidth, previewScreenHeight, fi.filePath());
        p2s.picturesBig = scalePixmapFit(true, false, projectorScreenWidth, projectorScreenHeight, fi.filePath());
      }
      else if (ui->cbTabPictures2Position->currentIndex() == 3) // Stretch
      {
        p2s.picturesSmall = scalePixmapStretch(previewScreenWidth, previewScreenHeight, fi.filePath());
        p2s.picturesBig = scalePixmapStretch(projectorScreenWidth, projectorScreenHeight, fi.filePath());
      }
      else if (ui->cbTabPictures2Position->currentIndex() == 4) // 1:1
      {
        p2s.picturesSmall = scalePixmapCenter(true, fi.filePath());
        p2s.picturesBig = scalePixmapCenter(false, fi.filePath());
      }
      P2S << p2s;

      QIcon icon(QPixmap(fi.filePath()));
      ui->lwPictures2->addItem(new QListWidgetItem(icon, QString("%1").arg(fi.fileName())));
    }
    else
    {
      QMessageBox::information(this, QCoreApplication::applicationName(), tr("File Read Error!"));
    }
  }
}

void MainWindow::on_btnTabPictures2Remove_clicked()
{
  if (ui->lwPictures2->count() == 0) return;
  if (ui->lwPictures2->currentRow() < 0) return;

  int currentItem = ui->lwPictures2->currentRow();
  if (currentItem >= pictures2Files.count()) // Ако нещо не е наред, изтрива всичко.
  {
    pictures2Files.clear();
    P2S.clear();
    ui->btnTabPictures2Auto->setChecked(false);
    ui->lwPictures2->clear();
    return;
  }

  pictures2Files.removeAt(currentItem);
  P2S.removeAt(currentItem);
  ui->lwPictures2->takeItem(currentItem);

  if (pictures2Files.count() == 0)
  {
    ui->btnTabPictures2Auto->setChecked(false);
  }
}

void MainWindow::on_cbTabPictures2Position_currentIndexChanged(int /*index*/)
{
  if (ui->lwPictures2->count() == 0) return; // Това е важно при първоначално инициализиране на комбобокса.
  int currentRow = ui->lwPictures2->currentRow();
  ui->lwPictures2->clear();
  loadPictures2(); // Презарежда картинките, за да ги направи в желаните размери.
  ui->lwPictures2->setCurrentRow(currentRow);
  if (!ui->btnTabPictures2Auto->isChecked())
  {
    on_lwPictures2_itemClicked(ui->lwPictures2->currentItem()); // Презарежда текущата картинка в желаните размери.
  }
}

void MainWindow::on_lwPictures2_itemActivated(QListWidgetItem *item)
{
  on_lwPictures2_itemClicked(item);
}
void MainWindow::on_lwPictures2_itemClicked(QListWidgetItem *item)
{
  if (ui->lwPictures2->count() == 0) return;
  if (ui->lwPictures2->currentRow() < 0) return;
  if (item == 0) return;
  if (ui->chbDisable->isChecked()) return;

  ui->btnTabPictures1Auto->setChecked(false); // Спира автоматичното показване на картинки, ако е пуснато.
  ui->btnTabPictures2Auto->setChecked(false); // Спира автоматичното показване на картинки, ако е пуснато.
  PW->labelText->setVisible(false);
  ui->labelText->setVisible(false);

  currentWallpaperPreviewScreen = P2S[ui->lwPictures2->currentRow()].picturesSmall;
  currentWallpaperProjectorScreen = P2S[ui->lwPictures2->currentRow()].picturesBig;
  showWallpaper();

  if (circleWidgetPreview->enabledCircles) // Ако кръговете се движат.
  {
    circleWidgetPreview->enableCircles(false);
    circleWidgetProjector->enableCircles(false);
  }
}

void MainWindow::on_btnTabPictures2Auto_clicked()
{
  if (!ui->btnTabPictures2Auto->isChecked()) return;
  if (ui->chbDisable->isChecked() || pictures2Files.count() == 0)
  {
    ui->btnTabPictures2Auto->setChecked(false);
    return;
  }
  PW->labelText->setVisible(false);
  ui->labelText->setVisible(false);
  ui->btnTabPictures1Auto->setChecked(false);

  pictures2FilesCurrentItem = ui->lwPictures2->currentRow() - 1; // Това е за да тръгне от селектираната картинка.
  if (pictures2FilesCurrentItem < -1) pictures2FilesCurrentItem = -1; // Ако още няма селектирана картинка.
  pictures2_Timer_singleShot();
}

void MainWindow::pictures2_Timer_singleShot()
{
  if (!ui->btnTabPictures2Auto->isChecked()) return;
  if (pictures2Files.count() == 0)
  {
    ui->btnTabPictures2Auto->setChecked(false);
    return;
  }

  pictures2FilesCurrentItem++;
  if (pictures2FilesCurrentItem >= pictures2Files.count()) pictures2FilesCurrentItem = 0;

  currentWallpaperPreviewScreen = P2S[pictures2FilesCurrentItem].picturesSmall;
  currentWallpaperProjectorScreen = P2S[pictures2FilesCurrentItem].picturesBig;
  showWallpaper();

  if (circleWidgetPreview->enabledCircles) // Ако кръговете се движат.
  {
    circleWidgetPreview->enableCircles(false);
    circleWidgetProjector->enableCircles(false);
  }

  QTimer::singleShot(ui->leTabPictures2Interval->text().toInt(), this, SLOT(pictures2_Timer_singleShot()));
}

void MainWindow::on_btnTabCustomTextSizeIn_clicked()
{
  customTextEditZoom++;

  QFont font = ui->teCustomText1->font();
  font.setPointSize(customTextEditZoom);
  ui->teCustomText1->setFont(font);
  ui->teCustomText2->setFont(font);
  ui->teCustomText3->setFont(font);
}

void MainWindow::on_btnTabCustomTextSizeOut_clicked()
{
  customTextEditZoom--;

  QFont font = ui->teCustomText1->font();
  font.setPointSize(customTextEditZoom);
  ui->teCustomText1->setFont(font);
  ui->teCustomText2->setFont(font);
  ui->teCustomText3->setFont(font);
}

void MainWindow::on_btnTabCustomTextHideText_clicked()
{
  on_btnTabSongsHideText_clicked();
}

void MainWindow::on_btnTabCustomTextShowText1_clicked()
{
  if (ui->chbDisable->isChecked()) return;

  if (ui->cbWallpaper->currentIndex() != 2) // Ако не е From Pictures 1,2.
  {
    ui->btnTabPictures1Auto->setChecked(false); // Спира автоматичното показване на картинки, ако е пуснато.
    ui->btnTabPictures2Auto->setChecked(false); // Спира автоматичното показване на картинки, ако е пуснато.
  }

  PW->labelText->setVisible(true);
  ui->labelText->setVisible(true);
  rowsInCouplet.clear();
  currentCouplet = "";

  for (QTextBlock it = ui->teCustomText1->document()->begin(); it != ui->teCustomText1->document()->end(); it = it.next())
  {
    rowsInCouplet << it.text();
  }

  if (ui->cbWallpaper->currentIndex() == 0) // Embedded Wallpaper.
  {
    if (!circleWidgetPreview->enabledCircles) // Ако кръговете не се движат.
    {
      circleWidgetPreview->setCircleBrightness(embeddedWallpaperCircleBrightness);
      circleWidgetProjector->setCircleBrightness(embeddedWallpaperCircleBrightness);
      circleWidgetPreview->enableCircles(true);
      circleWidgetProjector->enableCircles(true);
    }
  }

  showText();
}

void MainWindow::on_btnTabCustomTextShowText2_clicked()
{
  if (ui->chbDisable->isChecked()) return;

  if (ui->cbWallpaper->currentIndex() != 2) // Ако не е From Pictures 1,2.
  {
    ui->btnTabPictures1Auto->setChecked(false); // Спира автоматичното показване на картинки, ако е пуснато.
    ui->btnTabPictures2Auto->setChecked(false); // Спира автоматичното показване на картинки, ако е пуснато.
  }

  PW->labelText->setVisible(true);
  ui->labelText->setVisible(true);
  rowsInCouplet.clear();
  currentCouplet = "";

  for (QTextBlock it = ui->teCustomText2->document()->begin(); it != ui->teCustomText2->document()->end(); it = it.next())
  {
    rowsInCouplet << it.text();
  }

  if (ui->cbWallpaper->currentIndex() == 0) // Embedded Wallpaper.
  {
    if (!circleWidgetPreview->enabledCircles) // Ако кръговете не се движат.
    {
      circleWidgetPreview->setCircleBrightness(embeddedWallpaperCircleBrightness);
      circleWidgetProjector->setCircleBrightness(embeddedWallpaperCircleBrightness);
      circleWidgetPreview->enableCircles(true);
      circleWidgetProjector->enableCircles(true);
    }
  }

  showText();
}

void MainWindow::on_btnTabCustomTextShowText3_clicked()
{
  if (ui->chbDisable->isChecked()) return;

  if (ui->cbWallpaper->currentIndex() != 2) // Ако не е From Pictures 1,2.
  {
    ui->btnTabPictures1Auto->setChecked(false); // Спира автоматичното показване на картинки, ако е пуснато.
    ui->btnTabPictures2Auto->setChecked(false); // Спира автоматичното показване на картинки, ако е пуснато.
  }

  PW->labelText->setVisible(true);
  ui->labelText->setVisible(true);
  rowsInCouplet.clear();
  currentCouplet = "";

  for (QTextBlock it = ui->teCustomText3->document()->begin(); it != ui->teCustomText3->document()->end(); it = it.next())
  {
    rowsInCouplet << it.text();
  }

  if (ui->cbWallpaper->currentIndex() == 0) // Embedded Wallpaper.
  {
    if (!circleWidgetPreview->enabledCircles) // Ако кръговете не се движат.
    {
      circleWidgetPreview->setCircleBrightness(embeddedWallpaperCircleBrightness);
      circleWidgetProjector->setCircleBrightness(embeddedWallpaperCircleBrightness);
      circleWidgetPreview->enableCircles(true);
      circleWidgetProjector->enableCircles(true);
    }
  }

  showText();
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
  if (index == 0) // Ако е показан раздел "Songs"
  {
    ui->frameSongsControls->setVisible(true);
    if (ui->cbWallpaper->currentIndex() == 1 && SD.size() > 0 && ui->lwSongs->currentRow() >= 0 && SD[ui->lwSongs->currentRow()].personalWallpaper == 1) // Custom Wallpaper. Сменя фона, но само ако е в Custom Wallpaper режим и текущата песен има собствена картинка. Това се налага, защото може текущата песен да е показала собствената си картинка.
    {
      on_btnTabSongsHideText_clicked(); // Скрива текста. Това се налага при превключване между таба с песните и таба с потребителския текст и то само защото в този момент може да се смени фона и ще е странно текстът да остане. 
      setCurrentTextsWallpaper();
      showWallpaper();
    }
  }
  else if (index == 1) // Ако е показан раздел "Pictures1"
  {
    ui->frameSongsControls->setVisible(false);
    if (ui->lwPictures1->count() == 0) loadPictures1(); // Това става обикновено при първо отваряне на раздел "Pictures1".
  }
  else if (index == 2) // Ако е показан раздел "Pictures2"
  {
    ui->frameSongsControls->setVisible(false);
    if (pictures2Files.count() != ui->lwPictures2->count()) loadPictures2(); // Това става обикновено при първо отваряне на раздел "Pictures2".
  }
  else if (index == 3) // Ако е показан раздел "Custom Text"
  {
    ui->frameSongsControls->setVisible(true);
    if (ui->cbWallpaper->currentIndex() == 1 && SD.size() > 0 && ui->lwSongs->currentRow() >= 0 && SD[ui->lwSongs->currentRow()].personalWallpaper == 1) // Custom Wallpaper. Сменя фона, но само ако е в Custom Wallpaper режим и текущата песен има собствена картинка. Това се налага, защото може текущата песен да е показала собствената си картинка.
    {
      on_btnTabSongsHideText_clicked(); // Скрива текста. Това се налага при превключване между таба с песните и таба с потребителския текст и то само защото в този момент може да се смени фона и ще е странно текстът да остане. 
      setCurrentTextsWallpaper();
      showWallpaper();
    }
  }
}

void MainWindow::loadPictures2()
{
  QStringList Pictures2FilesH = pictures2Files;
  pictures2Files.clear();
  P2S.clear();

  QProgressBar *progressBar = new QProgressBar(ui->lwPictures2);
  progressBar->setMinimum(0);
  progressBar->setMaximum(Pictures2FilesH.count());
  progressBar->resize(ui->lwPictures2->size().width(), 16);
  progressBar->setAlignment(Qt::AlignHCenter);
  progressBar->setFormat(tr("Loading...")); // Loading %p%
  progressBar->show();

  for (int i = 0; i < Pictures2FilesH.count(); i++)
  {
    if (QFile(Pictures2FilesH[i]).exists())
    {
      QPixmap pixmap(Pictures2FilesH[i]);
      if (!pixmap.isNull()) // Ако е картинка.
      {
        QFileInfo fi(Pictures2FilesH[i]);
        pictures2Files << fi.filePath();

        QIcon icon(QPixmap(fi.filePath()));
        ui->lwPictures2->addItem(new QListWidgetItem(icon, QString("%1").arg(fi.fileName())));
        Pictures2Set p2s;
        if (ui->cbTabPictures2Position->currentIndex() == 0) // Auto (Fit or 1:1)
        {
          p2s.picturesSmall = scalePixmapFit(false, true, previewScreenWidth, previewScreenHeight, fi.filePath());
          p2s.picturesBig = scalePixmapFit(false, true, projectorScreenWidth, projectorScreenHeight, fi.filePath());
        }
        else if (ui->cbTabPictures2Position->currentIndex() == 1) // Fit
        {
          p2s.picturesSmall = scalePixmapFit(false, false, previewScreenWidth, previewScreenHeight, fi.filePath());
          p2s.picturesBig = scalePixmapFit(false, false, projectorScreenWidth, projectorScreenHeight, fi.filePath());
        }
        else if (ui->cbTabPictures2Position->currentIndex() == 2) // Fit Expanding
        {
          p2s.picturesSmall = scalePixmapFit(true, false, previewScreenWidth, previewScreenHeight, fi.filePath());
          p2s.picturesBig = scalePixmapFit(true, false, projectorScreenWidth, projectorScreenHeight, fi.filePath());
        }
        else if (ui->cbTabPictures2Position->currentIndex() == 3) // Stretch
        {
          p2s.picturesSmall = scalePixmapStretch(previewScreenWidth, previewScreenHeight, fi.filePath());
          p2s.picturesBig = scalePixmapStretch(projectorScreenWidth, projectorScreenHeight, fi.filePath());
        }
        else if (ui->cbTabPictures2Position->currentIndex() == 4) // 1:1
        {
          p2s.picturesSmall = scalePixmapCenter(true, fi.filePath());
          p2s.picturesBig = scalePixmapCenter(false, fi.filePath());
        }
        P2S << p2s;
        progressBar->setValue(i+1);
      }
    }
  }

  delete progressBar;
}

void MainWindow::projectorWindowMaximized()
{
  // Тази функция се извиква еднократно при първо максимизиране на прозореца за проектора.
  // Използва се за скриване на прозореца и за предварително премащабиране на картинките за фон.

  projectorScreenX = PW->geometry().x(); // rect() geometry()
  projectorScreenY = PW->geometry().y();
  projectorScreenWidth = PW->geometry().width();
  projectorScreenHeight = PW->geometry().height();

  PW->lower(); // Това ще постави на заден план екрана на проектора под другите екрани (на други програми-проектори).
  PW->setVisible(false); // Скрива прозореца за проектора.

  resizeGeometry();

#ifdef Q_OS_LINUX //Q_WS_WIN  Q_OS_LINUX  Q_OS_MAC  Q_OS_UNIX  Q_OS_WIN
  // Това е заради Линукс:
  // Ако ги няма долните редове се показва другия десктоп (Workspace) и остава на него, като в същото време прозорецът остава някак невидим. Голяма каша е в този Линукс...
  QTimer::singleShot(500, ui->btnProjector, SLOT(click())); // Симулира натискане на бутона за показване на прозореца на проектора.
  QTimer::singleShot(900, ui->btnProjector, SLOT(click())); // Симулира натискане на бутона за скриване на прозореца на проектора.
#endif
}

void MainWindow::resizeGeometry()
{
  PW->labelBlack->setGeometry(QRect(0, 0, projectorScreenWidth, projectorScreenHeight)); // PW->rect()
  PW->labelWallpaper->setGeometry(QRect(0, 0, projectorScreenWidth, projectorScreenHeight)); // PW->rect()
  PW->labelText->setGeometry(QRect(0, 0, projectorScreenWidth, projectorScreenHeight));

  // Формата на малкия екран трябва да е пропорционална на тази на проектора.
  // Това се налага, защото новите проектори са широкоекранни и е добре и малкия екран да е широкоекранен.
  // От друга страна повечето проектори работят с лаптопи, които са широкоекранни (16:9).
  // Ето защо малкия екран е с размери 19:9 (широкоекранен), а действителните размери се изчисляват по-долу.
  //
  // Mh/Mw = Gh/Gw   Mh = Mw * Gh / Gw
  // Mw/Mh = Gw/Gh   Mw = Mh * Gw / Gh
  //
  previewScreenWidth = ui->frameMiniScreen->size().width()-2;
  previewScreenHeight = previewScreenWidth * projectorScreenHeight / projectorScreenWidth;

  ui->frameMiniScreen->setMinimumSize(QSize(previewScreenWidth+2, previewScreenHeight+2));
  ui->frameMiniScreen->setMaximumSize(QSize(previewScreenWidth+2, previewScreenHeight+2));
  ui->labelBlack->resize(previewScreenWidth, previewScreenHeight);
  ui->labelWallpaper->resize(previewScreenWidth, previewScreenHeight);
  ui->labelText->resize(previewScreenWidth, previewScreenHeight);

  circleWidgetPreview->resize(previewScreenWidth, previewScreenHeight);
  circleWidgetProjector->resize(projectorScreenWidth, projectorScreenHeight);
  if (ui->cbWallpaper->currentIndex() == 0) // Embedded Wallpaper.
  {
    circleWidgetPreview->setCircleBrightness(embeddedWallpaperCircleBrightness);
    circleWidgetProjector->setCircleBrightness(embeddedWallpaperCircleBrightness);
    circleWidgetPreview->enableCircles(true);
    circleWidgetProjector->enableCircles(true);
  }

  setMinimumHeight(previewScreenHeight + 2 + ui->frameControl1->size().height() + 8);

  // Целта на следващите редове е да мащабира вградената картинка така, че да се намести на всякакъв екран.
  QPixmap embeddedPic(":/resources/images/embeddedbg_text.png"); // Това е вградената картинка, която трябва да се покаже. Ако е по-голяма ще се намали, ако е по-малка ще се увеличи. Ако не е пропорционална ще стърчи и ще е центрирана.
  embeddedWallpaperProjectorScreen = embeddedPic.scaled(projectorScreenWidth, projectorScreenHeight, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation); // Qt::FastTransformation   Qt::SmoothTransformation
  embeddedWallpaperPreviewScreen = embeddedPic.scaled(previewScreenWidth, previewScreenHeight, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation); // Qt::FastTransformation   Qt::SmoothTransformation

  // Мащабира картинките с цвят на фона, които се използват когато е избран цвят, а не картина.
  QPixmap pmcolorPj(projectorScreenWidth, projectorScreenHeight);
  QPixmap pmcolorPv(previewScreenWidth, previewScreenHeight);

  pmcolorPj.fill(wallpaperColor);
  pmcolorPv.fill(wallpaperColor);
  colorWallpaperProjectorScreen = pmcolorPj;
  colorWallpaperPreviewScreen = pmcolorPv;

  pmcolorPj.fill(QColor(0,0,0));
  pmcolorPv.fill(QColor(0,0,0));
  customWallpaperProjectorScreen = pmcolorPj; // Инициализира се с черен цвят.
  customWallpaperPreviewScreen = pmcolorPv;   // Инициализира се с черен цвят.
  if (!wallpaperFile.isEmpty())
  {
    QPixmap pmWallpaper(wallpaperFile);
    
    if (!pmWallpaper.isNull()) // Ако е картинка.
    {
      if (ui->cbWallpaperPosition->currentIndex() == 0) // Fit
      {
        customWallpaperProjectorScreen = scalePixmapFit(false, false, projectorScreenWidth, projectorScreenHeight, wallpaperFile);
        customWallpaperPreviewScreen = scalePixmapFit(false, false, previewScreenWidth, previewScreenHeight, wallpaperFile);
      }
      else if (ui->cbWallpaperPosition->currentIndex() == 1) // Fit Expanding
      {
        customWallpaperProjectorScreen = scalePixmapFit(true, false, projectorScreenWidth, projectorScreenHeight, wallpaperFile);
        customWallpaperPreviewScreen = scalePixmapFit(true, false, previewScreenWidth, previewScreenHeight, wallpaperFile);
      }
      else if (ui->cbWallpaperPosition->currentIndex() == 2) // Stretch
      {
        customWallpaperProjectorScreen = scalePixmapStretch(projectorScreenWidth, projectorScreenHeight, wallpaperFile);
        customWallpaperPreviewScreen = scalePixmapStretch(previewScreenWidth, previewScreenHeight, wallpaperFile);
      }
      else if (ui->cbWallpaperPosition->currentIndex() == 3) // 1:1
      {
        customWallpaperProjectorScreen = scalePixmapCenter(false, wallpaperFile);
        customWallpaperPreviewScreen = scalePixmapCenter(true, wallpaperFile);
      }
      ui->btnWallpaperCustom->setIcon(pmWallpaper.scaled(16, 16));
    }
  }

  setCurrentTextsWallpaper();

  PW->labelWallpaper->setPixmap(currentWallpaperProjectorScreen);
  ui->labelWallpaper->setPixmap(currentWallpaperPreviewScreen);

  if (ui->cbTitleBarPosition->currentIndex() == 0) // Left
  {
    textArea.setX(20 + titlePanelHeight);
    textArea.setY(15);
    textArea.setWidth(projectorScreenWidth - textArea.x() - 20); // Това е максималната дължина на един ред. По нея ще се определя колко думи да влязат в един ред.
    textArea.setHeight(projectorScreenHeight - textArea.y() - 15); // Това е височината на целия текст. По нея ще се пресмята разстоянието между редовете, така че да се съберат всички.
  }
  else if (ui->cbTitleBarPosition->currentIndex() == 2) // Right
  {
    textArea.setX(20);
    textArea.setY(15);
    textArea.setWidth(projectorScreenWidth - textArea.x() - 20 - titlePanelHeight); // Това е максималната дължина на един ред. По нея ще се определя колко думи да влязат в един ред.
    textArea.setHeight(projectorScreenHeight - textArea.y() - 15); // Това е височината на целия текст. По нея ще се пресмята разстоянието между редовете, така че да се съберат всички.
  }
  else if (ui->cbTitleBarPosition->currentIndex() == 1) // Top
  {
    textArea.setX(20);
    textArea.setY(15 + titlePanelHeight);
    textArea.setWidth(projectorScreenWidth - textArea.x() - 20); // Това е максималната дължина на един ред. По нея ще се определя колко думи да влязат в един ред.
    textArea.setHeight(projectorScreenHeight - textArea.y() - 15); // Това е височината на целия текст. По нея ще се пресмята разстоянието между редовете, така че да се съберат всички.
  }
  else if (ui->cbTitleBarPosition->currentIndex() == 3) // Bottom
  {
    textArea.setX(20);
    textArea.setY(15);
    textArea.setWidth(projectorScreenWidth - textArea.x() - 20); // Това е максималната дължина на един ред. По нея ще се определя колко думи да влязат в един ред.
    textArea.setHeight(projectorScreenHeight - textArea.y() - 15 - titlePanelHeight); // Това е височината на целия текст. По нея ще се пресмята разстоянието между редовете, така че да се съберат всички.
  }
  else
  {
    textArea.setX(20);
    textArea.setY(15);
    textArea.setWidth(projectorScreenWidth - textArea.x() - 20); // Това е максималната дължина на един ред. По нея ще се определя колко думи да влязат в един ред.
    textArea.setHeight(projectorScreenHeight - textArea.y() - 15); // Това е височината на целия текст. По нея ще се пресмята разстоянието между редовете, така че да се съберат всички.
  }
}

void MainWindow::setCurrentTextsWallpaper()
{
  if (ui->cbWallpaper->currentIndex() == 0)
  {
    currentWallpaperProjectorScreen = embeddedWallpaperProjectorScreen;
    currentWallpaperPreviewScreen = embeddedWallpaperPreviewScreen;
  }
  else if (ui->cbWallpaper->currentIndex() == 1)
  {
    ui->btnTabPictures1Auto->setChecked(false); // Спира автоматичното показване на картинки, ако е пуснато.
    ui->btnTabPictures2Auto->setChecked(false); // Спира автоматичното показване на картинки, ако е пуснато.
      
    if (ui->tabWidget->currentIndex() == 0 && SD.size() > 0 && ui->lwSongs->currentRow() >= 0 && SD[ui->lwSongs->currentRow()].personalWallpaper == 1) // Ако е избран табът с песните и текущата песен има собствена картинка.
    {
      int currentIndex = ui->lwSongs->currentRow();
      if (SD[currentIndex].personalWallpaperPosition == 0) // Fit
      {
        currentWallpaperProjectorScreen = scalePixmapFit(false, false, projectorScreenWidth, projectorScreenHeight, SD[currentIndex].personalWallpaperPath);
        currentWallpaperPreviewScreen = scalePixmapFit(false, false, previewScreenWidth, previewScreenHeight, SD[currentIndex].personalWallpaperPath);
      }
      else if (SD[currentIndex].personalWallpaperPosition == 1) // Fit Expanding
      {
        currentWallpaperProjectorScreen = scalePixmapFit(true, false, projectorScreenWidth, projectorScreenHeight, SD[currentIndex].personalWallpaperPath);
        currentWallpaperPreviewScreen = scalePixmapFit(true, false, previewScreenWidth, previewScreenHeight, SD[currentIndex].personalWallpaperPath);
      }
      else if (SD[currentIndex].personalWallpaperPosition == 2) // Stretch
      {
        currentWallpaperProjectorScreen = scalePixmapStretch(projectorScreenWidth, projectorScreenHeight, SD[currentIndex].personalWallpaperPath);
        currentWallpaperPreviewScreen = scalePixmapStretch(previewScreenWidth, previewScreenHeight, SD[currentIndex].personalWallpaperPath);
      }
      else if (SD[currentIndex].personalWallpaperPosition == 3) // 1:1
      {
        currentWallpaperProjectorScreen = scalePixmapCenter(false, SD[currentIndex].personalWallpaperPath);
        currentWallpaperPreviewScreen = scalePixmapCenter(true, SD[currentIndex].personalWallpaperPath);
      }
    }
    else // Ако е избран табът с песните, но текущата песен няма собствена картинка или ако е избран друг таб.
    {
      currentWallpaperProjectorScreen = customWallpaperProjectorScreen;
      currentWallpaperPreviewScreen = customWallpaperPreviewScreen;
    }
  }
  else if (ui->cbWallpaper->currentIndex() == 2)
  {
  }
  else if (ui->cbWallpaper->currentIndex() == 3)
  {
    currentWallpaperProjectorScreen = colorWallpaperProjectorScreen;
    currentWallpaperPreviewScreen = colorWallpaperPreviewScreen;
  }
  else if (ui->cbWallpaper->currentIndex() == 4)
  {
    currentWallpaperProjectorScreen = colorWallpaperProjectorScreen;
    currentWallpaperPreviewScreen = colorWallpaperPreviewScreen;
  }
}

QPixmap MainWindow::scalePixmapFit(bool expanding, bool retainsOriginalSizeIfItCan, int width, int height, QString filePath)
{
  // Закоментираният код е моя начин за изчисляване размера на картинката, така че да се вмести в екрана.
  // После открих опцията Qt::KeepAspectRatio на метода scaled и ще използвам него, но оставям моя код като идея.
  //
  //QPixmap pixmap(filePath);
  //QPixmap scaledPixmap;
  //
  //double p1 = (double)width / (double)height;
  //double p2 = (double)pixmap.width() / (double)pixmap.height();
  //
  //if (p2 > p1) // Ако картинката е пропорционално по-широка от екрана, но по височина е по-малка.
  //{
  //  scaledPixmap = pixmap.scaledToWidth(width, Qt::FastTransformation); // Qt::SmoothTransformation
  //}
  //else // Ако картинката е пропорционално по-висока от екрана, но по широчина е по-малка.
  //{
  //  scaledPixmap = pixmap.scaledToHeight(height, Qt::FastTransformation); // Qt::SmoothTransformation
  //}
  //
  //return scaledPixmap;
  QPixmap pixmap(filePath);
  if (expanding)
  {
    return pixmap.scaled(width, height, Qt::KeepAspectRatioByExpanding, Qt::FastTransformation); // Qt::SmoothTransformation
  }
  else if (retainsOriginalSizeIfItCan && pixmap.width() <= projectorScreenWidth && pixmap.height() <= projectorScreenHeight)
  {
    if (width == previewScreenWidth) // За малкия екран - картинката се намаля, за да е пропорционална на тази на проектора.
    {
      int width = (int)( (double)previewScreenWidth / (double)projectorScreenWidth * (double)pixmap.width() );
      return pixmap.scaledToWidth(width, Qt::FastTransformation); // Qt::SmoothTransformation
    }
    else // За екрана на проектора - картинката се използва 1 към 1.
    {
      return pixmap;
    }
  }
  else
  {
    return pixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::FastTransformation); // Qt::SmoothTransformation
  }
}

QPixmap MainWindow::scalePixmapStretch(int width, int height, QString filePath)
{
  QPixmap pixmap(filePath);
  return pixmap.scaled(width, height, Qt::IgnoreAspectRatio, Qt::FastTransformation); // Qt::SmoothTransformation
}

QPixmap MainWindow::scalePixmapCenter(bool preview, QString filePath)
{
  QPixmap pixmap(filePath);
  if (preview) // За малкия екран - картинката се намаля за да е пропорционална на тази на проектора.
  {
    int width = (int)( (double)previewScreenWidth / (double)projectorScreenWidth * (double)pixmap.width() );
    return pixmap.scaledToWidth(width, Qt::FastTransformation); // Qt::SmoothTransformation
  }
  else // За екрана на проектора - картинката се използва 1 към 1.
  {
    return pixmap;
  }
}

void MainWindow::resizeText()
{
  QFontMetrics fm(textFontH);
  lineSpacing = fm.lineSpacing();
  int lineWidth;
  int maxLineWidth = 0;

  // Определя се дължината на най-дългия ред:
  for (int i = 0; i < rowsInCoupletH.count(); i++)
  {
    lineWidth = fm.width(rowsInCoupletH[i]);
    if (maxLineWidth < lineWidth) maxLineWidth = lineWidth;
  }

  if ( maxLineWidth > textArea.width() && textFontH.pointSize() > 1) // Ако най-дългият ред не се събира.
  {
    textFontH.setPointSize(textFontH.pointSize() - 1);
    resizeText(); // Рекурсивно извикване на този метод, но с по-малък размер на шрифта.
    return;
  }

  // Проверка дали се събират всички редове на текста:
  textHeight = (rowsInCoupletH.count() * lineSpacing) - (lineSpacing - textFontH.pointSize()); // Изважда се последното разстояние между редовете.
  if (rowsInCoupletH.count() > 1 && textHeight > textArea.height()) // Ако редовете не се събират.
  {
    // Прави се изчисление дали текстът ще се събере, ако се намали разстоянието между редовете.
    int spacing = (textArea.height() - (rowsInCoupletH.count() * textFontH.pointSize())) / rowsInCoupletH.count() - 1; // Това е разстоянието между редовете, ако се направи така, че всички редове да се съберат.
    if (spacing > 15) // Ако това разстояние е достатъчно за да не се получи припокриване на редовете.
    {
      lineSpacing = textFontH.pointSize() + spacing;
      textHeight = (rowsInCoupletH.count() * lineSpacing) - (lineSpacing - textFontH.pointSize()); // Изважда се последното разстояние между редовете.
    }
    else // Ако това разстояние е твърде малко вследствие на което ще се получи припокриване на редовете.
    {
      if (textFontH.pointSize() > 1)
      {
        textFontH.setPointSize(textFontH.pointSize() - 1);
        resizeText(); // Рекурсивно извикване на този метод, но с по-малък размер на шрифта.
        return;
      }
    }
  }
}

bool MainWindow::isUTF8TextCodec(QString fileName)
{
  // Този метод проверява дали текстът е в UTF-8 или UTF-8-BOM енкодинг.
  // http://stackoverflow.com/questions/18226858/detect-text-file-encoding
  // http://stackoverflow.com/questions/18227530/check-if-utf-8-string-is-valid-in-qt/18228382#18228382
  
  // Същият метод се намира и в extextwindow.cpp. Всяка промяна да се отрази и в двата.

  QFile file(fileName);
  if (!file.exists()) return true; // Ако няма такъв файл ще го възприеме като UTF-8.

  file.open(QIODevice::ReadOnly | QIODevice::Text);
  QTextCodec::ConverterState state;
  QTextCodec *codec = QTextCodec::codecForName("UTF-8");
  const QByteArray data = file.readAll();
  const QString text = codec->toUnicode(data.constData(), data.size(), &state); // Прочита целия текст, като се опитва да го декодира като UTF-8.
  file.close();

  if (text.isEmpty()) // Ако файлът е празен ще го възприеме като UTF-8.
  {
    return true; // UTF-8
  }
  else if (state.invalidChars > 0) // Ако има грешки при декодирането.
  {
    return false; // Not a UTF-8 text - using system default locale
  }
  else
  {
    return true; // UTF-8
  }
}

void MainWindow::showWallpaper()
{
  if (ui->chbDisable->isChecked()) return;

  // Показва картинката за фон:
  ui->labelWallpaper->setPixmap(currentWallpaperPreviewScreen);
  PW->labelWallpaper->setPixmap(currentWallpaperProjectorScreen);
}

void MainWindow::showText()
{
  if (ui->chbDisable->isChecked()) return;

  if (ui->tabWidget->currentIndex() == 1 || ui->tabWidget->currentIndex() == 2) return; // Ако е показан раздел "Pictures1" или  "Pictures2"

  // Показва текста:
  textFont.setFamily(ui->cbFont->currentText());
  textFontH = textFont;
  rowsInCoupletH = rowsInCouplet;

  if (textUpper)
  {
    for (int i = 0; i < rowsInCoupletH.count(); i++)
    {
      rowsInCoupletH[i] = rowsInCoupletH[i].toUpper();
    }
  }

  resizeText();
  ui->lblFontPointSize->setText(tr("%1 (Max %2)").arg(textFontH.pointSize()).arg(textFont.pointSize()));
  

  QPixmap pixmap(projectorScreenWidth, projectorScreenHeight); // Върху тази картинка ще се изписва текста.
  pixmap.fill(QColor(0, 0, 0, 0)); // Прозрачност.

  QPainter painter(&pixmap);
  painter.setFont(textFontH);
  painter.setRenderHints(QPainter::Antialiasing, false); // Това не важи за текста, защото се взима Antialias на TextFontH. Важи за всички други обекти, като панел за името на текста и книгата.

  int shiftY = 0;
  if (ui->cbTextAlignV->currentIndex() == 1) // Ако текстът е по средата.
    shiftY = (textArea.height() - textHeight) / 2;
  else if (ui->cbTextAlignV->currentIndex() == 2) // Ако текстът е долу.
    shiftY = textArea.height() - textHeight;
  int Y = textArea.y() + shiftY + textFontH.pointSize(); // Положение по Y на първия ред.
  int X; // Положение по X за всеки ред.
  QFontMetrics fm(textFontH);

  if (textOutline > 2) // Ако трябва да се изпише сянка.
  {
    int textShadow = textOutline - 2;
    painter.setPen(textOutlineColor);
    for (int i = 0; i < rowsInCoupletH.count(); i++)
    {
      X = textArea.x();
      if (ui->cbTextAlignH->currentIndex() == 1) // Ако текстът е по средата.
        X = X + (textArea.width() - fm.width(rowsInCoupletH[i])) / 2;
      else if (ui->cbTextAlignH->currentIndex() == 2) // Ако текстът е в дясно.
        X = X + textArea.width() - fm.width(rowsInCoupletH[i]);

      painter.drawText(X + textShadow, Y + (i*lineSpacing) + textShadow, rowsInCoupletH[i]);
    }
  }
  else if (textOutline > 0) // Ако трябва да се изпише контур.
  {
    painter.setPen(textOutlineColor);
    for (int i = 0; i < rowsInCoupletH.count(); i++)
    {
      X = textArea.x();
      if (ui->cbTextAlignH->currentIndex() == 1) // Ако текстът е по средата.
        X = X + (textArea.width() - fm.width(rowsInCoupletH[i])) / 2;
      else if (ui->cbTextAlignH->currentIndex() == 2) // Ако текстът е в дясно.
        X = X + textArea.width() - fm.width(rowsInCoupletH[i]);

      painter.drawText(X + textOutline, Y + (i*lineSpacing) + textOutline, rowsInCoupletH[i]);
      painter.drawText(X + textOutline, Y + (i*lineSpacing) - textOutline, rowsInCoupletH[i]);
      painter.drawText(X - textOutline, Y + (i*lineSpacing) - textOutline, rowsInCoupletH[i]);
      painter.drawText(X - textOutline, Y + (i*lineSpacing) + textOutline, rowsInCoupletH[i]);
    }
  }

  painter.setPen(textColor);
  for (int i = 0; i < rowsInCoupletH.count(); i++)
  {
    X = textArea.x();
    if (ui->cbTextAlignH->currentIndex() == 1) // Ако текстът е по средата.
      X = X + (textArea.width() - fm.width(rowsInCoupletH[i])) / 2;
    else if (ui->cbTextAlignH->currentIndex() == 2) // Ако текстът е в дясно.
      X = X + textArea.width() - fm.width(rowsInCoupletH[i]);

    painter.drawText(X, Y + (i*lineSpacing), rowsInCoupletH[i]);
  }

  // Заглавния панел:
  if (rowsInCoupletH.count() > 0) // Ако има текст.
  {
    if (ui->cbTitleBarPosition->currentIndex() == 0) // Left
    {
      painter.setPen(QPen(textColor, 1));
      //painter.drawRect(textArea);
      QColor titlePanelColor(textColor);
      if (ui->cbTitleBarAlpha->currentIndex() == 0)
        titlePanelColor.setAlpha(80);
      else
        titlePanelColor.setAlpha(0);
      painter.setBrush(titlePanelColor);
      painter.drawRect(-1, -1, titlePanelHeight, projectorScreenHeight+2);

      if (ui->tabWidget->currentIndex() == 0 && ui->chbTabSongsShowTitle->isChecked()) // Ако е показан раздел "Songs" и е разрешено показването на името на песента.
      {
        painter.rotate(270); // -90
        QFont titlePanelFont(textFontH);
        titlePanelFont.setBold(true);
        titlePanelFont.setItalic(true);
        
        // Изписва името на песента:
        QString songTitle = ui->lblTitle->windowTitle(); // Взима името в чистия му вид (без html таговете за форматиране).
        titlePanelFont.setPointSize(40);
        QFontMetrics fmC(titlePanelFont);
        int wordWidthC = fmC.width(songTitle); // Дължина на думата в точки.
        painter.setFont(titlePanelFont);
        painter.setPen(textOutlineColor);
        painter.drawText(-wordWidthC-19, 76, songTitle);
        painter.setPen(textColor);
        painter.drawText(-wordWidthC-20, 75, songTitle);
      }
    }
    else if (ui->cbTitleBarPosition->currentIndex() == 2) // Right
    {
      painter.setPen(QPen(textColor, 1));
      //painter.drawRect(textArea);
      QColor titlePanelColor(textColor);
      if (ui->cbTitleBarAlpha->currentIndex() == 0)
        titlePanelColor.setAlpha(80);
      else
        titlePanelColor.setAlpha(0);
      painter.setBrush(titlePanelColor);
      painter.drawRect(projectorScreenWidth-titlePanelHeight, -1, titlePanelHeight, projectorScreenHeight+2);

      if (ui->tabWidget->currentIndex() == 0 && ui->chbTabSongsShowTitle->isChecked()) // Ако е показан раздел "Songs" и е разрешено показването на името на песента.
      {
        painter.rotate(270); // -90
        QFont titlePanelFont(textFontH);
        titlePanelFont.setBold(true);
        titlePanelFont.setItalic(true);
        
        // Изписва името на песента:
        QString songTitle = ui->lblTitle->windowTitle(); // Взима името в чистия му вид (без html таговете за форматиране).
        titlePanelFont.setPointSize(40);
        QFontMetrics fmC(titlePanelFont);
        int wordWidthC = fmC.width(songTitle); // Дължина на думата в точки.
        painter.setFont(titlePanelFont);
        painter.setPen(textOutlineColor);
        painter.drawText(-wordWidthC-19, projectorScreenWidth+35-75, songTitle);
        painter.setPen(textColor);
        painter.drawText(-wordWidthC-20, projectorScreenWidth+35-76, songTitle);
      }
    }
    else if (ui->cbTitleBarPosition->currentIndex() == 1) // Top
    {
      painter.setPen(QPen(textColor, 1));
      //painter.drawRect(textArea);
      QColor titlePanelColor(textColor);
      if (ui->cbTitleBarAlpha->currentIndex() == 0)
        titlePanelColor.setAlpha(80);
      else
        titlePanelColor.setAlpha(0);
      painter.setBrush(titlePanelColor);
      painter.drawRect(-1, -1, projectorScreenWidth+2, titlePanelHeight);

      if (ui->tabWidget->currentIndex() == 0 && ui->chbTabSongsShowTitle->isChecked()) // Ако е показан раздел "Songs" и е разрешено показването на името на песента.
      {
        QFont titlePanelFont(textFontH);
        titlePanelFont.setBold(true);
        titlePanelFont.setItalic(true);

        // Изписва името на песента:
        QString songTitle = ui->lblTitle->windowTitle(); // Взима името в чистия му вид (без html таговете за форматиране).
        titlePanelFont.setPointSize(40);
        QFontMetrics fmC(titlePanelFont);
        int wordWidthC = fmC.width(songTitle); // Дължина на думата в точки.
        painter.setFont(titlePanelFont);
        painter.setPen(textOutlineColor);
        painter.drawText(projectorScreenWidth-wordWidthC-19, 76, songTitle);
        painter.setPen(textColor);
        painter.drawText(projectorScreenWidth-wordWidthC-20, 75, songTitle);
      }
    }
    else if (ui->cbTitleBarPosition->currentIndex() == 3) // Bottom
    {
      painter.setPen(QPen(textColor, 1));
      //painter.drawRect(textArea);
      QColor titlePanelColor(textColor);
      if (ui->cbTitleBarAlpha->currentIndex() == 0)
        titlePanelColor.setAlpha(80);
      else
        titlePanelColor.setAlpha(0);
      painter.setBrush(titlePanelColor);
      painter.drawRect(-1, projectorScreenHeight-titlePanelHeight, projectorScreenWidth+2, titlePanelHeight);

      if (ui->tabWidget->currentIndex() == 0 && ui->chbTabSongsShowTitle->isChecked()) // Ако е показан раздел "Songs" и е разрешено показването на името на песента.
      {
        QFont titlePanelFont(textFontH);
        titlePanelFont.setBold(true);
        titlePanelFont.setItalic(true);

        // Изписва името на песента:
        QString songTitle = ui->lblTitle->windowTitle(); // Взима името в чистия му вид (без html таговете за форматиране).
        titlePanelFont.setPointSize(40);
        QFontMetrics fmC(titlePanelFont);
        int wordWidthC = fmC.width(songTitle); // Дължина на думата в точки.
        painter.setFont(titlePanelFont);
        painter.setPen(textOutlineColor);
        painter.drawText(projectorScreenWidth-wordWidthC-19, projectorScreenHeight+35-75, songTitle);
        painter.setPen(textColor);
        painter.drawText(projectorScreenWidth-wordWidthC-20, projectorScreenHeight+35-76, songTitle);
      }
    }
  }

  if (ui->btnProjector->isChecked()) PW->labelText->setPixmap(pixmap);
  ui->labelText->setPixmap( pixmap.scaled(previewScreenWidth, previewScreenHeight) ); // Първо премащабира голямата картинка и после я показва.
}
