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

#include "extextwindow.h"
#include <QMessageBox> // QMessageBox::information(this, QCoreApplication::applicationName(), QString(""));

ExTextWindow::ExTextWindow(QString filePath, QString fileName, int Width, int Height, QWidget *parent) : QMainWindow(parent)
{
  setMinimumSize(10, 10);
  resize(Width, Height);

  this->filePath = filePath;
  this->fileName = fileName;
  this->isUTF8 = isUTF8TextCodec(filePath);
  this->isUTF8BOM = isUTF8BOMTextCodec(filePath);
#if defined(Q_OS_WIN)
  this->isWindows = true;
#else
  this->isWindows = false;
#endif

  editor = new QPlainTextEdit;
  editor->setMinimumSize(30, 10);
  editor->setWordWrapMode(QTextOption::NoWrap); // WordWrap

  toolBar = new QToolBar(this);
  toolBar->setContentsMargins(0, 0, 0, 0);
  toolBar->layout()->setMargin(1); // Марджини.
  toolBar->layout()->setSpacing(1); // Разстояние между обектите.
  toolBar->setMovable(false);

  btnSave = new QPushButton(QString("Save"), toolBar);
  connect(btnSave, SIGNAL(clicked()), this, SLOT(save()));

  btnClose = new QPushButton(QString("Close"), toolBar);
  connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
  
  toolBar->addWidget(btnClose);
  toolBar->addWidget(btnSave);

  addToolBar(Qt::TopToolBarArea, toolBar);
  setCentralWidget(editor);

  editor->setFocus();

  // Отваря файла:
  QFile *file = new QFile(filePath);
  if (file->open(QFile::ReadWrite | QFile::Text))
  {
    QTextStream in(file);
    if (isUTF8)
      in.setCodec(QTextCodec::codecForLocale()); // В main.cpp това е установено на UTF-8.
    else
      in.setCodec(QTextCodec::codecForName("System")); // Взима системния енкодинг.
    in.setAutoDetectUnicode(true); // Това май е излишно. Без значение какъв кодек е избран по-горе, това може да разпознае UTF-8-BOM, но не и стандартния UTF-8 (без BOM байтовете).

    editor->document()->setPlainText(in.readAll());
    editor->document()->setModified(false);
    editor->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor); // Показва текста от началото.
  }
  file->close();
  delete file;
}

ExTextWindow::~ExTextWindow()
{
  delete toolBar;
  delete editor;
}

void ExTextWindow::closeEvent(QCloseEvent *e)
{
  if (maybeSave())
  {
    deleteLater(); // Това ще предизвика извикването на деконструктора, който по принцип не се извиква при натискане за бутона на затваряне на формата, а само при delete.
    e->accept();
  }
  else
  {
    e->ignore();
  }
}

void ExTextWindow::keyPressEvent(QKeyEvent * event)
{
  QMainWindow::keyPressEvent(event);

  if (event->modifiers() == Qt::ControlModifier) // Ако е натиснат Ctrl.
  {
    if (event->key() == Qt::Key_S)
    {
      fileSave();
    }
  }
}

bool ExTextWindow::isUTF8TextCodec(QString fileName)
{
  // Този метод проверява дали текстът е в UTF-8 или UTF-8-BOM енкодинг.
  // http://stackoverflow.com/questions/18226858/detect-text-file-encoding
  // http://stackoverflow.com/questions/18227530/check-if-utf-8-string-is-valid-in-qt/18228382#18228382
  
  // Същият метод се намира и в mainwindow.cpp. Всяка промяна да се отрази и в двата.

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

bool ExTextWindow::isUTF8BOMTextCodec(QString fileName)
{
  // Този метод проверява дали текста е в UTF-8-BOM енкодинг.
  // http://stackoverflow.com/questions/18226858/detect-text-file-encoding
  // http://stackoverflow.com/questions/18227530/check-if-utf-8-string-is-valid-in-qt/18228382#18228382

  QFile file(fileName);
  if (!file.exists()) return isWindows; // Ако няма такъв файл ще го възприеме като UTF-8-BOM (в случай, че е под Windows).

  file.open(QIODevice::ReadOnly | QIODevice::Text);
  QTextCodec *codec = QTextCodec::codecForUtfText(file.readAll(), QTextCodec::codecForName("System"));
  file.close();

  if (file.size() == 0) // Ако файлът е празен ще го възприеме като UTF-8-BOM (в случай, че е под Windows).
  {
    return isWindows; // UTF-8-BOM
  }
  else if (codec == QTextCodec::codecForName("System")) // Ако няма открит BOM.
  {
    return false;
  }
  else
  {
    return true; // UTF-8-BOM
  }
}

bool ExTextWindow::maybeSave()
{
  if (!editor->document()->isModified()) return true; // Ако няма промяна в текста.
  QMessageBox::StandardButton ret;
  ret = QMessageBox::warning(this, QCoreApplication::applicationName(), QString("The document has been modified.\nDo you want to save your changes?"),
                             QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
  if (ret == QMessageBox::Save)
    return fileSave();
  else if (ret == QMessageBox::Cancel)
    return false;
  return true;
}

bool ExTextWindow::fileSave()
{
  QFile *file = new QFile(filePath);
  if (file->open(QFile::ReadWrite | QFile::Text | QIODevice::Truncate))
  {
    QTextStream out(file);
    if (isUTF8)
      out.setCodec(QTextCodec::codecForLocale()); // В main.cpp това е установено на UTF-8.
    else
      out.setCodec(QTextCodec::codecForName("System")); // Взима системния енкодинг.
    out.setGenerateByteOrderMark(isUTF8BOM); // Това записва UTF-8 с BOM (само ако се използва UTF-8 кодировка). Без този ред ще записва в чист UTF-8, което е за предпочитане, но ако файлът е бил отворен и е бил с BOM, трябва да се запише с BOM, за да може да се отваря коректно и с програмата с която е бил създаден.

    out << editor->document()->toPlainText();
  }
  file->close();
  delete file;

  editor->document()->setModified(false);
  emit textChanged(fileName);

  return true;
}

void ExTextWindow::save()
{
  fileSave();
}
