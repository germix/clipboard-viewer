#ifndef UTILS_H
#define UTILS_H
#include <QByteArray>
#include <windows.h>

class QWidget;

// https://msdn.microsoft.com/en-us/library/windows/desktop/ff729168(v=vs.85).aspx
#ifndef CF_DIBV5
#define CF_DIBV5 17
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _FORMATLOOKUP
{
	int				id;
	const char*		name;
}FORMATLOOKUP;

#define REGISTER_FORMAT(f) { f, #f, },

////////////////////////////////////////////////////////////////////////////////////////////////////

int			getCf(const FORMATETC& formatetc);
FORMATETC	setCf(int cf);
bool		SetData(const QByteArray& data, STGMEDIUM* pmedium);
QByteArray	GetData(int cf);
QByteArray	GetData(int cf, IDataObject* DataObject);
bool		CanGetData(int cf, IDataObject* DataObject);
bool		CanGetData(int cf, IDataObject* DataObject, DWORD tymed);
QWidget*	GetDisplayForClipboardData(const QString& formatName);

#endif // UTILS_H
