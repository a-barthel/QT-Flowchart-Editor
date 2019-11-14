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

#include "arrow.h"
#include "diagramitem.h"
#include "diagramscene.h"
#include "diagramtextitem.h"
#include "mainwindow.h"
#include <QUndoCommand>
#include <QUndoStack>
//#include <QUndoView>
#include <QtWidgets>

const int InsertTextButton = 10;

struct save_arrow {
	quint16 source;
	quint16 target;
};

QDataStream & operator << (QDataStream &out, const save_arrow & arrow) {
	return out << arrow.source << arrow.target;
}

QDataStream & operator >> (QDataStream &in, save_arrow & arrow) {
	return in >> arrow.source >> arrow.target;
}

//! [0]
MainWindow::MainWindow()
{
	createActions();
	createToolBox();
	createMenus();

	scene = new DiagramScene(itemMenu, this);
	scene->setSceneRect(QRectF(0, 0, 5000, 5000));
	connect(scene, SIGNAL(itemInserted(DiagramItem*)),
			this, SLOT(itemInserted(DiagramItem*)));
	connect(scene, SIGNAL(textInserted(QGraphicsTextItem*)),
			this, SLOT(textInserted(QGraphicsTextItem*)));
	connect(scene, SIGNAL(itemSelected(QGraphicsItem*)),
			this, SLOT(itemSelected(QGraphicsItem*)));
	createToolbars();

	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(toolBox);
	view = new QGraphicsView(scene);
	layout->addWidget(view);

	QWidget *widget = new QWidget;
	widget->setLayout(layout);

	setCentralWidget(widget);
	setWindowTitle(tr("Diagramscene"));
	setUnifiedTitleAndToolBarOnMac(true);
}
//! [0]

//! [1]
void MainWindow::backgroundButtonGroupClicked(QAbstractButton *button)
{
	QList<QAbstractButton *> buttons = backgroundButtonGroup->buttons();
	foreach (QAbstractButton *myButton, buttons) {
		if (myButton != button)
			button->setChecked(false);
	}
	QString text = button->text();
	if (text == tr("Blue Grid"))
		scene->setBackgroundBrush(QPixmap(":/images/background1.png"));
	else if (text == tr("White Grid"))
		scene->setBackgroundBrush(QPixmap(":/images/background2.png"));
	else if (text == tr("Gray Grid"))
		scene->setBackgroundBrush(QPixmap(":/images/background3.png"));
	else
		scene->setBackgroundBrush(QPixmap(":/images/background4.png"));

	scene->update();
	view->update();
}
//! [1]

//! [2]
void MainWindow::buttonGroupClicked(int id)
{
	QList<QAbstractButton *> buttons = buttonGroup->buttons();
	foreach (QAbstractButton *button, buttons) {
		if (buttonGroup->button(id) != button)
			button->setChecked(false);
	}
	if (id == InsertTextButton) {
		scene->setMode(DiagramScene::InsertText);
	} else {
		scene->setItemType(DiagramItem::DiagramType(id));
		scene->setMode(DiagramScene::InsertItem);
	}
}
//! [2]

//! [3]
void MainWindow::deleteItem()
{
	foreach (QGraphicsItem *item, scene->selectedItems()) {
		if (item->type() == Arrow::Type) {
			scene->removeItem(item);
			Arrow *arrow = qgraphicsitem_cast<Arrow *>(item);
			arrow->startItem()->removeArrow(arrow);
			arrow->endItem()->removeArrow(arrow);
			delete item;
		}
	}

	foreach (QGraphicsItem *item, scene->selectedItems()) {
		 if (item->type() == DiagramItem::Type)
			 qgraphicsitem_cast<DiagramItem *>(item)->removeArrows();
		 scene->removeItem(item);
		 delete item;
	 }
	//QUndoCommand *deleteCommand = new DeleteCommand(scene);
	//undoStack->push(deleteCommand);
}
//! [3]

//! [4]
void MainWindow::pointerGroupClicked(int)
{
	scene->setMode(DiagramScene::Mode(pointerTypeGroup->checkedId()));
}
//! [4]

//! [5]
void MainWindow::bringToFront()
{
	if (scene->selectedItems().isEmpty())
		return;

	QGraphicsItem *selectedItem = scene->selectedItems().first();
	QList<QGraphicsItem *> overlapItems = selectedItem->collidingItems();

	qreal zValue = 0;
	foreach (QGraphicsItem *item, overlapItems) {
		if (item->zValue() >= zValue && item->type() == DiagramItem::Type)
			zValue = item->zValue() + 0.1;
	}
	selectedItem->setZValue(zValue);
}
//! [5]

//! [6]
void MainWindow::sendToBack()
{
	if (scene->selectedItems().isEmpty())
		return;

	QGraphicsItem *selectedItem = scene->selectedItems().first();
	QList<QGraphicsItem *> overlapItems = selectedItem->collidingItems();

	qreal zValue = 0;
	foreach (QGraphicsItem *item, overlapItems) {
		if (item->zValue() <= zValue && item->type() == DiagramItem::Type)
			zValue = item->zValue() - 0.1;
	}
	selectedItem->setZValue(zValue);
}
//! [6]

inline QDataStream & operator << (QDataStream & out, DiagramItem *& item) {
	out << static_cast<int>(item->diagramType());
	out << item->pos();
	out << item->zValue();
	out << item->brush();
	out << item->getNumber();
	return out;
}

inline QDataStream & operator >> (QDataStream & in, DiagramItem *& item) {
	int x; DiagramItem::DiagramType type; qreal zVal; QPointF pos; QBrush brush; quint16 m_number;
	in >> x;
	type = static_cast<DiagramItem::DiagramType>(x);
	in >> pos >> zVal >> brush >> m_number;
	item = new DiagramItem(type, NULL);
	item->setPos(pos); item->setZValue(zVal); item->setBrush(brush);
	item->setNumber(m_number);
	return in;
}

void MainWindow::newFile() {
	openFile = "";
	scene->clear();
}

typedef DiagramScene::Mode Mode;
void MainWindow::open()
{
	scene->clear();
	openFile = QFileDialog::getOpenFileName(this,
													tr("Open File"),
													"", tr("Flowchart (*.fuck);; Any files (*.*)"));
	QFile file(openFile);
	if (!file.open(QIODevice::ReadWrite)) {
		QMessageBox::critical(this, tr("Could not open file"), tr("Error"), QMessageBox::Retry);
		return;
	}
	QDataStream stream(&file);
	quint16 numDItems;
	stream >> numDItems;

	for (quint16 x = 0; x < numDItems; ++x) {
		DiagramItem * pItem;
		stream >> pItem;
		pItem->setContextMenu(itemMenu);
        scene->setMode(DiagramScene::InsertItem);
		scene->addItem(pItem);
	}
	quint16 numAItems;
	stream >> numAItems;
	for (quint16 x = 0; x < numAItems; ++x) {
		save_arrow aItem;
		stream >> aItem;
		DiagramItem * start = scene->getByNumber(aItem.source);
		DiagramItem * end   = scene->getByNumber(aItem.target);
		Arrow * pArrow = new Arrow(start, end);
		start->addArrow(pArrow);
		end->addArrow(pArrow);
        pArrow->setVisible(true);
		pArrow->setZValue(-1000.0);
		scene->setMode(DiagramScene::InsertLine);
		scene->addItem(pArrow);
        pArrow->updatePosition();
	}
    scene->setMode(DiagramScene::MoveItem);
}

void MainWindow::save()
{
	if (openFile == NULL || openFile == "") {
		saveAs();
		return;
	}

	QFile file(openFile);
	if (!file.open(QIODevice::WriteOnly)) {
		QMessageBox::critical(this, tr("Could not save file"), tr("Error"), QMessageBox::Retry);
		return;
	}
	QDataStream stream (&file);
	quint16 count, diagItems, textItems;
	QList<Arrow*> allArrows; QList<DiagramItem*> dItems; QList<DiagramTextItem*> tItems;

	//! Count shit
	foreach (QGraphicsItem * pItem, scene->items()) {
	if (pItem->type() == DiagramItem::Type) ++diagItems, dItems << (qgraphicsitem_cast<DiagramItem*>(pItem));
        if (pItem->type() == DiagramTextItem::Type) ++textItems, tItems << (qgraphicsitem_cast<DiagramTextItem*>(pItem));
	}

	stream << diagItems;
	foreach (DiagramItem * item, dItems) {
		item->setNumber(count++);
		stream << item;
		for (int x = 0; x < item->getArrows().size(); ++x) if (!allArrows.contains(item->getArrows().at(x))) allArrows << item->getArrows().at(x);;
	}
	/**
	 * Iterate through this clitem's arrows and create save_arrow objects.
	 * This is given by the source and target of the pointers of an arrow.
	 * save_arrow arrow; (for the Arrow* pArrow)
	 * arrow.source = pArrow->startItem().getNumber();
	 * arrow.target = pArrow->endItem().getNumber();
	 */
	stream << allArrows.size();
	foreach (Arrow *item, allArrows) {
		save_arrow arrow;
		arrow.source = item->startItem()->getNumber();
		arrow.target = item->endItem()->getNumber();
		stream << arrow;
	}

	file.close();
}

void MainWindow::saveAs()
{
	fileName = QFileDialog::getSaveFileName(this, "Save File", "", ".fuck");
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly)) {
		QMessageBox::critical(this, tr("Could not save file"), tr("Error"), QMessageBox::Retry);
		return;
	}
	QDataStream out(&file);
	quint16 count, diagItems, textItems;
	QList<Arrow*> allArrows;
	int x; QList<DiagramItem *> dItems; QList<DiagramTextItem *> tItems;
	//! Count shit
	foreach (QGraphicsItem * pItem, scene->items()) {
		if (pItem->type() == DiagramItem::Type) ++diagItems, dItems << (qgraphicsitem_cast<DiagramItem*>(pItem));
        if (pItem->type() == DiagramTextItem::Type) ++textItems, tItems << (qgraphicsitem_cast<DiagramTextItem*>(pItem));
		++x; // this is apparently essential to the functionality of this save funciton.
			 // ... cause ... reasons? Probably some bit-level floating-point hacking. Yup.
	}

	out << diagItems;
	foreach (DiagramItem * item, dItems) {
		item->setNumber(count++);
		out << item;
		for (int x = 0; x < item->getArrows().size(); ++x) if (!allArrows.contains(item->getArrows().at(x))) allArrows << item->getArrows().at(x);;
	}

	quint16 numArrows = allArrows.size();
	out << numArrows;
	foreach (Arrow *item, allArrows) {
		save_arrow arrow;
		arrow.source = item->startItem()->getNumber();
		arrow.target = item->endItem()->getNumber();
		out << arrow;
	}
	file.close();
	openFile = fileName;
}

void MainWindow::cut()
{
	copy();
	deleteItem();
}

void MainWindow::copy()
{
	itemCopy.clear();
	x = 0;
	y = 0;
	tx = 0;
	ty = 0;
	DiagramItem * mainItem;
	QGraphicsLineItem * lItem;
	DiagramTextItem * tItem;
	foreach (QGraphicsItem *item, scene->selectedItems()) {
		if (item->type() == DiagramItem::Type) {
			DiagramItem * itemD = qgraphicsitem_cast<DiagramItem*>(item);
			mainItem = new DiagramItem(itemD->myDiagramType, NULL);
			mainItem->setBrush(itemD->brush());
			mainItem->setPos(itemD->pos());
			mainItem->setZValue(itemD->zValue());
			x = 10;
			y = 10;
			itemCopy.append(mainItem);
		}
		if (item->type() == QGraphicsLineItem::Type) {
			QGraphicsLineItem *lineItem = qgraphicsitem_cast<QGraphicsLineItem*>(item);
		}
		if (item->type() == DiagramTextItem::Type) {
			DiagramTextItem *textItem = qgraphicsitem_cast<DiagramTextItem*>(item);
			tItem = new DiagramTextItem();
			tItem->setPlainText(textItem->toPlainText());
			tItem->setFont(textItem->font());
			tItem->setX(textItem->x());
			tItem->setY(textItem->y());
			tx = 10;
			ty = 10;
			tItem->setToolTip(textItem->toolTip());
			itemCopy.append(tItem);
		}
		if (item->type() == QGraphicsItemGroup::Type) {
			QGraphicsItemGroup *groupItem = qgraphicsitem_cast<QGraphicsItemGroup*>(item);

		}
	}
}

void MainWindow::paste()
{
	DiagramItem *dItem;
	QGraphicsLineItem *lItem;
	DiagramTextItem * tItem;
	foreach (QGraphicsItem *item, itemCopy) {
		if (item->type() == DiagramItem::Type) {
			DiagramItem * temp = qgraphicsitem_cast<DiagramItem*>(item);
			dItem = new DiagramItem(temp->myDiagramType, NULL);
			dItem->setBrush(temp->brush());
			dItem->setPos(temp->pos() + QPointF(x,y));
			dItem->setZValue(temp->zValue());
			scene->addItem(dItem);
			x+=10;
			y+=10;
		}
		if (item->type() == QGraphicsLineItem::Type) {
			QGraphicsLineItem * temp = qgraphicsitem_cast<QGraphicsLineItem*>(item);
//extra credit
		}
		if (item->type() == DiagramTextItem::Type) {
			DiagramTextItem * textItem = qgraphicsitem_cast<DiagramTextItem*>(item);
			tItem = new DiagramTextItem();
			tItem->setPlainText(textItem->toPlainText());
			tItem->setFont(textItem->font());
			tItem->setX(textItem->x()+tx);
			tItem->setY(textItem->y()+ty);
			tx += 10;
			ty += 10;
			tItem->setZValue(textItem->zValue());
			tItem->setToolTip(textItem->toolTip());
			scene->addItem(tItem);
		}
		if (item->type() == QGraphicsItemGroup::Type) {
			QGraphicsItemGroup * groupItem = qgraphicsitem_cast<QGraphicsItemGroup*>(item);
		}
	}
}


void MainWindow::undo()
{

}

void MainWindow::redo()
{

}

void MainWindow::group()
{
	//foreach
	gr = scene->createItemGroup(scene->selectedItems());
	foreach (QGraphicsItem *item, scene->selectedItems()) {
		item->setFlag(QGraphicsItem::ItemIsMovable, false);
		item->setFlag(QGraphicsItem::ItemIsSelectable, false);
		//gr->addToGroup(item);
	}

	gr->setFlag(QGraphicsItem::ItemIsMovable, true);
	gr->setFlag(QGraphicsItem::ItemIsSelectable, true);
	//groupList.append(gr);
}

void MainWindow::ungroup()
{
	//int i = 0;
	//foreach (QGraphicsItemGroup *g, groupList) {
	//    if (scene->selectedItems() == groupList[i]->childItems()) {
	//        scene->destroyItemGroup(groupList[i]);
	//        return;
	//    }
	//    i++;
	//}

	scene->destroyItemGroup(gr);
}

//! [7]
void MainWindow::itemInserted(DiagramItem *item)
{
	pointerTypeGroup->button(int(DiagramScene::MoveItem))->setChecked(true);
	scene->setMode(DiagramScene::Mode(pointerTypeGroup->checkedId()));
	buttonGroup->button(int(item->diagramType()))->setChecked(false);
}
//! [7]

//! [8]
void MainWindow::textInserted(QGraphicsTextItem *)
{
	buttonGroup->button(InsertTextButton)->setChecked(false);
	scene->setMode(DiagramScene::Mode(pointerTypeGroup->checkedId()));
}
//! [8]

//! [9]
void MainWindow::currentFontChanged(const QFont &)
{
	handleFontChange();
}
//! [9]

//! [10]
void MainWindow::fontSizeChanged(const QString &)
{
	handleFontChange();
}
//! [10]

//! [11]
void MainWindow::sceneScaleChanged(const QString &scale)
{
	double newScale = scale.left(scale.indexOf(tr("%"))).toDouble() / 100.0;
	QMatrix oldMatrix = view->matrix();
	view->resetMatrix();
	view->translate(oldMatrix.dx(), oldMatrix.dy());
	view->scale(newScale, newScale);
}
//! [11]

//! [12]
void MainWindow::textColorChanged()
{
	textAction = qobject_cast<QAction *>(sender());
	fontColorToolButton->setIcon(createColorToolButtonIcon(
									 ":/images/textpointer.png",
									 qvariant_cast<QColor>(textAction->data())));
	textButtonTriggered();
}
//! [12]

//! [13]
void MainWindow::itemColorChanged()
{
	fillAction = qobject_cast<QAction *>(sender());
	fillColorToolButton->setIcon(createColorToolButtonIcon(
									 ":/images/floodfill.png",
									 qvariant_cast<QColor>(fillAction->data())));
	fillButtonTriggered();
}
//! [13]

//! [14]
void MainWindow::lineColorChanged()
{
	lineAction = qobject_cast<QAction *>(sender());
	lineColorToolButton->setIcon(createColorToolButtonIcon(
									 ":/images/linecolor.png",
									 qvariant_cast<QColor>(lineAction->data())));
	lineButtonTriggered();
}
//! [14]

//! [15]
void MainWindow::textButtonTriggered()
{
	scene->setTextColor(qvariant_cast<QColor>(textAction->data()));
}
//! [15]

//! [16]
void MainWindow::fillButtonTriggered()
{
	scene->setItemColor(qvariant_cast<QColor>(fillAction->data()));
}
//! [16]

//! [17]
void MainWindow::lineButtonTriggered()
{
	scene->setLineColor(qvariant_cast<QColor>(lineAction->data()));
}
//! [17]

//! [18]
void MainWindow::handleFontChange()
{
	QFont font = fontCombo->currentFont();
	font.setPointSize(fontSizeCombo->currentText().toInt());
	font.setWeight(boldAction->isChecked() ? QFont::Bold : QFont::Normal);
	font.setItalic(italicAction->isChecked());
	font.setUnderline(underlineAction->isChecked());

	scene->setFont(font);
}
//! [18]

//! [19]
void MainWindow::itemSelected(QGraphicsItem *item)
{
	DiagramTextItem *textItem =
	qgraphicsitem_cast<DiagramTextItem *>(item);

	QFont font = textItem->font();
	fontCombo->setCurrentFont(font);
	fontSizeCombo->setEditText(QString().setNum(font.pointSize()));
	boldAction->setChecked(font.weight() == QFont::Bold);
	italicAction->setChecked(font.italic());
	underlineAction->setChecked(font.underline());
}
//! [19]

//! [20]
void MainWindow::about()
{
	QMessageBox::about(this, tr("About Diagram Scene"),
					   tr("The <b>Diagram Scene</b> example shows "
						  "use of the graphics framework."));
}
//! [20]

//! [21]
void MainWindow::createToolBox()
{
	buttonGroup = new QButtonGroup(this);
	buttonGroup->setExclusive(false);
	connect(buttonGroup, SIGNAL(buttonClicked(int)),
			this, SLOT(buttonGroupClicked(int)));
	QGridLayout *layout = new QGridLayout;
	layout->addWidget(createCellWidget(tr("Conditional"), DiagramItem::Conditional), 0, 0);
	layout->addWidget(createCellWidget(tr("Process"), DiagramItem::Step),0, 1);
	layout->addWidget(createCellWidget(tr("Input/Output"), DiagramItem::Io), 1, 0);
	layout->addWidget(createCellWidget(tr("Terminal"), DiagramItem::Ellipse), 2, 0);
	layout->addWidget(createCellWidget(tr("Preperation"),DiagramItem::Hex), 2, 1);
	layout->addWidget(createCellWidget(tr("Off-Page Connector"), DiagramItem::Pent), 3, 0);
	layout->addWidget(createCellWidget(tr("Star"), DiagramItem::Star), 3, 1);
	layout->addWidget(createCellWidget(tr("Triangle"), DiagramItem::Tri), 4, 0);
	layout->addWidget(createCellWidget(tr("Decision"), DiagramItem::Dia), 4, 1);
	//layout->addWidget(createCellWidget(tr("State"), DiagramItem::Circle1), 5, 0);
//! [21]

	QToolButton *textButton = new QToolButton;
	textButton->setCheckable(true);
	buttonGroup->addButton(textButton, InsertTextButton);
	textButton->setIcon(QIcon(QPixmap(":/images/textpointer.png")));
	textButton->setIconSize(QSize(50, 50));
	QGridLayout *textLayout = new QGridLayout;
	textLayout->addWidget(textButton, 0, 0, Qt::AlignHCenter);
	textLayout->addWidget(new QLabel(tr("Text")), 1, 0, Qt::AlignCenter);
	QWidget *textWidget = new QWidget;
	textWidget->setLayout(textLayout);
	layout->addWidget(textWidget, 1, 1);

	layout->setRowStretch(3, 10);
	layout->setColumnStretch(2, 10);

	QWidget *itemWidget = new QWidget;
	itemWidget->setLayout(layout);

	backgroundButtonGroup = new QButtonGroup(this);
	connect(backgroundButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)),
			this, SLOT(backgroundButtonGroupClicked(QAbstractButton*)));

	QGridLayout *backgroundLayout = new QGridLayout;
	backgroundLayout->addWidget(createBackgroundCellWidget(tr("Blue Grid"),
														   ":/images/background1.png"), 0, 0);
	backgroundLayout->addWidget(createBackgroundCellWidget(tr("White Grid"),
														   ":/images/background2.png"), 0, 1);
	backgroundLayout->addWidget(createBackgroundCellWidget(tr("Gray Grid"),
														   ":/images/background3.png"), 1, 0);
	backgroundLayout->addWidget(createBackgroundCellWidget(tr("No Grid"),
														   ":/images/background4.png"), 1, 1);
//Do a University of Tennessee checkerboard background for mom & black/gold for Purdue
	backgroundLayout->setRowStretch(2, 10);
	backgroundLayout->setColumnStretch(2, 10);

	QWidget *backgroundWidget = new QWidget;
	backgroundWidget->setLayout(backgroundLayout);


//! [22]
	toolBox = new QToolBox;
	toolBox->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Ignored));
	toolBox->setMinimumWidth(itemWidget->sizeHint().width());
	toolBox->addItem(itemWidget, tr("Basic Flowchart Shapes"));
	toolBox->addItem(backgroundWidget, tr("Backgrounds"));
}
//! [22]

//! [23]
void MainWindow::createActions()
{
	toFrontAction = new QAction(QIcon(":/images/bringtofront.png"),
								tr("Bring to &Front"), this);
	toFrontAction->setShortcut(tr("Ctrl+F"));
	toFrontAction->setStatusTip(tr("Bring item to front"));
	connect(toFrontAction, SIGNAL(triggered()), this, SLOT(bringToFront()));
//! [23]

	sendBackAction = new QAction(QIcon(":/images/sendtoback.png"), tr("Send to &Back"), this);
	sendBackAction->setShortcut(tr("Ctrl+B"));
	sendBackAction->setStatusTip(tr("Send item to back"));
	connect(sendBackAction, SIGNAL(triggered()), this, SLOT(sendToBack()));

	openAct = new QAction(tr("&Open"), this);
	openAct->setShortcut(tr("Ctrl+O"));
	openAct->setStatusTip(tr("Open an existing file"));
	connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

	saveAct = new QAction(tr("&Save"), this);
	saveAct->setShortcut(tr("Ctrl+S"));
	saveAct->setStatusTip(tr("Save file"));
	connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

	saveAsAct = new QAction(tr("&Save As"),this);
	saveAsAct->setShortcut(tr("Ctrl+A"));
	saveAsAct->setStatusTip(tr("Save file as"));
	connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

	cutAct = new QAction(tr("&Cut"), this);
	cutAct->setShortcut(tr("Ctrl+T"));
	cutAct->setStatusTip(tr("Cut Item(s)"));
	connect(cutAct, SIGNAL(triggered()), this, SLOT(cut()));

	newAct = new QAction(tr("New"), this);
	newAct->setShortcut(tr("Ctrl+N"));
	newAct->setStatusTip(tr("New File"));
	connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));


	copyAct = new QAction(tr("&Copy"), this);
	copyAct->setShortcut(tr("Ctrl+Y"));
	copyAct->setStatusTip(tr("Copy Item(s)"));
	connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));

	pasteAct = new QAction(tr("&Paste"), this);
	pasteAct->setShortcut(tr("Ctrl+P"));
	pasteAct->setStatusTip(tr("Paste Item(s)"));
	connect(pasteAct, SIGNAL(triggered()), this, SLOT(paste()));

	deleteAction = new QAction(QIcon(":/images/delete.png"), tr("&Delete"), this);
	deleteAction->setShortcut(tr("Delete"));
	deleteAction->setStatusTip(tr("Delete item from diagram"));
	connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteItem()));

	exitAction = new QAction(tr("Exit"), this);
	exitAction->setShortcut(tr("Ctrl+X"));
	exitAction->setStatusTip(tr("Quit Scenediagram example"));
	connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

	boldAction = new QAction(tr("Bold"), this);
	boldAction->setCheckable(true);
	QPixmap pixmap(":/images/bold.png");
	boldAction->setIcon(QIcon(pixmap));
	boldAction->setShortcut(tr("Ctrl+B"));
	connect(boldAction, SIGNAL(triggered()), this, SLOT(handleFontChange()));

	italicAction = new QAction(QIcon(":/images/italic.png"), tr("Italic"), this);
	italicAction->setCheckable(true);
	italicAction->setShortcut(tr("Ctrl+I"));
	connect(italicAction, SIGNAL(triggered()), this, SLOT(handleFontChange()));

	underlineAction = new QAction(QIcon(":/images/underline.png"), tr("Underline"), this);
	underlineAction->setCheckable(true);
	underlineAction->setShortcut(tr("Ctrl+U"));
	connect(underlineAction, SIGNAL(triggered()), this, SLOT(handleFontChange()));

	aboutAction = new QAction(tr("About"), this);
	aboutAction->setShortcut(tr("Ctrl+B"));
	connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

	groupAct = new QAction(tr("Group"), this);
	groupAct->setShortcut(tr("Ctrl+G"));
	groupAct->setStatusTip(tr("Group Item(s)"));
	connect(groupAct, SIGNAL(triggered()), this, SLOT(group()));

	ungroupAct = new QAction(tr("Ungroup"), this);
	ungroupAct->setShortcut(tr("Ctrl+U"));
	ungroupAct->setStatusTip(tr("Ungroup Item(s)"));
	connect(ungroupAct, SIGNAL(triggered()), this, SLOT(ungroup()));

	undoAct = new QAction(tr("Undo"), this);
	undoAct->setShortcut(tr("Ctrl+O"));
	connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));

	redoAct = new QAction(tr("Redo"), this);
	redoAct->setShortcut(tr("Ctrl+R"));
	connect(redoAct, SIGNAL(triggered()), this, SLOT(redo()));
}

//! [24]
void MainWindow::createMenus()
{
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(openAct);
	fileMenu->addAction(newAct);
	fileMenu->addSeparator();
	fileMenu->addAction(saveAct);
	fileMenu->addAction(saveAsAct);
	fileMenu->addSeparator();
	fileMenu->addAction(exitAction);

	editMenu = menuBar()->addMenu(tr("&Edit"));
	editMenu->addAction(cutAct);
	editMenu->addAction(copyAct);
	editMenu->addAction(pasteAct);
	editMenu->addSeparator();
	editMenu->addAction(undoAct);
	editMenu->addAction(redoAct);

	toolMenu = menuBar()->addMenu(tr("&Tools"));
	toolMenu->addAction(groupAct);
	toolMenu->addAction(ungroupAct);

	itemMenu = menuBar()->addMenu(tr("&Item"));
	itemMenu->addAction(deleteAction);
	itemMenu->addSeparator();
	itemMenu->addAction(toFrontAction);
	itemMenu->addAction(sendBackAction);

	aboutMenu = menuBar()->addMenu(tr("&Help"));
	aboutMenu->addAction(aboutAction);
}
//! [24]

//! [25]
void MainWindow::createToolbars()
{
//! [25]
	editToolBar = addToolBar(tr("Edit"));
	editToolBar->addAction(deleteAction);
	editToolBar->addAction(toFrontAction);
	editToolBar->addAction(sendBackAction);

	fontCombo = new QFontComboBox();
	connect(fontCombo, SIGNAL(currentFontChanged(QFont)),
			this, SLOT(currentFontChanged(QFont)));

	fontSizeCombo = new QComboBox;
	fontSizeCombo->setEditable(true);
	for (int i = 8; i < 30; i = i + 2)
		fontSizeCombo->addItem(QString().setNum(i));
	QIntValidator *validator = new QIntValidator(2, 64, this);
	fontSizeCombo->setValidator(validator);
	connect(fontSizeCombo, SIGNAL(currentIndexChanged(QString)),
			this, SLOT(fontSizeChanged(QString)));

	fontColorToolButton = new QToolButton;
	fontColorToolButton->setPopupMode(QToolButton::MenuButtonPopup);
	fontColorToolButton->setMenu(createColorMenu(SLOT(textColorChanged()), Qt::black));
	textAction = fontColorToolButton->menu()->defaultAction();
	fontColorToolButton->setIcon(createColorToolButtonIcon(":/images/textpointer.png", Qt::black));
	fontColorToolButton->setAutoFillBackground(true);
	connect(fontColorToolButton, SIGNAL(clicked()),
			this, SLOT(textButtonTriggered()));

//! [26]
	fillColorToolButton = new QToolButton;
	fillColorToolButton->setPopupMode(QToolButton::MenuButtonPopup);
	fillColorToolButton->setMenu(createColorMenu(SLOT(itemColorChanged()), Qt::white));
	fillAction = fillColorToolButton->menu()->defaultAction();
	fillColorToolButton->setIcon(createColorToolButtonIcon(
									 ":/images/floodfill.png", Qt::white));
	connect(fillColorToolButton, SIGNAL(clicked()),
			this, SLOT(fillButtonTriggered()));
//! [26]

	lineColorToolButton = new QToolButton;
	lineColorToolButton->setPopupMode(QToolButton::MenuButtonPopup);
	lineColorToolButton->setMenu(createColorMenu(SLOT(lineColorChanged()), Qt::black));
	lineAction = lineColorToolButton->menu()->defaultAction();
	lineColorToolButton->setIcon(createColorToolButtonIcon(
									 ":/images/linecolor.png", Qt::black));
	connect(lineColorToolButton, SIGNAL(clicked()),
			this, SLOT(lineButtonTriggered()));

	textToolBar = addToolBar(tr("Font"));
	textToolBar->addWidget(fontCombo);
	textToolBar->addWidget(fontSizeCombo);
	textToolBar->addAction(boldAction);
	textToolBar->addAction(italicAction);
	textToolBar->addAction(underlineAction);

	colorToolBar = addToolBar(tr("Color"));
	colorToolBar->addWidget(fontColorToolButton);
	colorToolBar->addWidget(fillColorToolButton);
	colorToolBar->addWidget(lineColorToolButton);

	QToolButton *pointerButton = new QToolButton;
	pointerButton->setCheckable(true);
	pointerButton->setChecked(true);
	pointerButton->setIcon(QIcon(":/images/pointer.png"));
	QToolButton *linePointerButton = new QToolButton;
	linePointerButton->setCheckable(true);
	linePointerButton->setIcon(QIcon(":/images/linepointer.png"));

	pointerTypeGroup = new QButtonGroup(this);
	pointerTypeGroup->addButton(pointerButton, int(DiagramScene::MoveItem));
	pointerTypeGroup->addButton(linePointerButton, int(DiagramScene::InsertLine));
	connect(pointerTypeGroup, SIGNAL(buttonClicked(int)),
			this, SLOT(pointerGroupClicked(int)));

	sceneScaleCombo = new QComboBox;
	QStringList scales;
	scales << tr("50%") << tr("75%") << tr("100%") << tr("125%") << tr("150%");
	sceneScaleCombo->addItems(scales);
	sceneScaleCombo->setCurrentIndex(2);
	connect(sceneScaleCombo, SIGNAL(currentIndexChanged(QString)),
			this, SLOT(sceneScaleChanged(QString)));

	pointerToolbar = addToolBar(tr("Pointer type"));
	pointerToolbar->addWidget(pointerButton);
	pointerToolbar->addWidget(linePointerButton);
	pointerToolbar->addWidget(sceneScaleCombo);
//! [27]
}
//! [27]

//! [28]
QWidget *MainWindow::createBackgroundCellWidget(const QString &text, const QString &image)
{
	QToolButton *button = new QToolButton;
	button->setText(text);
	button->setIcon(QIcon(image));
	button->setIconSize(QSize(50, 50));
	button->setCheckable(true);
	backgroundButtonGroup->addButton(button);

	QGridLayout *layout = new QGridLayout;
	layout->addWidget(button, 0, 0, Qt::AlignHCenter);
	layout->addWidget(new QLabel(text), 1, 0, Qt::AlignCenter);

	QWidget *widget = new QWidget;
	widget->setLayout(layout);

	return widget;
}
//! [28]

//! [29]
QWidget *MainWindow::createCellWidget(const QString &text, DiagramItem::DiagramType type)
{

	DiagramItem item(type, itemMenu);
	QIcon icon(item.image());

	QToolButton *button = new QToolButton;
	button->setIcon(icon);
	button->setIconSize(QSize(50, 50));
	button->setCheckable(true);
	buttonGroup->addButton(button, int(type));

	QGridLayout *layout = new QGridLayout;
	layout->addWidget(button, 0, 0, Qt::AlignHCenter);
	layout->addWidget(new QLabel(text), 1, 0, Qt::AlignCenter);

	QWidget *widget = new QWidget;
	widget->setLayout(layout);

	return widget;
}
//! [29]

//! [30]
QMenu *MainWindow::createColorMenu(const char *slot, QColor defaultColor)
{
	QList<QColor> colors;
	colors << Qt::black << Qt::white << Qt::red << Qt::blue << Qt::yellow;
	QStringList names;
	names << tr("black") << tr("white") << tr("red") << tr("blue")
		  << tr("yellow");

	QMenu *colorMenu = new QMenu(this);
	for (int i = 0; i < colors.count(); ++i) {
		QAction *action = new QAction(names.at(i), this);
		action->setData(colors.at(i));
		action->setIcon(createColorIcon(colors.at(i)));
		connect(action, SIGNAL(triggered()), this, slot);
		colorMenu->addAction(action);
		if (colors.at(i) == defaultColor)
			colorMenu->setDefaultAction(action);
	}
	return colorMenu;
}
//! [30]

//! [31]
QIcon MainWindow::createColorToolButtonIcon(const QString &imageFile, QColor color)
{
	QPixmap pixmap(50, 80);
	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);
	QPixmap image(imageFile);
	// Draw icon centred horizontally on button.
	QRect target(4, 0, 42, 43);
	QRect source(0, 0, 42, 43);
	painter.fillRect(QRect(0, 60, 50, 80), color);
	painter.drawPixmap(target, image, source);

	return QIcon(pixmap);
}
//! [31]

//! [32]
QIcon MainWindow::createColorIcon(QColor color)
{
	QPixmap pixmap(20, 20);
	QPainter painter(&pixmap);
	painter.setPen(Qt::NoPen);
	painter.fillRect(QRect(0, 0, 20, 20), color);

	return QIcon(pixmap);
}
//! [32]
