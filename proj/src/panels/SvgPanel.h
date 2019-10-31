#ifndef SVGPANEL_H
#define SVGPANEL_H

#include <QWidget>

#include <QAbstractScrollArea>
class QSvgRenderer;

class SvgPanel : public QAbstractScrollArea
{
	Q_OBJECT
	QSvgRenderer* renderer;
	int imageX;
	int imageY;
public:
    explicit SvgPanel(QWidget *parent = 0);
	~SvgPanel();
public:
	void load(const QByteArray& data);
private:
	void setup();
public:
	void paintEvent(QPaintEvent* e);
	void resizeEvent(QResizeEvent* e);
signals:

public slots:
	void slotScrollBar_valueChanged(int value);

};

#endif // SVGPANEL_H
