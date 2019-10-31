#include "HexView.h"
#include <QPainter>
#include <QScrollBar>

#define BYTES_PER_LINE		16

HexView::HexView()
{
	font = QFont("Courier New", 10);
}
HexView::~HexView()
{
	setup();
}
void HexView::setData(const QByteArray& _data)
{
	data = _data;
	dataSize = data.size();
	linesCount = (qint64)(dataSize / BYTES_PER_LINE);
	if((dataSize % BYTES_PER_LINE) != 0)
		linesCount++;

	setup();
}
void HexView::setup()
{
#if 0
	verticalScrollBar()->setMaximum(dataSize / BYTES_PER_LINE);
#else
	QFontMetrics fm(font);

	qint64 max = qMax((qint64)0, linesCount - 1);
	qint64 page = qMin((qint64)(viewport()->height() / fm.height()), linesCount);
	qint64 pos = verticalScrollBar()->value();

	if(pos != 0 && pos > max - page + 1)
	{
		pos = max - page + 1;
	}
	verticalScrollBar()->setPageStep(page);
	if(linesCount == 0)
		verticalScrollBar()->setMaximum(0);
	else
		verticalScrollBar()->setMaximum(max-page+1);
	verticalScrollBar()->setValue(pos);
#endif
}
void HexView::paintEvent(QPaintEvent* e)
{
	Q_UNUSED(e);
	QPainter p(viewport());

	p.fillRect(rect(), Qt::white);
	QFontMetrics fm(font);

	QScrollBar* vsb = verticalScrollBar();
	qint64 lf = vsb->value();
	//qint64 ll = qMin((qint64)(vsb->value() + height()/fm.height()), linesCount);
	qint64 ll = (vsb->value() + height()/fm.height());
	if(height() % fm.height())
		ll++;
	ll = qMin(ll, linesCount);

	const char* buf;
	//char buf[BYTES_PER_LINE];

	p.setFont(font);
	int x = 0;
	int y = 0;
	for(qint64 i = lf; i < ll; i++)
	{
		QString line;
		qint64 off = i * BYTES_PER_LINE;
		qint64 len = qMin((dataSize - off), (qint64)BYTES_PER_LINE);

		buf = data.constData()+off;

		line = QString().sprintf("%08X  ", (unsigned int)off);

		for(int j = 0; j < BYTES_PER_LINE; j++)
		{
			if(j < len)
				line += QString().sprintf("%02X ", buf[j]&0xff);
			else
				line += "   ";
		}
		line += " ";
		for(int j = 0; j < BYTES_PER_LINE; j++)
		{
			unsigned char chr;
			if(j < len)
			{
				chr = buf[j];
				if(chr < 0x20 || chr >= 0x7F)
				{
					chr = '.';
				}
			}
			else
			{
				chr = ' ';
			}
			line += QString().sprintf("%c", chr);
		}
		p.drawText(x, y + fm.height() - fm.descent(), line);

		y += fm.height();
	}
	x = ((8 + 1)) * fm.width('X');
	p.drawLine(x, 0, x, height());
	x = ((8 + 2) + (BYTES_PER_LINE*3) + (0)) * fm.width('X');
	p.drawLine(x, 0, x, height());
}
void HexView::resizeEvent(QResizeEvent* e)
{
	Q_UNUSED(e);
	setup();
	update();
}

