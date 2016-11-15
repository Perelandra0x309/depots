/* DepotsWindow.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include <Application.h>
#include <Catalog.h>
#include <LayoutBuilder.h>

#include "DepotsWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DepotsWindow"


DepotsWindow::DepotsWindow(BRect size)
	:
	BWindow(size, B_TRANSLATE_SYSTEM_NAME("Depots"), B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE |
		B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS)
{
	fView = new DepotsView();
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.Add(fView)
	.End();
}


bool
DepotsWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}
