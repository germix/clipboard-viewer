#include "ClipboardHandler.h"
#include <QTextEdit>

#include <shlobj.h>

//
// https://msdn.microsoft.com/en-us/library/bb776902.aspx#CFSTR_SHELLIDLISTOFFSET
//
class ClipboardHandlerShellObjectOffsets : public ClipboardHandler
{
	ClipboardFormat cbfmtShellObjectOffsets;
public:
	ClipboardHandlerShellObjectOffsets()
		: ClipboardHandler()
		, cbfmtShellObjectOffsets("Shell Object Offsets")
	{
	}
	~ClipboardHandlerShellObjectOffsets()
	{
	}
public:
	virtual QVariant	read(IDataObject* pDataObj, const QString& formatName)
	{
		QVariant ret;
		if(cbfmtShellObjectOffsets.canGetData(formatName, pDataObj))
		{
			QString s;
			QByteArray data = cbfmtShellObjectOffsets.getData(pDataObj);
			POINT* points = (POINT*)data.constData();
			int count = data.size() / sizeof(POINT);

			for(int i = 0; i < count; i++)
			{
				s += QString().sprintf("POINT[%d]\n", i);
				s += QString().sprintf("    x: %d\n", points[i].x);
				s += QString().sprintf("    y: %d\n", points[i].y);
			}
			return s;
		}
		return ret;
	}
	virtual bool		canRead(IDataObject* pDataObj, const QString& formatName)
	{
		if(cbfmtShellObjectOffsets.canGetData(formatName, pDataObj))
		{
			return true;
		}
		return false;
	}
	virtual QWidget*	getWidget(IDataObject* pDataObj, const QString& formatName)
	{
		QVariant data = read(pDataObj, formatName);
		if(data.isValid())
		{
			QTextEdit* edit = new QTextEdit();
			edit->setReadOnly(true);
			edit->setText(data.toString());
			return edit;
		}
		return NULL;
	}
};

static ClipboardHandlerShellObjectOffsets s_ClipboardHandlerShellObjectOffsets;





