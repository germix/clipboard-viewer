#if 1
#include "ClipboardHandler.h"
#include <QTableWidget>
#include <QTextStream>

// https://en.wikipedia.org/wiki/Data_Interchange_Format
// https://en.wikipedia.org/wiki/Talk%3AData_Interchange_Format
// http://www.fileformat.info/format/dif/egff.htm
// http://www.hep.by/gnu/gnumeric/file-format-dif.shtml

class ClipboardHandlerDIF : public ClipboardHandler
{
	ClipboardFormat cbfmtDIF;
public:
	ClipboardHandlerDIF()
		: ClipboardHandler()
		, cbfmtDIF("CF_DIF")
	{
	}
	~ClipboardHandlerDIF()
	{
	}
public:
	virtual QVariant	read(IDataObject* pDataObj, const QString& formatName)
	{
		if(cbfmtDIF.canGetData(formatName, pDataObj))
		{
			return cbfmtDIF.getData(pDataObj);
		}
		return QVariant();
	}
	virtual bool		canRead(IDataObject* pDataObj, const QString& formatName)
	{
		return cbfmtDIF.canGetData(formatName, pDataObj);
	}
	virtual QWidget*	getWidget(IDataObject* pDataObj, const QString& formatName)
	{
		QVariant var = read(pDataObj, formatName);
		if(var.isValid())
		{
			QString data = var.toString();
			QTextStream text(&data);
			int currentRow = 0;
			QTableWidget* table = new QTableWidget();


			while(!text.atEnd())
			{
				QString s = text.readLine();
				if(s == "TABLE")
				{
					QString version = text.readLine();
					QString comment = text.readLine();

					qDebug("TABLE:");
					qDebug("  version: %ls", version.constData());
					qDebug("  comment: %ls", comment.constData());
				}
				else if(s == "VECTORS")
				{
					QString l1 = text.readLine();
					QString l2 = text.readLine();

					QStringList sl = l1.split(",");
					table->setRowCount(sl.at(1).toInt());
				}
				else if(s == "TUPLES")
				{
					QString l1 = text.readLine();
					QString l2 = text.readLine();

					QStringList sl = l1.split(",");
					table->setColumnCount(sl.at(1).toInt());
				}
				else if(s == "DATA")
				{
#if 1
					text.readLine();
					text.readLine();
#else
					QString l1 = text.readLine();
					QString l2 = text.readLine();
#endif
					while(!text.atEnd())
					{
						QString l1 = text.readLine();
						QString str = text.readLine();
						QString dataType = l1.split(",").at(0);

						//qDebug("dataType: %ls", dataType.constData());
						if(dataType == "-1")
						{
							if(str == "BOT")
							{
								for(int i = 0; i < table->columnCount(); i++)
								{
									QString s1 = text.readLine();
									QString s2 = text.readLine();

									s2 = s2.remove(0, 1);
									s2 = s2.remove(s2.size()-1, 1);
									QTableWidgetItem* item = new QTableWidgetItem(s2);

									//qDebug("s1: %ls", s1.constData());
									//qDebug("s2: %ls", s2.constData());

									item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
									table->setItem(currentRow, i, item);
								}
								currentRow++;
							}
							else if(str == "EOD")
							{
								qDebug("EOD");
								break;
							}
						}
						else if(dataType == "0")
						{
							// TODO
						}
						else if(dataType == '1')
						{
							// TODO
						}
					}
				}
			}
			return table;
		}
		return NULL;
	}
};

static ClipboardHandlerDIF s_ClipboardHandlerDIF;


#endif
