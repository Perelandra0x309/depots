/* DepotsWindow.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef DEPOTS_WINDOW_H
#define DEPOTS_WINDOW_H

#include <String.h>
#include <StringList.h>
#include <Window.h>

#include "DepotsSettings.h"
#include "DepotsView.h"


class DepotsWindow : public BWindow {
public:
							DepotsWindow(BRect size);
//							~DepotsWindow();
	virtual	bool			QuitRequested();
	virtual void			MessageReceived(BMessage*);
private:
	DepotsSettings			fSettings;
	DepotsView				*fView;
};

#endif
