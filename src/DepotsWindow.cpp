/* DepotsWindow.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include <Application.h>
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <Screen.h>

#include "AddRepoWindow.h"
#include "constants.h"
#include "DepotsWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DepotsWindow"


DepotsWindow::DepotsWindow(BRect size)
	:
	BWindow(size, B_TRANSLATE_SYSTEM_NAME("Depots"), B_TITLED_WINDOW, B_NOT_ZOOMABLE |
		B_ASYNCHRONOUS_CONTROLS)
{
	fView = new DepotsView();
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.Add(fView)
	.End();
	
	// Size and location on screen
	BSize viewSize(fView->MinSize());
	SetSizeLimits(viewSize.Width(), 9999, viewSize.Height(), 9999);
	BRect frame = fSettings.GetFrame();
	ResizeTo(frame.Width(), frame.Height());
	BScreen screen;
	BRect screenFrame = screen.Frame();
	if(screenFrame.right < frame.right || screenFrame.left > frame.left
		|| screenFrame.top > frame.top || screenFrame.bottom < frame.bottom)
		CenterOnScreen();
	else
		MoveTo(frame.left, frame.top);
	Show();
}

/*
DepotsWindow::~DepotsWindow()
{

}*/


bool
DepotsWindow::QuitRequested()
{
	fSettings.SetFrame(Frame());
	be_app->PostMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}


void
DepotsWindow::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		//TODO capture pkgman commands run while Depots application is running
		
		case ADD_REPO_WINDOW: {
			new AddRepoWindow(Frame(), this);
			break;
		}
		case ADD_REPO_URL: {
			BString url;
			status_t result = msg->FindString(key_url, &url);
			if(result == B_OK)
				fView->AddManualRepository(url);
			break;
		}
		case LIST_SELECTION_CHANGED: 
		case UPDATE_LIST: {
			fView->MessageReceived(msg);
			break;
		}
		default:
			BWindow::MessageReceived(msg);
	}
}
