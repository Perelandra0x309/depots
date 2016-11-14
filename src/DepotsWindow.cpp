/* DepotsWindow.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "DepotsWindow.h"

DepotsWindow::DepotsWindow(BRect size)
	:
	BWindow(size, "Depots", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE |
		B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS)
{
	
}


bool
MainWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}

