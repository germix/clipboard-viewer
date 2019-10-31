#include <QString>
#include "utils.h"

QString ClsidToQString(const CLSID& clsid)
{
	QString s;
#if 0
	s = QString().sprintf("{%X-%X-%X-%X%X-%X%X%X%X%X%X}",
		od->clsid.Data1,
		od->clsid.Data2,
		od->clsid.Data3,
		od->clsid.Data4[0],
		od->clsid.Data4[1],
		od->clsid.Data4[2],
		od->clsid.Data4[3],
		od->clsid.Data4[4],
		od->clsid.Data4[5],
		od->clsid.Data4[6],
		od->clsid.Data4[7]);
#else
	// https://msdn.microsoft.com/en-us/library/windows/desktop/ms683917(v=vs.85).aspx
	WCHAR* clsidString;
	StringFromCLSID(clsid, &clsidString);
	s = QString().sprintf("%ls", clsidString);
	CoTaskMemFree(clsidString);
#endif
	return s;
}


QString FileTimeToQString(const FILETIME* ftime)
{
	SYSTEMTIME utc;
	SYSTEMTIME local;

	FileTimeToSystemTime(ftime, &utc);

	return QString().sprintf("%02d/%02d/%d  %02d:%02d",
					  utc.wMonth, utc.wDay, utc.wYear,
					  utc.wHour, utc.wMinute);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int			getCf(const FORMATETC& formatetc)
{
	return formatetc.cfFormat;
}
FORMATETC	setCf(int cf)
{
	FORMATETC formatetc;
	formatetc.cfFormat = cf;
	formatetc.dwAspect = DVASPECT_CONTENT;
	formatetc.lindex = -1;
	formatetc.ptd = NULL;
	formatetc.tymed = TYMED_HGLOBAL;
	return formatetc;
}
bool		SetData(const QByteArray& data, STGMEDIUM* pmedium)
{
	HGLOBAL hData = GlobalAlloc(0, data.size());
	if (!hData)
		return false;

	void *out = GlobalLock(hData);
	memcpy(out, data.data(), data.size());
	GlobalUnlock(hData);
	pmedium->tymed = TYMED_HGLOBAL;
	pmedium->hGlobal = hData;
	pmedium->pUnkForRelease = 0;
	return true;
}
QByteArray	GetData(int cf)
{
	QByteArray data;
	IDataObject *pDataObj;

	if(S_OK == OleGetClipboard(&pDataObj))
	{
		data = GetData(cf, pDataObj);
		pDataObj->Release();
	}
	return data;
}
QByteArray	GetData(int cf, IDataObject* pDataObject)
{
	QByteArray data;
	FORMATETC formatetc = setCf(cf);
	STGMEDIUM s;
#if 0
	if(pDataObject->QueryGetData(&formatetc) == S_OK)
	{
		if(pDataObject->GetData(&formatetc, &s) == S_OK)
		{
			DWORD * val = (DWORD*)GlobalLock(s.hGlobal);
			data = QByteArray::fromRawData((char*)val, GlobalSize(s.hGlobal));
			data.detach();
			GlobalUnlock(s.hGlobal);
			ReleaseStgMedium(&s);

			return data;
		}
	}
	//
	//Try reading IStream data
	//
	if(data.size() == 0)
	{
		qDebug("Try reading IStream data");
		formatetc.tymed = TYMED_ISTREAM;
		if(pDataObject->QueryGetData(&formatetc) == S_OK)
		{
			if(pDataObject->GetData(&formatetc, &s) == S_OK)
			{
				char szBuffer[4096];
				ULONG actualRead = 0;
				LARGE_INTEGER pos = {{0, 0}};
				//Move to front (can fail depending on the data model implemented)
				HRESULT hr = s.pstm->Seek(pos, STREAM_SEEK_SET, NULL);
				while(SUCCEEDED(hr)){
					hr = s.pstm->Read(szBuffer, sizeof(szBuffer), &actualRead);
					if (SUCCEEDED(hr) && actualRead > 0) {
						data += QByteArray::fromRawData(szBuffer, actualRead);
					}
					if (actualRead != sizeof(szBuffer))
						break;
				}
				data.detach();
				ReleaseStgMedium(&s);
			}
		}
	}
#else
	formatetc.tymed = TYMED_HGLOBAL;
	if(pDataObject->QueryGetData(&formatetc) == S_OK)
	{
		if(pDataObject->GetData(&formatetc, &s) == S_OK)
		{
			if(s.tymed == TYMED_HGLOBAL)
			{
				DWORD * val = (DWORD*)GlobalLock(s.hGlobal);
				data = QByteArray::fromRawData((char*)val, GlobalSize(s.hGlobal));
				data.detach();
				GlobalUnlock(s.hGlobal);
				ReleaseStgMedium(&s);

				return data;
			}
		}
	}
	//
	//Try reading IStream data
	//
	if(data.size() == 0)
	{
		qDebug("Try reading IStream data");
		formatetc.tymed = TYMED_ISTREAM;
		if(pDataObject->QueryGetData(&formatetc) == S_OK)
		{
			if(pDataObject->GetData(&formatetc, &s) == S_OK)
			{
				if(s.tymed == TYMED_ISTREAM)
				{
					char szBuffer[4096];
					ULONG actualRead = 0;
					LARGE_INTEGER pos = {{0, 0}};
					//Move to front (can fail depending on the data model implemented)
					HRESULT hr = s.pstm->Seek(pos, STREAM_SEEK_SET, NULL);
					while(SUCCEEDED(hr))
					{
						hr = s.pstm->Read(szBuffer, sizeof(szBuffer), &actualRead);
						if(SUCCEEDED(hr) && actualRead > 0)
						{
							data += QByteArray::fromRawData(szBuffer, actualRead);
						}
						if(actualRead != sizeof(szBuffer))
							break;
					}
					data.detach();
					ReleaseStgMedium(&s);
				}
			}
		}
	}
#endif
	return data;
}
bool		CanGetData(int cf, IDataObject* pDataObject)
{
	FORMATETC fetc = setCf(cf);
	if(pDataObject->QueryGetData(&fetc) != S_OK)
	{
		fetc.tymed = TYMED_ISTREAM;
		return pDataObject->QueryGetData(&fetc) == S_OK;
	}
	return true;
}
bool		CanGetData(int cf, IDataObject* pDataObject, DWORD tymed)
{
	FORMATETC fetc;

	fetc.cfFormat	 = cf;
	fetc.dwAspect	 = DVASPECT_CONTENT;
	fetc.lindex		 = -1;
	fetc.ptd		 = NULL;
	fetc.tymed		 = tymed;
	if(pDataObject->QueryGetData(&fetc) == S_OK)
		return true;
	return false;
}
#include "ClipboardHandler.h"


QWidget*	GetDisplayForClipboardData(const QString& formatName)
{
	qDebug("GetDisplayForClipboardData(%ls)", formatName.data());
	IDataObject *pDataObj;

	if(S_OK == OleGetClipboard(&pDataObj))
	{
		QWidget* widget = 0;
		QList<ClipboardHandler*>* handlers = ClipboardHandler::HandlerList();

		for(int i = 0; i < handlers->size(); i++)
		{
			if(NULL != (widget = handlers->at(i)->getWidget(pDataObj, formatName)))
			{
				break;
			}
		}
		pDataObj->Release();

		return widget;
	}
	return NULL;
}


