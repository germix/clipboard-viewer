#include "SvgPanel.h"
#include <QSvgRenderer>
#include <QPainter>
#include <QScrollBar>

SvgPanel::SvgPanel(QWidget* parent) : QAbstractScrollArea(parent)
{
	renderer = new QSvgRenderer(this);

	connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slotScrollBar_valueChanged(int)));
	connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slotScrollBar_valueChanged(int)));
}
SvgPanel::~SvgPanel()
{
}
void SvgPanel::load(const QByteArray& data)
{
	renderer->load(data);
	setup();
	viewport()->update();
}
void SvgPanel::setup()
{
	QScrollBar* hbar = horizontalScrollBar();
	QScrollBar* vbar = verticalScrollBar();

	if(!renderer->isValid())
	{
		hbar->setRange(0, 0);
		hbar->setPageStep(0);

		vbar->setRange(0, 0);
		vbar->setPageStep(0);
	}
	else
	{
		QSize s = renderer->defaultSize();

		if(!(s.width() < viewport()->width()))
		{
			imageX = -hbar->value();
		}
		else
		{
			imageX = (viewport()->width() - s.width())/2;
		}
		if(!(s.height() < viewport()->height()))
		{
			imageY = -vbar->value();
		}
		else
		{
			imageY = (viewport()->height() - s.height())/2;
		}
		hbar->setRange(0, qMax(0, s.width() - viewport()->width()));
		hbar->setPageStep(viewport()->width());

		vbar->setRange(0, qMax(0, s.height() - viewport()->height()));
		vbar->setPageStep(viewport()->height());
	}
}
void SvgPanel::paintEvent(QPaintEvent* e)
{
	Q_UNUSED(e);
	QPainter p(viewport());
	QPalette pal = palette();

	p.fillRect(rect(), pal.background());

	QSize s = renderer->defaultSize();
	renderer->render(&p, QRectF(imageX, imageY, s.width(), s.height()));
}
void SvgPanel::resizeEvent(QResizeEvent* e)
{
	Q_UNUSED(e);
	setup();
}
void SvgPanel::slotScrollBar_valueChanged(int value)
{
	Q_UNUSED(value);
	setup();
	viewport()->update();
}

