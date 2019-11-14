/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "diagramitem.h"
#include <QTextStream>

#include <QMainWindow>
//#include <QPr

class DiagramScene;

QT_BEGIN_NAMESPACE
class QAction;
class QToolBox;
class QSpinBox;
class QComboBox;
class QFontComboBox;
class QButtonGroup;
class QLineEdit;
class QGraphicsTextItem;
class QFont;
class QToolButton;
class QAbstractButton;
class QGraphicsView;
class QUndoCommand;
class QUndoStack;
QT_END_NAMESPACE

//! [0]
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
   MainWindow();
	friend QDataStream & operator << (QDataStream & out, DiagramItem *& item);
	friend QDataStream & operator >> (QDataStream & in, DiagramItem *& item);

private:
	Q_SLOT void backgroundButtonGroupClicked(QAbstractButton *button);
	Q_SLOT void buttonGroupClicked(int id);
	Q_SLOT void deleteItem();
	Q_SLOT void pointerGroupClicked(int id);
	Q_SLOT void bringToFront();
	Q_SLOT void sendToBack();
	Q_SLOT void open();
	Q_SLOT void save();
	Q_SLOT void saveAs();
	Q_SLOT void paste();
	Q_SLOT void cut();
	Q_SLOT void copy();
	Q_SLOT void itemInserted(DiagramItem *item);
	Q_SLOT void textInserted(QGraphicsTextItem *item);
	Q_SLOT void currentFontChanged(const QFont &font);
	Q_SLOT void fontSizeChanged(const QString &size);
	Q_SLOT void sceneScaleChanged(const QString &scale);
	Q_SLOT void textColorChanged();
	Q_SLOT void itemColorChanged();
	Q_SLOT void lineColorChanged();
	Q_SLOT void textButtonTriggered();
	Q_SLOT void fillButtonTriggered();
	Q_SLOT void lineButtonTriggered();
	Q_SLOT void handleFontChange();
	Q_SLOT void itemSelected(QGraphicsItem *item);
	Q_SLOT void about();

	//friend QDataStream & operator <<
	//void CreateUndoView();

	void undo();
	void newFile();
	void redo();
	void group();
	void ungroup();

	//void serialize(QTextStream &text);
	void createToolBox();
	void createActions();
	void createMenus();
	void createToolbars();
	QWidget *createBackgroundCellWidget(const QString &text,
										const QString &image);
	QWidget *createCellWidget(const QString &text,
							  DiagramItem::DiagramType type);
	QMenu *createColorMenu(const char *slot, QColor defaultColor);
	QIcon createColorToolButtonIcon(const QString &image, QColor color);
	QIcon createColorIcon(QColor color);

	DiagramScene *scene;
	QGraphicsView *view;

	QAction *exitAction;
	QAction *addAction;
	QAction *deleteAction;

	QAction *openAct;
	QAction *saveAct;
	QAction *saveAsAct;
	QAction *copyAct;
	QAction *cutAct;
	QAction *pasteAct;
	QAction *undoAct;
	QAction *redoAct;
	QAction *groupAct;
	QAction *ungroupAct;
	QAction *newAct;

	QAction *toFrontAction;
	QAction *sendBackAction;
	QAction *aboutAction;

	QMenu *fileMenu;
	QMenu *itemMenu;
	QMenu *aboutMenu;
	QMenu *editMenu;
	QMenu *toolMenu;

	QToolBar *textToolBar;
	QToolBar *editToolBar;
	QToolBar *colorToolBar;
	QToolBar *pointerToolbar;

	QComboBox *sceneScaleCombo;
	QComboBox *itemColorCombo;
	QComboBox *textColorCombo;
	QComboBox *fontSizeCombo;
	QFontComboBox *fontCombo;

	QToolBox *toolBox;
	QButtonGroup *buttonGroup;
	QButtonGroup *pointerTypeGroup;
	QButtonGroup *backgroundButtonGroup;
	QToolButton *fontColorToolButton;
	QToolButton *fillColorToolButton;
	QToolButton *lineColorToolButton;
	QAction *boldAction;
	QAction *underlineAction;
	QAction *italicAction;
	QAction *textAction;
	QAction *fillAction;
	QAction *lineAction;

	QString fileName;
	QString openFile;

	QPointF point;
	qreal x;
	qreal y;
	qreal tx;
	qreal ty;

	QGraphicsItemGroup *gr;
	QList<QGraphicsItemGroup*> groupList;

	QUndoStack *undoStack;
	//QUndoView *undoView;
	QList<QGraphicsItem*> itemCopy;
};
//! [0]

#endif // MAINWINDOW_H
