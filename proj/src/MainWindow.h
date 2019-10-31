#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QTreeWidget>

namespace Ui {
    class MainWindow;
}

#include "Languages.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT
	Ui::MainWindow*	ui;
#ifdef ICON_STATUSBAR
	QWidget*		statusWidget;
#endif
	bool			clearTriggered;

	QSplitter*		splitter;
	QWidget*		splitterContainer;
	QWidget*		currentDisplay;

	Languages		languages;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
	void changeEvent(QEvent* e);
private:
	void enumData();
	void setupSize();
	void replaceDisplay(QWidget* widget);
private slots:
	void slotActions();
	void slotCopyName();
	void slotShowAsRaw();

	void slotClipboard_dataChanged();
	void slotTreeWidget_customContextMenuRequested(const QPoint& pos);
	void slotTreeWidget_itemDoubleClicked(QTreeWidgetItem* item, int column);
};

#endif // MAINWINDOW_H
