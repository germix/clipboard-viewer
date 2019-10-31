#ifndef HEXVIEW_H
#define HEXVIEW_H
#include <QAbstractScrollArea>
#include <QBuffer>

class HexView : public QAbstractScrollArea
{
	QByteArray data;
	QFont font;
	qint64 dataSize;
	qint64 linesCount;
public:
	HexView();
	~HexView();
public:
	void setData(const QByteArray& data);
public:
	void paintEvent(QPaintEvent* e);
	void resizeEvent(QResizeEvent* e);
private:
	void setup();
};

#endif // HEXVIEW_H
