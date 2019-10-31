#include "ClipboardHandler.h"

class ClipboardHandlerSVG : public ClipboardHandler
{
	ClipboardFormat cbfmtSVG;
	ClipboardFormat cbfmtSVGCompressed;
	ClipboardFormat cbfmtXInkscapeSVG;
	ClipboardFormat cbfmtXInkscapeSVGCompressed;
public:
	ClipboardHandlerSVG()
		: ClipboardHandler()
		, cbfmtSVG("image/svg+xml")
		, cbfmtSVGCompressed("image/svg+xml-compressed")
		, cbfmtXInkscapeSVG("image/x-inkscape-svg")
		, cbfmtXInkscapeSVGCompressed("image/x-inkscape-svg-compressed")
	{
	}
	~ClipboardHandlerSVG()
	{
	}
public:
	virtual QVariant	read(IDataObject* pDataObj, const QString& formatName);
	virtual bool		canRead(IDataObject* pDataObj, const QString& formatName);
	virtual QWidget*	getWidget(IDataObject* pDataObj, const QString& formatName);
};

static ClipboardHandlerSVG s_ClipboardHandlerSVG;

QVariant	ClipboardHandlerSVG::read(IDataObject* pDataObj, const QString& formatName)
{
	if(cbfmtSVG.canGetData(formatName, pDataObj))
	{
		return cbfmtSVG.getData(pDataObj);
	}
	if(cbfmtSVGCompressed.canGetData(formatName, pDataObj))
	{
		return cbfmtSVGCompressed.getData(pDataObj);
	}
	if(cbfmtXInkscapeSVG.canGetData(formatName, pDataObj))
	{
		return cbfmtXInkscapeSVG.getData(pDataObj);
	}
	if(cbfmtXInkscapeSVGCompressed.canGetData(formatName, pDataObj))
	{
		return cbfmtXInkscapeSVGCompressed.getData(pDataObj);
	}
	return QVariant();
}
bool		ClipboardHandlerSVG::canRead(IDataObject* pDataObj, const QString& formatName)
{
	return cbfmtSVG.canGetData(formatName, pDataObj)
			|| cbfmtSVGCompressed.canGetData(formatName, pDataObj)
			|| cbfmtXInkscapeSVG.canGetData(formatName, pDataObj)
			|| cbfmtXInkscapeSVGCompressed.canGetData(formatName, pDataObj);
}
#include "panels/SvgPanel.h"

QWidget*	ClipboardHandlerSVG::getWidget(IDataObject* pDataObj, const QString& formatName)
{
	QVariant data = read(pDataObj, formatName);
	if(data.isValid())
	{
		SvgPanel* panel = new SvgPanel();
		panel->load(data.toByteArray());

		return panel;
	}
	return NULL;
}
