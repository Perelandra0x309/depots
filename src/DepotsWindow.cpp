/* DepotsWindow.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include <Application.h>
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <Screen.h>

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
	
	// Location on screen
	MoveTo(fSettings.GetLocation());
	BScreen screen;
	BRect screenFrame = screen.Frame();
	BRect frame = Frame();
	if(screenFrame.right < frame.right || screenFrame.left > frame.left
		|| screenFrame.top > frame.top || screenFrame.bottom < frame.bottom)
		CenterOnScreen();
	
	Show();
}


bool
DepotsWindow::QuitRequested()
{
	fSettings.SetLocation(Frame().LeftTop());
	be_app->PostMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}
