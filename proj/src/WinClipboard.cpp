#include "WinClipboard.h"
#include <tchar.h>
#include <windows.h>
#include <objidl.h>
#include "utils.h"

static const FORMATLOOKUP g_aClipLook[] =
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
//#if (WINVER >= 0x0500)
	REGISTER_FORMAT(CF_DIBV5)
//#else
//#endif
	REGISTER_FORMAT(CF_OWNERDISPLAY)
	REGISTER_FORMAT(CF_DSPTEXT)
	REGISTER_FORMAT(CF_DSPBITMAP)
	REGISTER_FORMAT(CF_DSPMETAFILEPICT)
	REGISTER_FORMAT(CF_DSPENHMETAFILE)

	REGISTER_FORMAT(CF_PRIVATEFIRST)
	//REGISTER_FORMAT(CF_PRIVATELAST)

	REGISTER_FORMAT(CF_GDIOBJFIRST)
	//REGISTER_FORMAT(CF_GDIOBJLAST)
	0, 0,
};
static const FORMATLOOKUP g_aAspectLook[] =
{
	{	DVASPECT_CONTENT,			"Content",		},
	{	DVASPECT_THUMBNAIL,			"Thumbnail",	},
	{	DVASPECT_ICON,				"Icon",			},
	{	DVASPECT_DOCPRINT,			"DocPrint",		},
	{	0, 0,	},
};
static const FORMATLOOKUP g_aTymedLook[] =
{
	{	TYMED_HGLOBAL,				"hGlobal",		},
	{	TYMED_FILE,					"File",			},
	{	TYMED_ISTREAM,				"IStream",		},
	{	TYMED_ISTORAGE,				"IStorage",		},
	{	TYMED_GDI,					"GDI",			},
	{	TYMED_MFPICT,				"MFPict",		},
	{	TYMED_ENHMF,				"ENHMF",		},
	{	TYMED_NULL,					"Null",			},
	{	0, 0,	},
};
static PCSTR lookupFormat(const FORMATLOOKUP* lpLookup, ULONG cfFormat)
{
	for(int i = 0; lpLookup[i].name != NULL; i++)
	{
		if(lpLookup[i].id == cfFormat)
		{
			return lpLookup[i].name;
		}
	}
	return NULL;
}
static QTreeWidgetItem* newFormat(FORMATETC *pfmtetc)
{
	PCSTR ptr;
	WCHAR buf[64];
	QString s;
	QTreeWidgetItem* item;
	static PCSTR szUnknown = "Unknown";

	item = new QTreeWidgetItem();

	// cfFormat
	if(GetClipboardFormatNameW(pfmtetc->cfFormat, buf, 64))
	{
		s = QString::fromWCharArray(buf);
	}
	else
	{
		if(NULL != (ptr = lookupFormat(g_aClipLook, pfmtetc->cfFormat)))
		{
			s = QString::fromLatin1(ptr);
		}
		else
		{
			s = QString("Unknown format (%1)").arg(pfmtetc->cfFormat);
		}
	}
	item->setData(0, Qt::DisplayRole, s);

	s.clear();

	// ptd
	if(pfmtetc->ptd != 0)
	{
		item->setData(1, Qt::DisplayRole, QString().sprintf("%08x", pfmtetc->ptd));
	}

	// dwAspect - Agregar constante DVASPECT_xxx
	if(NULL == (ptr = lookupFormat(g_aAspectLook, pfmtetc->dwAspect)))
	{
		ptr = szUnknown;
	}
	item->setData(2, Qt::DisplayRole, QString::fromLatin1(ptr));

	// lindex - Agregar valor lindex
	item->setData(3, Qt::DisplayRole, QString().sprintf("%d", pfmtetc->lindex));

	// tymed - Agregar valor(es) TYMED
	for(int i = 1; i <= 64; i <<= 2)
	{
		if(pfmtetc->tymed & i)
		{
			if(NULL == (ptr = lookupFormat(g_aTymedLook, i)))
			{
				ptr = szUnknown;
			}
			if(!s.size())
				s += QString().sprintf("%s", ptr);
			else
				s += QString().sprintf(", %s", ptr);
		}
	}
	item->setData(4, Qt::DisplayRole, s);

	item->setData(0, Qt::UserRole, (uint)pfmtetc->cfFormat);

	return item;
}

bool WinClipboard::clear()
{
	bool success = false;

	if(::OpenClipboard(NULL))
	{
		if(::EmptyClipboard())
		{
			success = true;
		}
		::CloseClipboard();
	}
	return success;
}
QList<QTreeWidgetItem*> WinClipboard::enumerate()
{
	IDataObject *pDataObject;
	QList<QTreeWidgetItem*> list;

	if(S_OK == ::OleGetClipboard(&pDataObject))
	{
		IEnumFORMATETC* pEnumFormats;
		if(S_OK == pDataObject->EnumFormatEtc(DATADIR_GET, &pEnumFormats))
		{
			ULONG num;
			FORMATETC fmtetc;
			while(S_OK == pEnumFormats->Next(1, &fmtetc, &num))
			{
				list.append(newFormat(&fmtetc));
			}
			pEnumFormats->Release();
		}
		pDataObject->Release();
	}
	return list;
}

