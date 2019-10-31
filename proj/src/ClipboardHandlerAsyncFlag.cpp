#include "ClipboardHandler.h"
#include <QTextEdit>

class ClipboardHandlerAsyncFlag : public ClipboardHandler
{
	ClipboardFormat cbfmtAsyncFlag;
public:
	ClipboardHandlerAsyncFlag()
		: ClipboardHandler()
		, cbfmtAsyncFlag("AsyncFlag")
	{
	}
	~ClipboardHandlerAsyncFlag()
	{
	}
public:
	virtual QVariant	read(IDataObject* pDataObj, const QString& formatName)
	{
		if(cbfmtAsyncFlag.canGetData(formatName, pDataObj))
		{
			QString s;
			QByteArray data = cbfmtAsyncFlag.getData(pDataObj);
			BOOL* boolValue = (BOOL*)data.constData();

			if(*boolValue)
				s = "AsyncFlag: true";
			else
				s = "AsyncFlag: false";

			return s;
		}
		return QVariant();
	}
	virtual bool		canRead(IDataObject* pDataObj, const QString& formatName)
	{
		return cbfmtAsyncFlag.canGetData(formatName, pDataObj);
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

static ClipboardHandlerAsyncFlag s_ClipboardHandlerAsyncFlag;
