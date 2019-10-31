#include "ClipboardHandler.h"
#include <QTableWidget>

class ClipboardHandlerCSV : public ClipboardHandler
{
	ClipboardFormat cbfmtCSV;
public:
	ClipboardHandlerCSV()
		: ClipboardHandler()
		, cbfmtCSV("Csv")
	{
	}
	~ClipboardHandlerCSV()
	{
	}
public:
	virtual QVariant	read(IDataObject* pDataObj, const QString& formatName)
	{
		if(cbfmtCSV.canGetData(formatName, pDataObj))
		{
			return cbfmtCSV.getData(pDataObj);
		}
		return QVariant();
	}
	virtual bool		canRead(IDataObject* pDataObj, const QString& formatName)
	{
		return cbfmtCSV.canGetData(formatName, pDataObj);
	}
	virtual QWidget*	getWidget(IDataObject* pDataObj, const QString& formatName)
	{
		QVariant var = read(pDataObj, formatName);
		if(var.isValid())
		{
			QTableWidget* table = new QTableWidget();
			QString data = var.toString();
			QStringList rowOfData = data.split("\r\n");

			int c = rowOfData.count();
			if(c > 0 && rowOfData.at(c-1).isEmpty())
				c--;

			table->setRowCount(c);

			for(int x = 0; x < rowOfData.size(); x++)
			{
				QStringList rowData = rowOfData.at(x).split(";");
				table->setColumnCount(qMax(rowData.count(), table->columnCount()));
				if(rowData.isEmpty())
					break;
				for(int y = 0; y < rowData.size(); y++)
				{
					QTableWidgetItem* item = new QTableWidgetItem(rowData[y]);
					item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
					table->setItem(x, y, item);
				}
			}
			return table;
		}
		return NULL;
	}
};

static ClipboardHandlerCSV s_ClipboardHandlerCSV;




