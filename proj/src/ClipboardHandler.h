#ifndef CLIPBOARDHANDLER_H
#define CLIPBOARDHANDLER_H
#include <QList>
#include <QWidget>
#include <QVariant>
#include <windows.h>
#include "utils.h"
#include "ClipboardFormat.h"

class ClipboardHandler
{
public:
	ClipboardHandler();
	virtual ~ClipboardHandler();
public:
	virtual QVariant	read(IDataObject* pDataObj, const QString& formatName) = 0;
	virtual bool		canRead(IDataObject* pDataObj, const QString& formatName) = 0;

	virtual QWidget*	getWidget(IDataObject* pDataObj, const QString& formatName) = 0;

public:
	static QList<ClipboardHandler*>* HandlerList();
};

#endif // CLIPBOARDHANDLER_H
