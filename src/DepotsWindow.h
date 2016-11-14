/* DepotsWindow.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef DEPOTS_WINDOW_H
#define DEPOTS_WINDOW_H

#include "DepotsView.h"


class DepotsWindow : public BWindow {
public:
							DepotsWindow(BRect size);
	virtual	bool			QuitRequested();
	
private:
	DepotsView				*fView;
};

#endif
