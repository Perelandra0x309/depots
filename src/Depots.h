/* Depots.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef DEPOTS_H
#define DEPOTS_H


#include <Application.h>
#include <Point.h>
#include <Rect.h>
#include <StringView.h>

#include "DepotsWindow.h"


class URLView : public BStringView {
public:
							URLView(const char *name, const char *url);
	virtual void			DetachedFromWindow();
	virtual void			MouseMoved(BPoint where, uint32 code,
								const BMessage *dragMessage);
	virtual void			MouseDown(BPoint point);
	virtual void			Draw(BRect bounds);

private:
	BString					fUrl;
	bool					fMouseOver;
};

class DepotsApplication : public BApplication {
public:
							DepotsApplication();
	virtual void			AboutRequested();

private:
	DepotsWindow			*fWindow;
};

#endif
