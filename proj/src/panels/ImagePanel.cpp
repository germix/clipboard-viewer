#include "ImagePanel.h"
#include <QLabel>
#include <QScrollBar>
#include <QPainter>
#include <QMouseEvent>

ImagePanel::ImagePanel(QWidget *parent) : QAbstractScrollArea(parent)
{
	connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slotScrollBar_valueChanged(int)));
	connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slotScrollBar_valueChanged(int)));
}
ImagePanel::~ImagePanel()
{
}
void ImagePanel::load(const QImage& img)
{
	image = img;
	setup();
	viewport()->update();
}
void ImagePanel::setup()
{
	if(image.isNull())
	{
		return;
	}
	QScrollBar* hbar = horizontalScrollBar();
	QScrollBar* vbar = verticalScrollBar();

	if(!(image.width() < viewport()->width()))
	{
		imageX = -hbar->value();
	}
	else
	{
		imageX = (viewport()->width() - image.width())/2;
	}
	if(!(image.height() < viewport()->height()))
	{
		imageY = -vbar->value();
	}
	else
	{
		imageY = (viewport()->height() - image.height())/2;
	}
	hbar->setRange(0, qMax(0, image.width() - viewport()->width()));
	hbar->setPageStep(viewport()->width());

	vbar->setRange(0, qMax(0, image.height() - viewport()->height()));
	vbar->setPageStep(viewport()->height());
}
void ImagePanel::paintEvent(QPaintEvent* e)
{
	Q_UNUSED(e);
	QPainter p(viewport());
	QPalette pal = palette();

	p.fillRect(rect(), pal.background());
	p.drawImage(imageX, imageY, image);
}
void ImagePanel::resizeEvent(QResizeEvent* e)
{
	Q_UNUSED(e);
	setup();
}
void ImagePanel::mouseMoveEvent(QMouseEvent* e)
{
	if(!mousePressed)
	{
		setCursor(Qt::OpenHandCursor);
	}
	else
	{
		int dx = e->pos().x()-refPos.x();
		int dy = e->pos().y()-refPos.y();
		QScrollBar* hsb = horizontalScrollBar();
		QScrollBar* vsb = verticalScrollBar();

		refPos = e->pos();
		hsb->setValue(hsb->value()-dx);
		vsb->setValue(vsb->value()-dy);
	}
}
void ImagePanel::mousePressEvent(QMouseEvent* e)
{
	if(e->button() == Qt::LeftButton)
	{
		refPos = e->pos();
		mousePressed = true;
		setCursor(Qt::ClosedHandCursor);
	}
}
void ImagePanel::mouseReleaseEvent(QMouseEvent* e)
{
	if(e->button() == Qt::LeftButton)
	{
		mousePressed = false;
		setCursor(Qt::OpenHandCursor);
	}
}
void ImagePanel::slotScrollBar_valueChanged(int value)
{
	Q_UNUSED(value);
	setup();
	viewport()->update();
}

