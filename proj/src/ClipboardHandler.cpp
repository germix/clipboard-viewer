#include "ClipboardHandler.h"

ClipboardHandler::ClipboardHandler()
{
	HandlerList()->append(this);
}
ClipboardHandler::~ClipboardHandler()
{
	HandlerList()->removeOne(this);
}
QList<ClipboardHandler*>* ClipboardHandler::HandlerList()
{
	static QList<ClipboardHandler*>* sClipboardHandlers = 0;

	if(!sClipboardHandlers)
		sClipboardHandlers = new QList<ClipboardHandler*>();
	return sClipboardHandlers;
}


