#include "ClipboardHandler.h"
#include <shlobj.h>
#include <QTreeWidget>
#include <QHeaderView>

class ClipboardHandlerPath : public ClipboardHandler
{
	ClipboardFormat cbfmtDrop;
	ClipboardFormat cbfmtFileName;
	ClipboardFormat cbfmtFileNameW;
	ClipboardFormat cbfmtShellIDListArray;
public:
	ClipboardHandlerPath()
		: ClipboardHandler()
		, cbfmtDrop("CF_HDROP")
		, cbfmtFileName("FileName")
		, cbfmtFileNameW("FileNameW")
		, cbfmtShellIDListArray("Shell IDList Array")
	{
	}
	~ClipboardHandlerPath()
	{
	}
public:
	virtual QVariant	read(IDataObject* pDataObj, const QString& formatName);
	virtual bool		canRead(IDataObject* pDataObj, const QString& formatName);
	virtual QWidget*	getWidget(IDataObject* pDataObj, const QString& formatName);
};

static ClipboardHandlerPath s_ClipboardHandlerPath;

QVariant	ClipboardHandlerPath::read(IDataObject* pDataObj, const QString& formatName)
{
#if 0	//!!!
	if(formatName == cbfmtDrop.name)
	{
		FORMATETC fetc;
		STGMEDIUM stgm;

		// ...
		fetc.cfFormat		 = CF_HDROP;
		fetc.ptd			 = NULL;
		fetc.dwAspect		 = DVASPECT_CONTENT;
		fetc.lindex			 = -1;
		fetc.tymed			 = TYMED_HGLOBAL;
		// ...
		stgm.tymed			 = TYMED_HGLOBAL;
		stgm.hGlobal		 = NULL;
		stgm.pUnkForRelease	 = NULL;
		if(S_OK == pDataObj->QueryGetData(&fetc))
		{
			HDROP hdrop;
			QStringList pathList;
			WCHAR fileName[MAX_PATH];

			if(S_OK == pDataObj->GetData(&fetc, &stgm))
			{
				hdrop = (HDROP)stgm.hGlobal;
				for(int i = 0; DragQueryFileW(hdrop, i, fileName, MAX_PATH); i++)
				{
					pathList.append(QString::fromWCharArray(fileName));
				}
				DragFinish(hdrop);

				ReleaseStgMedium(&stgm);

				return QVariant(pathList);
			}
		}
	}
#else
	if(cbfmtDrop.canGetData(formatName, pDataObj))
	{
		QByteArray data = cbfmtDrop.getData(pDataObj);
		if(data.isEmpty())
			return QVariant();
		QString fileName;
		QStringList pathList;

		LPDROPFILES hdrop = (LPDROPFILES)data.data();
		if(!hdrop->fWide)
		{
			int i = 0;
			const char* files = (const char *)data.data() + hdrop->pFiles;

			while(files[i])
			{
				fileName = QString::fromLocal8Bit(files+i);
				pathList += fileName;
				i += fileName.length()+1;
			}
		}
		else
		{
			int i = 0;
			const wchar_t* filesW = (const wchar_t *)(data.data() + hdrop->pFiles);

			while(filesW[i])
			{
				fileName = QString::fromWCharArray(filesW + i);
				pathList += fileName;
				i += fileName.length()+1;
			}
		}
		return pathList;
	}
#endif
	else if(cbfmtFileName.canGetData(formatName, pDataObj))
	{
		QByteArray data = cbfmtFileName.getData(pDataObj);
		QStringList pathList;

		pathList.append(QString::fromLatin1((const char*)data.data()));
		return QVariant(pathList);
	}
	else if(cbfmtFileNameW.canGetData(formatName, pDataObj))
	{
		QByteArray data = cbfmtFileNameW.getData(pDataObj);
		QStringList pathList;

		pathList.append(QString::fromWCharArray((const wchar_t*)data.data()));
		return QVariant(pathList);
	}
	else if(cbfmtShellIDListArray.canGetData(formatName, pDataObj))
	{
		//
		// http://winapi.freetechsecrets.com/win32/WIN32Shell_IDList_Array_Format.htm
		// http://winapi.freetechsecrets.com/win32/WIN32CIDA.htm
		//
		QByteArray data = cbfmtShellIDListArray.getData(pDataObj);
		QStringList pathList;
		const char* buf = data.constData();
		CIDA* cida = (CIDA*)buf;
		ITEMIDLIST* itemIdList;

		// Me salteo el primero porque es la carpeta
		for(unsigned int i = 1; i < cida->cidl+1; i++)
		{
			itemIdList = (ITEMIDLIST*)(buf + cida->aoffset[i]);
			WCHAR sz[MAX_PATH];
			SHGetPathFromIDList(itemIdList, sz);
			pathList.append(QString((QChar*)sz, wcslen(sz)));
		}
		return pathList;
	}
	return QVariant();
}
bool		ClipboardHandlerPath::canRead(IDataObject* pDataObj, const QString& formatName)
{
	return (cbfmtDrop.canGetData(formatName, pDataObj)
			|| cbfmtFileName.canGetData(formatName, pDataObj)
			|| cbfmtFileNameW.canGetData(formatName, pDataObj)
			|| cbfmtShellIDListArray.canGetData(formatName, pDataObj));
}
QWidget*	ClipboardHandlerPath::getWidget(IDataObject* pDataObj, const QString& formatName)
{
	QVariant data = read(pDataObj, formatName);
	if(data.isValid())
	{
		QTreeWidget* tree = new QTreeWidget(0);
		QTreeWidgetItem* item;
		QStringList fileList = data.toStringList();
		QList<QTreeWidgetItem*> itemList;

		tree->header()->setVisible(false);

		for(int i = 0; i < fileList.size(); i++)
		{
			QString name = fileList.at(i);

			item = new QTreeWidgetItem();
			item->setData(0, Qt::DisplayRole, name);
			itemList.append(item);
		}
		tree->addTopLevelItems(itemList);
		return tree;
	}
	return NULL;
}


