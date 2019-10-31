#if 0

#include "ClipboardHandler.h"

// http://git.eclipse.org/c/platform/eclipse.platform.swt.git/plain/bundles/org.eclipse.swt/Eclipse%20SWT%20Drag%20and%20Drop/win32/org/eclipse/swt/dnd/URLTransfer.java

class ClipboardHandlerUniformResourceLocator : public ClipboardHandler
{
	ClipboardFormat cbfmtA;
	ClipboardFormat cbfmtW;
public:
	ClipboardHandlerUniformResourceLocator()
		: ClipboardHandler()
		, cbfmtA("UniformResourceLocator")
		, cbfmtW("UniformResourceLocatorW")
	{
	}
	~ClipboardHandlerUniformResourceLocator()
	{
	}
public:
	virtual QVariant	read(IDataObject* pDataObj, const QString& formatName)
	{
		if(cbfmtA.canGetData(formatName, pDataObj))
		{
			QByteArray data = cbfmtA.getData(pDataObj);
		}
		else if(cbfmtW.canGetData(formatName, pDataObj))
		{
			QByteArray data = cbfmtW.getData(pDataObj);

			return QString::fromUtf16((const ushort*)data.constData(), data.size());
		}
		return QVariant();
	}
	virtual bool		canRead(IDataObject* pDataObj, const QString& formatName)
	{
		return cbfmtA.canGetData(formatName, pDataObj)
				|| cbfmtW.canGetData(formatName, pDataObj);
	}
	virtual QWidget*	getWidget(IDataObject* pDataObj, const QString& formatName)
	{
		QVariant var = read(pDataObj, formatName);
		if(!var.isNull())
		{
			QString s = var.toString();
			QTextEdit* edit = new QTextEdit();
			edit->setReadOnly(true);
			edit->setText(s);
			return edit;
		}
		return NULL;
	}
};

static ClipboardHandlerUniformResourceLocator s_ClipboardHandlerUniformResourceLocator;

#endif
