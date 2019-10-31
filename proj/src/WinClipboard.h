#ifndef WINCLIPBOARD_H
#define WINCLIPBOARD_H
#include <QList>
#include <QTreeWidgetItem>

class WinClipboard
{
public:
	static bool clear();
	static QList<QTreeWidgetItem*> enumerate();
};

#endif // WINCLIPBOARD_H
