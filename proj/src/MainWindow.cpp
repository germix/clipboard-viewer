#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "HexView.h"
#include "AboutDialog.h"
#include "utils.h"
#include "WinClipboard.h"



#define SETTINGS_APPLICATION "ClipboardViewer"
#define SETTINGS_ORGANIZATION "Germix"

#include <QLabel>
#include <QSettings>
#include <QClipboard>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	clearTriggered(false)
{
	ui->setupUi(this);
	QSettings s(SETTINGS_ORGANIZATION, SETTINGS_APPLICATION);

	languages.init(ui->menu_Languages, "translations", "cbvw", s.value("Language").toString());

#ifdef ICON_STATUSBAR
	//
	// Iniciar barra de estado
	//
	{
		QWidget* w = new QWidget();
		QHBoxLayout* l = new QHBoxLayout();
		QLabel* lblIcon = new QLabel();
		QLabel* lblText = new QLabel();

		/*
		w->setMinimumWidth(width);
		w->setMaximumWidth(width);
		*/
		lblIcon->setPixmap(QPixmap(":/status-warning.png"));
		lblIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
		l->addWidget(lblIcon);
		l->addWidget(lblText);
		l->setSpacing(2);
		l->setContentsMargins(0, 0, 0, 0);
		w->setLayout(l);
		w->setVisible(false);
		ui->statusBar->addWidget(w, 1);

		lblText->setText(tr("The clipboard is outdated!"));
		statusWidget = w;
	}
#endif
	//
	// ...
	//
	currentDisplay = NULL;

#ifdef USE_SPLITTER
	splitter = new QSplitter(Qt::Vertical);
	splitter->addWidget(ui->treeWidget);
#ifdef USE_SPLITTER_CONTAINER
	splitterContainer = new QWidget();
	splitter->addWidget(splitterContainer);
	QHBoxLayout* l = new QHBoxLayout();
	l->setContentsMargins(0, 0, 0, 0);
	splitterContainer->setLayout(l);
#endif
	setCentralWidget(splitter);
#endif

	//
	// Acciones
	//
	connect(ui->actionProgramExit, SIGNAL(triggered()), this, SLOT(slotActions()));
	connect(ui->actionProgramClear, SIGNAL(triggered()), this, SLOT(slotActions()));
	connect(ui->actionProgramRefresh, SIGNAL(triggered()), this, SLOT(slotActions()));
	connect(ui->actionHelpAbout, SIGNAL(triggered()), this, SLOT(slotActions()));
	connect(ui->actionOptionsAutoRefresh, SIGNAL(triggered()), this, SLOT(slotActions()));

	//
	// Recuperar geometría y estado
	//
	restoreGeometry(s.value("WindowGeometry").toByteArray());
	restoreState(s.value("WindowState").toByteArray());
	ui->treeWidget->header()->restoreState(s.value("TreeWidgetHeaderState").toByteArray());

	//
	// Clipboard
	//
	const QClipboard* cb = QApplication::clipboard();
	connect(cb, SIGNAL(dataChanged()), this, SLOT(slotClipboard_dataChanged()));

	// ...
	ui->menu_View->addAction(ui->toolBar->toggleViewAction());

	// ...
	ui->actionOptionsAutoRefresh->setChecked(s.value("AutoRefresh").toBool());

	// ...
	enumData();
	setupSize();
}

MainWindow::~MainWindow()
{
	QSettings s(SETTINGS_ORGANIZATION, SETTINGS_APPLICATION);

	s.setValue("Language", languages.language());
	s.setValue("WindowGeometry", saveGeometry());
	s.setValue("WindowState", saveState());
	s.setValue("TreeWidgetHeaderState", ui->treeWidget->header()->saveState());
	s.setValue("AutoRefresh", ui->actionOptionsAutoRefresh->isChecked());
	delete ui;
}
void MainWindow::changeEvent(QEvent* e)
{
	if(e != NULL)
	{
		switch(e->type())
		{
#if 1
			case QEvent::LocaleChange:
				{
					QString locale = QLocale::system().name();
					locale.truncate(locale.lastIndexOf('_'));
					languages.load(locale);
				}
				break;
#endif
			case QEvent::LanguageChange:
				ui->retranslateUi(this);
				break;
			default:
				break;
		}
	}
	QMainWindow::changeEvent(e);
}
void MainWindow::enumData()
{
	ui->treeWidget->clear();
	ui->treeWidget->addTopLevelItems(WinClipboard::enumerate());
#ifdef ICON_STATUSBAR
	statusWidget->hide();
#else
	ui->statusBar->clearMessage();
#endif
}
void MainWindow::setupSize()
{
	if(currentDisplay)
	{
		ui->treeWidget->setMinimumHeight(225);
		ui->treeWidget->setMaximumHeight(225);
	}
	else
	{
		ui->treeWidget->setMaximumHeight(16777215);
	}
}

void MainWindow::replaceDisplay(QWidget* widget)
{
#ifdef USE_SPLITTER
	if(currentDisplay != NULL)
	{
		delete currentDisplay;
	}
	currentDisplay = widget;
	if(currentDisplay != NULL)
	{
#ifdef USE_SPLITTER_CONTAINER
		splitterContainer->layout()->addWidget(widget);
#else
		splitter->addWidget(widget);
#endif
	}
#else
	if(currentDisplay)
	{
		ui->verticalLayout->removeWidget(currentDisplay);
		delete currentDisplay;
		currentDisplay = NULL;
	}
	currentDisplay = widget;
	if(currentDisplay)
	{
		ui->verticalLayout->addWidget(currentDisplay);
	}
	setupSize();
#endif
}
void MainWindow::slotActions()
{
	QAction* action = qobject_cast<QAction*>(sender());

	if(action == ui->actionProgramExit)
	{
		close();
	}
	else if(action == ui->actionProgramClear)
	{

		if(QMessageBox::question(0,
								 tr("Clear the clipboard"),
								 tr("Clear the clipboard?"),
								 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
		{
			clearTriggered = true;
			if(WinClipboard::clear())
			{
				ui->treeWidget->clear();
				replaceDisplay(NULL);
			}
			else
			{
				QMessageBox::information(0,
										 tr("Error"),
										 tr("Cannot empty clipboard"),
										 QMessageBox::Ok);
			}
		}
	}
	if(action == ui->actionProgramRefresh)
	{
		enumData();
	}
	else if(action == ui->actionHelpAbout)
	{
		AboutDialog(this).exec();
	}
}
void MainWindow::slotCopyName()
{
	QTreeWidgetItem* item = ui->treeWidget->currentItem();
	if(item)
	{
		qApp->clipboard()->setText(item->text(0));
	}
}
void MainWindow::slotShowAsRaw()
{
	QTreeWidgetItem* item = ui->treeWidget->currentItem();
	if(NULL != item)
	{
		extern QByteArray GetData(int cf);

		QByteArray data = GetData(item->data(0, Qt::UserRole).toUInt());

		HexView* hv = new HexView();
		hv->setData(data);
		replaceDisplay(hv);
	}
}
void MainWindow::slotClipboard_dataChanged()
{
	if(ui->actionOptionsAutoRefresh->isChecked())
	{
		enumData();
	}
	else if(!clearTriggered)
	{
#ifdef ICON_STATUSBAR
		statusWidget->show();
#else
		ui->statusBar->showMessage(tr("The clipboard is outdated!"));
#endif
	}
	clearTriggered = false;
}
void MainWindow::slotTreeWidget_customContextMenuRequested(const QPoint& pos)
{
	Q_UNUSED(pos);
	QTreeWidgetItem* item = ui->treeWidget->currentItem();
	if(NULL != item)
	{
		QMenu menu;

		menu.addAction(tr("Copy name"), this, SLOT(slotCopyName()));
		menu.addAction(tr("Show raw data"), this, SLOT(slotShowAsRaw()));
		menu.exec(QCursor::pos());
	}
}
void MainWindow::slotTreeWidget_itemDoubleClicked(QTreeWidgetItem* item, int column)
{
	Q_UNUSED(column);
	QWidget* w = GetDisplayForClipboardData(item->data(0, Qt::DisplayRole).toString());
	if(!w)
	{
		QByteArray data = GetData(item->data(0, Qt::UserRole).toUInt());

		//if(!data.isEmpty())
		{
			HexView* hv = new HexView();
			hv->setData(data);
			w = hv;
		}
	}

	if(!w)
		enumData();
	replaceDisplay(w);
}




