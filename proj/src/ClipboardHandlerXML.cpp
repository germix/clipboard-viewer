#include "ClipboardHandler.h"

#include <QBuffer>
#include <QTreeView>
#include <QDomDocument>

#include "dom/domitem.h"
#include "dom/dommodel.h"

class ClipboardHandlerXML : public ClipboardHandler
{
	ClipboardFormat cbfmtXML;
public:
	ClipboardHandlerXML()
		: ClipboardHandler()
		, cbfmtXML("XML Spreadsheet")
	{
	}
	~ClipboardHandlerXML()
	{
	}
public:
	virtual QVariant	read(IDataObject* pDataObj, const QString& formatName)
	{
		if(cbfmtXML.canGetData(formatName, pDataObj))
		{
			return cbfmtXML.getData(pDataObj);
		}
		return QVariant();
	}
	virtual bool		canRead(IDataObject* pDataObj, const QString& formatName)
	{
		return cbfmtXML.canGetData(formatName, pDataObj);
	}
	virtual QWidget*	getWidget(IDataObject* pDataObj, const QString& formatName)
	{
		QVariant var = read(pDataObj, formatName);
		if(!var.isNull())
		{
			QDomDocument document;
			if (document.setContent(var.toString()))
			{
				QTreeView* view = new QTreeView();
				DomModel *newModel = new DomModel(document, view);
				view->setModel(newModel);

				return view;
			}
		}
		return NULL;
	}
};

static ClipboardHandlerXML s_ClipboardHandlerXML;
