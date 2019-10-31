#include "ClipboardHandler.h"
#include <QTextEdit>

class ClipboardHandlerPreferredDropEffect : public ClipboardHandler
{
	ClipboardFormat cbfmt;
public:
	ClipboardHandlerPreferredDropEffect()
		: ClipboardHandler()
		, cbfmt("Preferred DropEffect")
	{
	}
	~ClipboardHandlerPreferredDropEffect()
	{
	}
public:
	virtual QVariant	read(IDataObject* pDataObj, const QString& formatName)
	{
		if(cbfmt.canGetData(formatName, pDataObj))
		{
			QByteArray data = cbfmt.getData(pDataObj);
			if(data.size() == 4)
			{
				return data;
			}
		}
		return QVariant();
	}
	virtual bool		canRead(IDataObject* pDataObj, const QString& formatName)
	{
		return cbfmt.canGetData(formatName, pDataObj);
	}
	virtual QWidget*	getWidget(IDataObject* pDataObj, const QString& formatName)
	{
		QVariant var = read(pDataObj, formatName);
		if(!var.isNull())
		{
			QString s1;
			QString s2;
			QByteArray ba = var.toByteArray();
			QDataStream ds(&ba, QIODevice::ReadOnly);
			ds.setByteOrder(QDataStream::LittleEndian);
			quint32 effects;

			ds >> effects;

			s1 += "0x" + QString::number(effects, 16);

			static struct
			{
				quint32 val;
				const char* name;
			}table[] =
			{
			//	{	DROPEFFECT_NONE,	"DROPEFFECT_NONE"	},
				{	DROPEFFECT_COPY,	"DROPEFFECT_COPY"	},
				{	DROPEFFECT_MOVE,	"DROPEFFECT_MOVE"	},
				{	DROPEFFECT_LINK,	"DROPEFFECT_LINK"	},
				{	DROPEFFECT_SCROLL,	"DROPEFFECT_SCROLL"	},
			};
			for(int i = 0; i < sizeof(table)/sizeof(table[0]); i++)
			{
				if(effects & table[i].val)
				{
					effects ^= table[i].val;
					if(!s2.isEmpty())
						s2 += " | ";
					s2 += table[i].name;
				}
			}
			if(effects == 0)
			{
				if(s2.isEmpty())
				{
					s2 = "DROPEFFECT_NONE";
				}
			}
			else
			{
				if(!s2.isEmpty())
					s2 += " | ";
				s2 += "0x" + QString::number(effects, 16);
			}
			QTextEdit* edit = new QTextEdit();
			edit->setReadOnly(true);
			edit->setText("Hex: " + s1 + "\nFlags: " + s2);
			return edit;
		}
		return NULL;
	}
};

static ClipboardHandlerPreferredDropEffect s_ClipboardHandlerPreferredDropEffect;


