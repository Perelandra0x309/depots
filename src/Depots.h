/* Depots.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef DEPOTS_H
#define DEPOTS_H

#include <Application.h>

#include "DepotsWindow.h"

class DepotsApplication : public BApplication {
public:
						DepotsApplication();
			void		AboutRequested();
private:
	DepotsWindow		*fWindow;
};

#endif
