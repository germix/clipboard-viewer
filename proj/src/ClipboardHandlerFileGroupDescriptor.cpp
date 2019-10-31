#include "ClipboardHandler.h"
#include <QTextEdit>

#include <shlobj.h>

QString ClsidToQString(const CLSID& clsid);
QString FileTimeToQString(const FILETIME* ftime);

//
// https://msdn.microsoft.com/en-us/library/bb773290.aspx
//
class ClipboardHandlerFileGroupDescriptor : public ClipboardHandler
{
	ClipboardFormat cbfmtFileGroupDescriptorA;
	ClipboardFormat cbfmtFileGroupDescriptorW;
public:
	ClipboardHandlerFileGroupDescriptor()
		: ClipboardHandler()
		, cbfmtFileGroupDescriptorA("FileGroupDescriptor")
		, cbfmtFileGroupDescriptorW("FileGroupDescriptorW")
	{
	}
	~ClipboardHandlerFileGroupDescriptor()
	{
	}
public:
	virtual QVariant	read(IDataObject* pDataObj, const QString& formatName)
	{
		QVariant ret;
		if(cbfmtFileGroupDescriptorA.canGetData(formatName, pDataObj))
		{
			QString s;
			QByteArray data = cbfmtFileGroupDescriptorA.getData(pDataObj);
			FILEGROUPDESCRIPTORA* fgd = (FILEGROUPDESCRIPTORA*)data.constData();

			for(unsigned int i = 0; i < fgd->cItems; i++)
			{
				s += QString().sprintf("FILEDESCRIPTOR[%d]\n", i);
				s += QString().sprintf("    dwFlags: %x\n", fgd->fgd[i].dwFlags);
				s += QString().sprintf("    clsid: %ls\n", ClsidToQString(fgd->fgd[i].clsid).constData());

				s += QString().sprintf("    sizel.cx: %d\n", fgd->fgd[i].sizel.cx);
				s += QString().sprintf("    sizel.cy: %d\n", fgd->fgd[i].sizel.cy);

				s += QString().sprintf("    pointl.x: %d\n", fgd->fgd[i].pointl.x);
				s += QString().sprintf("    pointl.y: %d\n", fgd->fgd[i].pointl.y);

				s += QString().sprintf("    dwFileAttributes: %x\n", fgd->fgd[i].dwFileAttributes);

				s += QString().sprintf("    ftCreationTime: %ls\n", FileTimeToQString(&fgd->fgd[i].ftCreationTime).constData());
				s += QString().sprintf("    ftLastAccessTime: %ls\n", FileTimeToQString(&fgd->fgd[i].ftLastAccessTime).constData());
				s += QString().sprintf("    ftLastWriteTime: %ls\n", FileTimeToQString(&fgd->fgd[i].ftLastWriteTime).constData());

				s += QString().sprintf("    nFileSizeHigh: %d\n", fgd->fgd[i].nFileSizeHigh);
				s += QString().sprintf("    nFileSizeLow: %d\n", fgd->fgd[i].nFileSizeLow);
				s += QString().sprintf("    cFileName: %s\n", fgd->fgd[i].cFileName);
			}
			return s;
		}
		else if(cbfmtFileGroupDescriptorW.canGetData(formatName, pDataObj))
		{
			QString s;
			QByteArray data = cbfmtFileGroupDescriptorW.getData(pDataObj);
			FILEGROUPDESCRIPTORW* fgd = (FILEGROUPDESCRIPTORW*)data.constData();

			for(int i = 0; i < fgd->cItems; i++)
			{
				s += QString().sprintf("FILEDESCRIPTOR[%d]\n", i);
				s += QString().sprintf("    dwFlags: %x\n", fgd->fgd[i].dwFlags);
				s += QString().sprintf("    clsid: %ls\n", ClsidToQString(fgd->fgd[i].clsid).constData());

				s += QString().sprintf("    sizel.cx: %d\n", fgd->fgd[i].sizel.cx);
				s += QString().sprintf("    sizel.cy: %d\n", fgd->fgd[i].sizel.cy);

				s += QString().sprintf("    pointl.x: %d\n", fgd->fgd[i].pointl.x);
				s += QString().sprintf("    pointl.y: %d\n", fgd->fgd[i].pointl.y);

				s += QString().sprintf("    dwFileAttributes: %x\n", fgd->fgd[i].dwFileAttributes);

				s += QString().sprintf("    ftCreationTime: %ls\n", FileTimeToQString(&fgd->fgd[i].ftCreationTime).constData());
				s += QString().sprintf("    ftLastAccessTime: %ls\n", FileTimeToQString(&fgd->fgd[i].ftLastAccessTime).constData());
				s += QString().sprintf("    ftLastWriteTime: %ls\n", FileTimeToQString(&fgd->fgd[i].ftLastWriteTime).constData());

				s += QString().sprintf("    nFileSizeHigh: %d\n", fgd->fgd[i].nFileSizeHigh);
				s += QString().sprintf("    nFileSizeLow: %d\n", fgd->fgd[i].nFileSizeLow);
				s += QString().sprintf("    cFileName: %ls\n", fgd->fgd[i].cFileName);
			}
			return s;
		}
		return ret;
	}
	virtual bool		canRead(IDataObject* pDataObj, const QString& formatName)
	{
		if(cbfmtFileGroupDescriptorA.canGetData(formatName, pDataObj)
				|| cbfmtFileGroupDescriptorW.canGetData(formatName, pDataObj))
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

////////////////////////////////////////////////////////////////////////////////////////////////////

static ClipboardHandlerFileGroupDescriptor s_ClipboardHandlerFileGroupDescriptor;





