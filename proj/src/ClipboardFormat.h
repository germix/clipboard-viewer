#ifndef CLIPBOARDFORMAT_H
#define CLIPBOARDFORMAT_H
#include <QString>
#include <QByteArray>
#include <windows.h>

class ClipboardFormat
{
public:
	int			id;
	QString		name;
public:
	ClipboardFormat(const QString& n);
	~ClipboardFormat();
public:
	QByteArray	getData(IDataObject* pDataObject);
	bool		canGetData(const QString& formatName, IDataObject* pDataObject);
};

#endif // CLIPBOARDFORMAT_H
