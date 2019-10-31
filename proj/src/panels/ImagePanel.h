#ifndef IMAGEPANEL_H
#define IMAGEPANEL_H

#include <QScrollArea>

class QLabel;

class ImagePanel : public QAbstractScrollArea
{
	Q_OBJECT
	QImage image;
	int imageX;
	int imageY;
	QPoint refPos;
	bool mousePressed;
public:
    explicit ImagePanel(QWidget *parent = 0);
	~ImagePanel();
public:
	void load(const QImage& img);
private:
	void setup();
public:
	void paintEvent(QPaintEvent* e);
	void resizeEvent(QResizeEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
	void mousePressEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
signals:

public slots:
	void slotScrollBar_valueChanged(int value);
};

#endif // IMAGEPANEL_H
