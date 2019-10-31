#include "ClipboardHandler.h"
#include <QTextEdit>

class ClipboardHandlerText : public ClipboardHandler
{
public:
	ClipboardHandlerText()
		: ClipboardHandler()
	{
	}
	~ClipboardHandlerText()
	{
	}
public:
	virtual QVariant	read(IDataObject* pDataObj, const QString& formatName)
	{

		QVariant ret;
		if(canRead(pDataObj, formatName))
		{
#if 0
			QString str;
			QByteArray data = GetData(CF_UNICODETEXT, pDataObj);
			QVariant::Type preferredType = QVariant::String;

			if(!data.isEmpty())
			{
				str = QString::fromWCharArray((const wchar_t *)data.data());
				str.replace(QLatin1String("\r\n"), QLatin1String("\n"));
			}
			else
			{
				data = GetData(CF_TEXT, pDataObj);
				if(!data.isEmpty())
				{
					const char* d = data.data();
					const int s = qstrlen(d);
					QByteArray r(data.size()+1, '\0');
					char* o = r.data();
					int j=0;
					for(int i=0; i<s; i++)
					{
						char c = d[i];
						if (c!='\r')
							o[j++]=c;
					}
					o[j]=0;
					str = QString::fromLocal8Bit(r);
				}
			}
			if(preferredType == QVariant::String)
				ret = str;
			else
				ret = str.toUtf8();

#endif
			QString str;
			QByteArray data;

			if(formatName == "CF_TEXT")
			{
				data = GetData(CF_TEXT, pDataObj);
				if(!data.isEmpty())
				{
					const char* d = data.data();
					const int s = qstrlen(d);
					QByteArray r(data.size()+1, '\0');
					char* o = r.data();
					int j=0;
					for(int i=0; i<s; i++)
					{
						char c = d[i];
						if (c != '\r')
							o[j++]=c;
					}
					o[j] = 0;
					str = QString::fromLocal8Bit(r);
				}
				return str;
			}
			else if(formatName == "CF_UNICODETEXT")
			{
				data = GetData(CF_UNICODETEXT, pDataObj);
				if(!data.isEmpty())
				{
					str = QString::fromWCharArray((const wchar_t *)data.data());
					str.replace(QLatin1String("\r\n"), QLatin1String("\n"));
				}
				return str;
			}
#ifdef USE_OEMTEXT
			else if(formatName == "CF_OEMTEXT")
			{
				data = GetData(CF_OEMTEXT, pDataObj);

				if(!data.isEmpty())
				{
					static WCHAR* wbuf = new WCHAR[0x1000];
					memset(wbuf, 0, sizeof(WCHAR)*0x1000);
					OemToCharBuffW(data.constData(), wbuf, 0x1000);
					str = QString::fromWCharArray((const wchar_t *)wbuf);

					delete [] wbuf;
				}

				return str;
			}
#endif
		}
		return ret;
	}
	virtual bool		canRead(IDataObject* pDataObj, const QString& formatName)
	{

		if(formatName == "CF_TEXT" || formatName == "CF_UNICODETEXT"
#ifdef USE_OEMTEXT
		   || formatName == "CF_OEMTEXT"
#endif
		   )
		{
			return (CanGetData(CF_TEXT, pDataObj)
					|| CanGetData(CF_UNICODETEXT, pDataObj)
#ifdef USE_OEMTEXT
					|| CanGetData(CF_OEMTEXT, pDataObj)
#endif
					);
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
static ClipboardHandlerText s_ClipboardHandlerText;

