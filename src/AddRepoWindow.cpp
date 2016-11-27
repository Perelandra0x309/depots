/* AddRepoWindow.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include <Catalog.h>
#include <LayoutBuilder.h>


#include "AddRepoWindow.h"
#include "constants.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "AddRepoWindow"


AddRepoWindow::AddRepoWindow(BRect size, BLooper *looper)
	:
	BWindow(size, "AddWindow", B_MODAL_WINDOW, //B_NOT_V_RESIZABLE | 
		B_ASYNCHRONOUS_CONTROLS |  B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
	msgLooper(looper)
{
	fView = new BView("view", B_SUPPORTS_LAYOUT);
	fView->SetExplicitMinSize(BSize(size.Width(), B_SIZE_UNSET));
	fText = new BTextControl("text", "Repository url:", "", new BMessage(ADD_BUTTON_PRESSED));
	fAddButton = new BButton("Add", new BMessage(ADD_BUTTON_PRESSED));
	fCancelButton = new BButton("Cancel", new BMessage(CANCEL_BUTTON_PRESSED));
	
	BLayoutBuilder::Group<>(fView, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING)
		.Add(fText)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(fAddButton)
			.Add(fCancelButton);
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.Add(fView);
	fText->MakeFocus();
	
	// Location on screen
	MoveTo(size.left + 20, size.top + 20);
	Show();
}


void
AddRepoWindow::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case CANCEL_BUTTON_PRESSED: {
			if(QuitRequested())
				Quit();
			break;
		}
		case ADD_BUTTON_PRESSED: {
			BString url(fText->Text());
			if(url != "")
			{
				BMessage *addMsg = new BMessage(ADD_REPO_URL);
				addMsg->AddString(key_url, url);
				msgLooper->PostMessage(addMsg);
				Quit();
			}
			break;
		}
		default:
			BWindow::MessageReceived(msg);
	}
}
