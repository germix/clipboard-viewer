#include "ClipboardFormat.h"
#include "utils.h"


static int RegisterMimeType(const QString& mime)
{
	int cf = 0;
	static FORMATLOOKUP defaultMimes[] =
	{
		REGISTER_FORMAT(CF_TEXT)
		REGISTER_FORMAT(CF_BITMAP)
		REGISTER_FORMAT(CF_METAFILEPICT)
		REGISTER_FORMAT(CF_SYLK)
		REGISTER_FORMAT(CF_DIF)
		REGISTER_FORMAT(CF_TIFF)
		REGISTER_FORMAT(CF_OEMTEXT)
		REGISTER_FORMAT(CF_DIB)
		REGISTER_FORMAT(CF_PALETTE)
		REGISTER_FORMAT(CF_PENDATA)
		REGISTER_FORMAT(CF_RIFF)
		REGISTER_FORMAT(CF_WAVE)
		REGISTER_FORMAT(CF_UNICODETEXT)
		REGISTER_FORMAT(CF_ENHMETAFILE)
		REGISTER_FORMAT(CF_HDROP)
		REGISTER_FORMAT(CF_LOCALE)
		REGISTER_FORMAT(CF_DIBV5)

		REGISTER_FORMAT(CF_OWNERDISPLAY)
		REGISTER_FORMAT(CF_DSPTEXT)
		REGISTER_FORMAT(CF_DSPBITMAP)
		REGISTER_FORMAT(CF_DSPMETAFILEPICT)
		REGISTER_FORMAT(CF_DSPENHMETAFILE)
	};
	for(unsigned int i = 0; i < (sizeof(defaultMimes)/sizeof(defaultMimes[0])); i++)
	{
		if(mime == defaultMimes[i].name)
		{
			cf = defaultMimes[i].id;
			break;
		}
	}
	if(!cf)
	{
		cf = RegisterClipboardFormatW(reinterpret_cast<const wchar_t*>(mime.utf16()));
		if(!cf)
			qErrnoWarning("RegisterMimeType: Failed to register clipboard format");
	}
	return cf;
}

ClipboardFormat::ClipboardFormat(const QString& n) : name(n)
{
	id = RegisterMimeType(name);
}
ClipboardFormat::~ClipboardFormat()
{
}
QByteArray	ClipboardFormat::getData(IDataObject* pDataObject)
{
	return ::GetData(id, pDataObject);
}
bool		ClipboardFormat::canGetData(const QString& formatName, IDataObject* pDataObject)
{
	return (formatName == name && ::CanGetData(id, pDataObject));
}
