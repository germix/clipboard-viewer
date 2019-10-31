#include "ClipboardHandler.h"

#include <QTextBrowser>

#define USE_RICH_TEXT

class ClipboardHandlerHtml : public ClipboardHandler
{
	ClipboardFormat cbfmtHtml;
#ifdef USE_RICH_TEXT
	ClipboardFormat cbfmtRichText;
#endif
public:
	ClipboardHandlerHtml()
		: ClipboardHandler()
		, cbfmtHtml("HTML Format")
#ifdef USE_RICH_TEXT
		, cbfmtRichText("Rich Text Format")
#endif
	{
	}
	~ClipboardHandlerHtml()
	{
	}
public:
	virtual QVariant	read(IDataObject* pDataObj, const QString& formatName);
	virtual bool		canRead(IDataObject* pDataObj, const QString& formatName);
	virtual QWidget*	getWidget(IDataObject* pDataObj, const QString& formatName);
};

//#define CLIPBOARDHANDLERHTML_DEBUG


static ClipboardHandlerHtml s_ClipboardHandlerHtml;


QVariant	ClipboardHandlerHtml::read(IDataObject* pDataObj, const QString& formatName)
{
	QVariant result;
	if(cbfmtHtml.canGetData(formatName, pDataObj))
	{
		QByteArray html = cbfmtHtml.getData(pDataObj);
#ifdef CLIPBOARDHANDLERHTML_DEBUG
		qDebug("ClipboardHandlerHtml::read");
		qDebug("raw :");
		qDebug(html);
#endif
		int start = html.indexOf("StartFragment:");
		int end = html.indexOf("EndFragment:");

		if (start != -1)
		{
			int startOffset = start + 14;
			int i = startOffset;
			while (html.at(i) != '\r' && html.at(i) != '\n')
				++i;
			QByteArray byteCount = html.mid(startOffset, i - startOffset);
			start = byteCount.toInt();
		}
		if (end != -1)
		{
			int endOffset = end + 12;
			int i = endOffset ;
			while (html.at(i) != '\r' && html.at(i) != '\n')
				++i;
			QByteArray byteCount = html.mid(endOffset , i - endOffset);
			end = byteCount.toInt();
		}

		if (end > start && start > 0)
		{
			html = "<!--StartFragment-->" + html.mid(start, end - start);
			html += "<!--EndFragment-->";
			html.replace('\r', "");
			result = QString::fromUtf8(html);
		}
	}
#ifdef USE_RICH_TEXT
	else if(cbfmtRichText.canGetData(formatName, pDataObj))
	{
		QByteArray richText = cbfmtRichText.getData(pDataObj);

		return QString::fromLocal8Bit(richText.data());
	}
#endif
	return result;
}
bool		ClipboardHandlerHtml::canRead(IDataObject* pDataObj, const QString& formatName)
{
	return (cbfmtHtml.canGetData(formatName, pDataObj)
#ifdef USE_RICH_TEXT
			|| cbfmtRichText.canGetData(formatName, pDataObj)
#endif
			);
}
#include <QTextCodec>
QWidget*	ClipboardHandlerHtml::getWidget(IDataObject* pDataObj, const QString& formatName)
{
	QVariant data = read(pDataObj, formatName);
	if(data.isValid())
	{
		QTextBrowser* view = new QTextBrowser(0);
		view->setReadOnly(true);
		if(formatName == cbfmtHtml.name)
		{
			view->setHtml(data.toString());
		}
#ifdef USE_RICH_TEXT
		if(formatName == cbfmtRichText.name)
		{
			view->setText(data.toString());
		}
#endif
		return view;
	}
	return NULL;
}




