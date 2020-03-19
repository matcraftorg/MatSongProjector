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
 
/***************************************************************************
 *   Идеи и части от кода са взети от програмата проектор на Ненчо Ненов   *
 ***************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "extextwindow.h"
#include "circlewidget.h"
#include "projectorwindow.h"

struct SongData // Структура от данни за всяка песен.
{
  QString filePath; // Име на файла на песента с пътя.
  QString fileName; // Име на файла на песента без пътя.
  QString personalWallpaperPath;     // Име на файла на картинка за фон на песента, ако е със собствен фон.
  quint8  personalWallpaperPosition; // Позицията на картинката за фон, ако е със собствен фон 0 - Fit; 1 - Fit Ex; 2 - Stretch; 3 - 1:1 (това е индексът на комбобокса).
  quint8  personalWallpaper;         // Указва дали тази песен има собствена картинка за фон: 0 - няма; 1 - има (това е индексът на комбобокса).
  bool    UTF8TextCodec; // Запомня дали кодировката на текста е UTF-8. Това е за по-бързо търсене в текста.
};

struct Pictures2Set // Преоразмерени картинки.
{
  QPixmap picturesBig;   // Преоразмерена картинка за екрана на проектора.
  QPixmap picturesSmall; // Преоразмерена картинка за малкия екран.
};

namespace Ui
{
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

protected:
  void closeEvent(QCloseEvent *e);
  void keyPressEvent(QKeyEvent *event);
  void resizeEvent(QResizeEvent *event);
  void moveEvent(QMoveEvent *event);

private:
  CircleWidget *circleWidgetPreview;
  CircleWidget *circleWidgetProjector;
  ProjectorWindow *PW;
  QList<SongData> SD;
  bool    SDPersonalWallpaperChanged; // Флаг, указващ, че има промени в списъка със собствените картинки за фон на песните.
  int     projectorScreenXAuto;  // Отместване по X, където трябва да се покаже екранът на проектора, когато местоположението му се определя автоматично.
  int     projectorScreenX;      // Отместване по X, където трябва да се покаже екранът на проектора, когато местоположението му се определя ръчно.
  int     projectorScreenY;      // Отместване по Y, където трябва да се покаже екранът на проектора, когато местоположението му се определя ръчно.
  int     projectorScreenWidth;  // Широчина на екрана на проектора, когато местоположението му се определя ръчно.
  int     projectorScreenHeight; // Височина на екрана на проектора, когато местоположението му се определя ръчно.
  int     previewScreenWidth;    // Широчина на малкия екран. Изчислява се автоматично.
  int     previewScreenHeight;   // Височина на малкия екран. Изчислява се автоматично.
  int     titlePanelHeight;      // Широчина/височина на заглавния панел.
  QSize   mainWindowSize; // Помни размера на прозореца, когато не е максимизиран.
  QPoint  mainWindowPos;  // Помни позицията на прозореца, когато не е максимизиран.
  int     songListWidgetZoom; // Големина на шрифта на списъка с имената на файловете на песните.
  int     textListWidgetZoom; // Големина на шрифта на текста на песните.
  int     customTextEditZoom; // Големина на шрифта на текста на потребителския текст.
  QString wallpapersPath; // Път до последно отворената директория с картинки за фон на песните.
  QString songsPath; // Път до последно отворената директория за текст (песни).
  QString pictures1Path; // Път до последно отворената директория с картинки.
  QString pictures2Path; // Път до последно отворената директория с картинки за автоматично сменяне.
  QString currentCouplet; // Текущ куплет в песента.
  QStringList currentCouplets; // Текстът на файла (песента), който е активен в момента. Всеки куплет е отделен стринг.
  QStringList rowsInCouplet;  // Текущият куплет, представен като списък от редове.
  QStringList rowsInCoupletH; // Помощна. Текущият куплет, представен като списък от редове, но може да е TextUpper.
  QStringList pictures2Files; // Списък с имената на файловете на картинките, които ще се сменят автоматично.
  QList<Pictures2Set> P2S; // Списък с преоразмерените картинки, които ще се сменят автоматично.
  int     pictures1FilesCurrentItem; // Текуща снимка от списъка.
  int     pictures2FilesCurrentItem; // Текуща снимка от списъка.
  QPixmap embeddedWallpaperPreviewScreen;   // Вграден фон за малкия екран.
  QPixmap embeddedWallpaperProjectorScreen; // Вграден фон за проектора.
  QPixmap customWallpaperPreviewScreen;     // Външно изображение за фон за малкия екран.
  QPixmap customWallpaperProjectorScreen;   // Външно изображение за фон за проектора.
  QPixmap colorWallpaperPreviewScreen;      // Цвят на фон за малкия екран.
  QPixmap colorWallpaperProjectorScreen;    // Цвят на фон за проектора.
  QPixmap currentWallpaperPreviewScreen;    // Текущ фон за малкия екран (един от горните, според състоянието на радио-бутоните).
  QPixmap currentWallpaperProjectorScreen;  // Текущ фон за проектора (един от горните, според състоянието на радио-бутоните).
  QRect   textArea; // Достъпната площ върху която ще се изписва текстът (X, Y, Width, Height).
  QFont   textFont; // Шрифт на текста (съдържа трите компонента: FontName, Bold, Italic).
  QFont   textFontH; // Преизчислен шрифт (TextFont) на текста показан на екрана, така че да се събира целият текст (PointSize се изчислява автоматично и е по-малък или равен на зададения с цел да се покаже целият текст).
  int     lineSpacing; // Разстояние между редовете (по-точно от началото на единия ред до началото на другия).
  int     textHeight;  // Височина на текста, когато е събран изцяло на екрана.
  QColor  textColor; // Цвят на шрифта.
  QColor  textOutlineColor; // Цвят на контура на текста.
  int     textOutline; // Дебелина на контура на текста в пиксели.
  bool    textUpper; // Главни(=true)/нормални(=false) букви.
  QString wallpaperFile; // Име на файла на картинката за фон. Ако е празен, значи се използва цвят вместо картинка.
  QColor  wallpaperColor; // Цвят на фона, когато не се използва картинка.
  QColor  lwTextColorB; // Цвят на фона на текста в lwText.
  int     embeddedWallpaperCircleBrightness; // Яркостта на кръговете на Embedded Wallpaper.

  Ui::MainWindow *ui;
  void loadSettings();
  void saveSettings();
  void loadSongs(bool refreshFavourites);
  void loadCouplets(QString fileName);
  void savePersonalWallpaperSet();
  void loadPictures1();
  void loadPictures2();
  void setCurrentTextsWallpaper(); // Установява текущата картинка, когато е в режим на показване на текст.
  QPixmap scalePixmapFit(bool expanding, bool retainsOriginalSizeIfItCan, int width, int height, QString filePath);
  QPixmap scalePixmapStretch(int width, int height, QString filePath);
  QPixmap scalePixmapCenter(bool preview, QString filePath);
  void resizeText();
  void resizeGeometry();
  bool isUTF8TextCodec(QString fileName);

private slots:    
  void showWallpaper();
  void showText();
  void projectorWindowMaximized();
  void songTextChanged(QString fileName);
  //
  void on_btnProjector_clicked();
  void on_lblTitle_linkActivated(const QString &link);
  void on_lwText_itemActivated(QListWidgetItem *item);
  void on_lwText_itemClicked(QListWidgetItem *item);
  void on_lwSongsFavourites_itemActivated(QListWidgetItem *item);
  void on_lwSongsFavourites_itemClicked(QListWidgetItem *item);
  void on_lwSongs_itemActivated(QListWidgetItem *item);
  void on_lwSongs_itemClicked(QListWidgetItem *item);
  void on_btnTextZoomIn_clicked();
  void on_btnTextZoomOut_clicked();
  void on_btnTextBold_clicked();
  void on_btnTextItalic_clicked();
  void on_btnTextUpper_clicked();
  void on_btnTextColor_clicked();
  void on_cbFont_currentFontChanged(const QFont &font);
  void on_cbTextAlignH_currentIndexChanged(int index);
  void on_cbTextAlignV_currentIndexChanged(int index);
  void on_cbTextOutline_currentIndexChanged(int index);
  void on_btnTextOutlineColor_clicked();
  void on_cbTitleBarPosition_currentIndexChanged(int index);
  void on_cbTitleBarAlpha_currentIndexChanged(int index);
  void on_cbWallpaperPosition_currentIndexChanged(int index);
  void on_cbWallpaper_currentIndexChanged(int index);
  void on_btnWallpaperEmbedded_clicked();
  void on_btnWallpaperCustom_clicked();
  void on_btnWallpaperColor_clicked();
  //
  void on_btnAbout_clicked();
  void on_btnQt_clicked();
  //
  void on_chbTabSongsShowTitle_clicked();
  void on_btnTabSongsAddToFavourites_clicked();
  void on_btnTabSongsDeleteFromFavourites_clicked();
  void on_btnTabSongsFilterSearchInText_clicked();
  void on_leTabSongsFilter_textChanged(const QString &text);
  void on_btnTabSongsClearFilter_clicked();
  void on_btnTabSongsFolder_clicked();
  void on_btnTabSongsFile_clicked();
  void on_btnTabSongsNew_clicked();
  void on_cbTabSongsWallpaper_currentIndexChanged(int index);
  void on_cbTabSongsWallpaperPosition_currentIndexChanged(int index);
  void on_btnTabSongsWallpaper_clicked();
  void on_btnTabSongsDelete_clicked();
  void on_btnTabSongsRename_clicked();
  void on_btnTabSongsEdit_clicked();
  void on_btnTabSongsHideText_clicked();
  void on_btnTabSongsSizeInS_clicked();
  void on_btnTabSongsSizeOutS_clicked();
  void on_btnTabSongsSizeInT_clicked();
  void on_btnTabSongsSizeOutT_clicked();
  //
  void on_btnTabPictures1Folder_clicked();
  void on_cbTabPictures1Position_currentIndexChanged(int index);
  void on_lwPictures1_itemActivated(QListWidgetItem *item);
  void on_lwPictures1_itemClicked(QListWidgetItem *item);
  void on_btnTabPictures1Auto_clicked();
  void pictures1_Timer_singleShot();
  //
  void on_btnTabPictures2Add_clicked();
  void on_btnTabPictures2Remove_clicked();
  void on_cbTabPictures2Position_currentIndexChanged(int index);
  void on_lwPictures2_itemActivated(QListWidgetItem *item);
  void on_lwPictures2_itemClicked(QListWidgetItem *item);
  void on_btnTabPictures2Auto_clicked();
  void pictures2_Timer_singleShot();
  //
  void on_btnTabCustomTextSizeIn_clicked();
  void on_btnTabCustomTextSizeOut_clicked();
  void on_btnTabCustomTextHideText_clicked();
  void on_btnTabCustomTextShowText1_clicked();
  void on_btnTabCustomTextShowText2_clicked();
  void on_btnTabCustomTextShowText3_clicked();
  //
  void on_tabWidget_currentChanged(int index);
};

#endif // MAINWINDOW_H
