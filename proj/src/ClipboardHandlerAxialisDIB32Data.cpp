#include "ClipboardHandler.h"

#include "panels/ImagePanel.h"

class ClipboardHandlerAxialisDIB32Data : public ClipboardHandler
{
	ClipboardFormat cbfmt;
public:
	ClipboardHandlerAxialisDIB32Data()
		: ClipboardHandler()
		, cbfmt("Axialis DIB32 Data")
	{
	}
	~ClipboardHandlerAxialisDIB32Data()
	{
	}
public:
	virtual QVariant	read(IDataObject* pDataObj, const QString& formatName)
	{
		if(cbfmt.canGetData(formatName, pDataObj))
		{
			QByteArray data = cbfmt.getData(pDataObj);
			const char* buf = data.constData();
			uint32_t hdrSize = *((unsigned long*)buf);
			uint32_t imageWidth = *((unsigned long*)(buf+4));
			uint32_t imageHeight = *((unsigned long*)(buf+8));
			QImage image(imageWidth, imageHeight, QImage::Format_ARGB32);

			for(uint32_t y = 0; y < imageHeight; y++)
			{
				memcpy(
						image.bits() + (y*imageWidth)*4,
						buf + hdrSize + (imageHeight - y - 1)*imageWidth*4, imageWidth*4);
			}
			return image;
		}
		return QVariant();
	}
	virtual bool		canRead(IDataObject* pDataObj, const QString& formatName)
	{
		return cbfmt.canGetData(formatName, pDataObj);
	}
	virtual QWidget*	getWidget(IDataObject* pDataObj, const QString& formatName)
	{
		QVariant data = read(pDataObj, formatName);
		if(data.isValid())
		{
			ImagePanel* panel = new ImagePanel();
			panel->load(data.value<QImage>());
			return panel;
		}
		return NULL;
	}
};

static ClipboardHandlerAxialisDIB32Data s_ClipboardHandlerAxialisDIB32Data;



