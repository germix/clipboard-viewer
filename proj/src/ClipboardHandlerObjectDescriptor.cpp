#include "ClipboardHandler.h"
#include <QTextEdit>

QString ClsidToQString(const CLSID& clsid);

//
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms683735(v=vs.85).aspx
//
#ifndef __IOLETypes_INTERFACE_DEFINED__
typedef struct tagOBJECTDESCRIPTOR {
  ULONG  cbSize;
  CLSID  clsid;
  DWORD  dwDrawAspect;
  SIZEL  sizel;
  POINTL pointl;
  DWORD  dwStatus;
  DWORD  dwFullUserTypeName;
  DWORD  dwSrcOfCopy;
} OBJECTDESCRIPTOR, *LPOBJECTDESCRIPTOR, *POBJECTDESCRIPTOR;
#endif

//
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms683735(v=vs.85).aspx
//
#ifndef __IOleObject_INTERFACE_DEFINED__
typedef enum tagOLEMISC {
  OLEMISC_RECOMPOSEONRESIZE             = 0x1,
  OLEMISC_ONLYICONIC                    = 0x2,
  OLEMISC_INSERTNOTREPLACE              = 0x4,
  OLEMISC_STATIC                        = 0x8,
  OLEMISC_CANTLINKINSIDE                = 0x10,
  OLEMISC_CANLINKBYOLE1                 = 0x20,
  OLEMISC_ISLINKOBJECT                  = 0x40,
  OLEMISC_INSIDEOUT                     = 0x80,
  OLEMISC_ACTIVATEWHENVISIBLE           = 0x100,
  OLEMISC_RENDERINGISDEVICEINDEPENDENT  = 0x200,
  OLEMISC_INVISIBLEATRUNTIME            = 0x400,
  OLEMISC_ALWAYSRUN                     = 0x800,
  OLEMISC_ACTSLIKEBUTTON                = 0x1000,
  OLEMISC_ACTSLIKELABEL                 = 0x2000,
  OLEMISC_NOUIACTIVATE                  = 0x4000,
  OLEMISC_ALIGNABLE                     = 0x8000,
  OLEMISC_SIMPLEFRAME                   = 0x10000,
  OLEMISC_SETCLIENTSITEFIRST            = 0x20000,
  OLEMISC_IMEMODE                       = 0x40000,
  OLEMISC_IGNOREACTIVATEWHENVISIBLE     = 0x80000,
  OLEMISC_WANTSTOMENUMERGE              = 0x100000,
  OLEMISC_SUPPORTSMULTILEVELUNDO        = 0x200000
} OLEMISC;
#endif

class ClipboardHandlerObjectDescriptor : public ClipboardHandler
{
	ClipboardFormat cbfmtObjectDescriptor;
	ClipboardFormat cbfmtLinkSourceDescriptor;
public:
	ClipboardHandlerObjectDescriptor()
		: ClipboardHandler()
		, cbfmtObjectDescriptor("Object Descriptor")
		, cbfmtLinkSourceDescriptor("Link Source Descriptor")
	{
	}
	~ClipboardHandlerObjectDescriptor()
	{
	}
public:
	virtual QVariant	read(IDataObject* pDataObj, const QString& formatName)
	{
		if(canRead(pDataObj, formatName))
		{
			QByteArray data;
			if(formatName == "Object Descriptor")
				data = cbfmtObjectDescriptor.getData(pDataObj);
			else
				data = cbfmtLinkSourceDescriptor.getData(pDataObj);

			if(!data.isEmpty())
			{
				QString s;
				OBJECTDESCRIPTOR* od = (OBJECTDESCRIPTOR*)data.constData();

				s += QString().sprintf("cbSize           = %u\n", od->cbSize);
				s += QString().sprintf("clsid            = %ls\n", ClsidToQString(od->clsid).constData());

				s += QString().sprintf("dwDrawAspect     = %ls\n", toDrawAspect(od->dwDrawAspect).constData());

				s += QString().sprintf("sizel            = (%d,%d)\n", (int)od->sizel.cx, (int)od->sizel.cy);
				s += QString().sprintf("pointl           = (%d,%d)\n", (int)od->pointl.x, (int)od->pointl.y);
				s += QString().sprintf("dwStatus         = %ls\n", toStatus(od->dwStatus).constData());
				s += QString().sprintf("FullUserTypeName = %ls\n", data.constData() + od->dwFullUserTypeName);
				s += QString().sprintf("SrcOfCopy        = %ls\n", data.constData() + od->dwSrcOfCopy);

				return s;
			}
		}
		return QVariant();
	}
	virtual bool		canRead(IDataObject* pDataObj, const QString& formatName)
	{
		return cbfmtObjectDescriptor.canGetData(formatName, pDataObj)
				|| cbfmtLinkSourceDescriptor.canGetData(formatName, pDataObj);
	}
	virtual QWidget*	getWidget(IDataObject* pDataObj, const QString& formatName)
	{
		QVariant data = read(pDataObj, formatName);
		if(data.isValid())
		{
			QTextEdit* edit = new QTextEdit();
			edit->setReadOnly(true);
			edit->setFont(QFont("Courier New"));
			edit->setText(data.toString());
			return edit;
		}
		return NULL;
	}
private:
	QString toDrawAspect(DWORD dwDrawAspect)
	{
		QString s;

		switch(dwDrawAspect)
		{
			case DVASPECT_CONTENT:		s = "DVASPECT_CONTENT";		break;
			case DVASPECT_THUMBNAIL:	s = "DVASPECT_THUMBNAIL";	break;
			case DVASPECT_ICON:			s = "DVASPECT_ICON";		break;
			case DVASPECT_DOCPRINT:		s = "DVASPECT_DOCPRINT";	break;
		}
		s += "(" + QString::number(dwDrawAspect) + ")";
		return s;
	}
	QString toStatus(DWORD dwStatus)
	{
		#define VALUE_STRINGIFIED(v) { v, #v },

		static struct
		{
			unsigned long value;
			const char* name;
		}
		table[] =
		{
			VALUE_STRINGIFIED(OLEMISC_RECOMPOSEONRESIZE)
			VALUE_STRINGIFIED(OLEMISC_ONLYICONIC)
			VALUE_STRINGIFIED(OLEMISC_INSERTNOTREPLACE)
			VALUE_STRINGIFIED(OLEMISC_STATIC)
			VALUE_STRINGIFIED(OLEMISC_CANTLINKINSIDE)
			VALUE_STRINGIFIED(OLEMISC_CANLINKBYOLE1)
			VALUE_STRINGIFIED(OLEMISC_ISLINKOBJECT)
			VALUE_STRINGIFIED(OLEMISC_INSIDEOUT)
			VALUE_STRINGIFIED(OLEMISC_ACTIVATEWHENVISIBLE)
			VALUE_STRINGIFIED(OLEMISC_RENDERINGISDEVICEINDEPENDENT)
			VALUE_STRINGIFIED(OLEMISC_INVISIBLEATRUNTIME)
			VALUE_STRINGIFIED(OLEMISC_ALWAYSRUN)
			VALUE_STRINGIFIED(OLEMISC_ACTSLIKEBUTTON)
			VALUE_STRINGIFIED(OLEMISC_ACTSLIKELABEL)
			VALUE_STRINGIFIED(OLEMISC_NOUIACTIVATE)
			VALUE_STRINGIFIED(OLEMISC_ALIGNABLE)
			VALUE_STRINGIFIED(OLEMISC_SIMPLEFRAME)
			VALUE_STRINGIFIED(OLEMISC_SETCLIENTSITEFIRST)
			VALUE_STRINGIFIED(OLEMISC_IMEMODE)
			VALUE_STRINGIFIED(OLEMISC_IGNOREACTIVATEWHENVISIBLE)
			VALUE_STRINGIFIED(OLEMISC_WANTSTOMENUMERGE)
			VALUE_STRINGIFIED(OLEMISC_SUPPORTSMULTILEVELUNDO)
		};
		QString s;

		for(unsigned int i = 0; i < sizeof(table)/sizeof(table[0]); i++)
		{
			if(dwStatus & table[i].value)
			{
				if(!s.isEmpty())
					s += "|";
				s += table[i].name;
			}
		}
		return s;
	}
};
static ClipboardHandlerObjectDescriptor s_ClipboardHandlerObjectDescriptor;

