/* AddRepoWindow.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include <Alert.h>
#include <Application.h>
#include <Catalog.h>
#include <LayoutBuilder.h>

#include "AddRepoWindow.h"
#include "constants.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "AddRepoWindow"


AddRepoWindow::AddRepoWindow(BRect size, BLooper *looper)
	:
	BWindow(size, "AddWindow", B_BORDERED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL,
				B_NOT_RESIZABLE | B_NOT_MOVABLE | B_ASYNCHRONOUS_CONTROLS
				| B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
	fReplyLooper(looper)
{
	fView = new BView("view", B_SUPPORTS_LAYOUT);
	fText = new BTextControl("text", B_TRANSLATE_COMMENT("Depot URL:", "Text box label"), "", new BMessage(ADD_BUTTON_PRESSED));
	fAddButton = new BButton(B_TRANSLATE_COMMENT("Add", "Button label"), new BMessage(ADD_BUTTON_PRESSED));
	fAddButton->MakeDefault(true);
	fCancelButton = new BButton(kCancelLabel, new BMessage(CANCEL_BUTTON_PRESSED));
	SetWidth(size.Width());
	
	BLayoutBuilder::Group<>(fView, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING)
		.Add(fText)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.AddGlue()
			.Add(fCancelButton)
			.Add(fAddButton);
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.Add(fView);
	fText->MakeFocus();
	
	// Move to bottom of window
	Layout(true);
	MoveTo(size.left, size.bottom - Frame().Height());
	Show();
}


void
AddRepoWindow::Quit()
{
	fReplyLooper->PostMessage(ADD_WINDOW_CLOSED);
	BWindow::Quit();
}


void
AddRepoWindow::SetWidth(float width)
{
	Lock();
	fView->SetExplicitSize(BSize(width, B_SIZE_UNSET));
	Unlock();
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
				// URL must have a protocol
				if(url.FindFirst("://") == B_ERROR)
				{
					BAlert *alert = new BAlert("error", B_TRANSLATE_COMMENT("The URL must start with a protocol, "
									"for example http:// or https://", "Add URL error message"), kOKLabel,
										NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
					alert->SetFeel(B_MODAL_APP_WINDOW_FEEL);
				//	alert->Go(&fDummyInvoker);
					BRect windowFrame = be_app->WindowAt(0)->Frame();
					alert->Go(NULL);
					alert->CenterIn(windowFrame);
				}
				else
				{
					BMessage *addMsg = new BMessage(ADD_REPO_URL);
					addMsg->AddString(key_url, url);
					fReplyLooper->PostMessage(addMsg);
					Quit();
				}
			}
			break;
		}
		default:
			BWindow::MessageReceived(msg);
	}
}
