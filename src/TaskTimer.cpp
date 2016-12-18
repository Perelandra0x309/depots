/* TaskTimer.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
//#include <Alert.h>
#include <Button.h>
#include <Catalog.h>
#include <LayoutBuilder.h>
//#include <View.h>

#include "constants.h"
#include "TaskTimer.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "TaskTimer"


TaskTimer::TaskTimer(int32 seconds)
	:
//	BWindow(BRect(100,100,300,200), "TimerWindow", B_MODAL_WINDOW,
//		B_ASYNCHRONOUS_CONTROLS |  B_AUTO_UPDATE_SIZE_LIMITS),
	BLooper(),
	fSeconds(seconds),
	fIsStopped(true),
	fMsgRunner(NULL),
	fTimeoutMessage(TASK_TIMEOUT),
	fTimeoutAlert(NULL)
{
/*	fSkipButton = new BButton(B_TRANSLATE_COMMENT("Skip", "Button label"),
								new BMessage(TASK_TIMER_SKIP_BUTTON));
	fCancelAllButton = new BButton(B_TRANSLATE_COMMENT("Cancel All", "Button label"),
								new BMessage(TASK_TIMER_CANCEL_ALL_BUTTON));
	BLayoutBuilder::Group<>(this, B_HORIZONTAL)
		.SetInsets(B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING)
		.Add(fCancelAllButton)
		.Add(fSkipButton);
	SetDefaultButton(fSkipButton);*/
	Run();
}


bool
TaskTimer::QuitRequested()
{
//	Hide();
	return true;
}


void
TaskTimer::MessageReceived(BMessage *msg)
{
	switch(msg->what)
	{
		case TASK_TIMEOUT: {
		//	Show();
			fMsgRunner = NULL;
			fTimeoutAlert = new BAlert("timeout", "Task timed out.", "Keep Trying", "Cancel task", NULL,
											B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
			fTimeoutAlert->SetShortcut(0, B_ESCAPE);
			fTimeoutAlert->Go(&fTimeoutAlertInvoker);
			break;
		}
		case TIMEOUT_ALERT_BUTTON_SELECTION: {
		//	Hide();
			(new BAlert("test", "Alert window closed", "OK"))->Go(NULL);
			fTimeoutAlert = NULL;
			// Tiemout alert was invoked by user and timer still has not been stopped
			if(!fIsStopped)
			{
				// TODO send message to stop task
			}
			break;
		}
	}
}


void
TaskTimer::Init()
{
	fMessenger.SetTo(this);
	fTimeoutAlertInvoker.SetMessage(new BMessage(TIMEOUT_ALERT_BUTTON_SELECTION));
	fTimeoutAlertInvoker.SetTarget(this);
}


void
TaskTimer::Start(const char *name)
{
	fIsStopped = false;
	fDepotName.SetTo(name);
	int32 microSecs = fSeconds*1000000;
	microSecs = 1000000;//TODO remove debug code
	if(fMsgRunner == NULL)
	{
		fMsgRunner = new BMessageRunner(fMessenger, &fTimeoutMessage, microSecs, 1);
	}
}


void
TaskTimer::Stop(const char *name)
{
	fIsStopped = true;
	if(fMsgRunner != NULL)
	{
		//TODO set max timeout instead?
		fMsgRunner->SetCount(0);
		delete fMsgRunner;
		fMsgRunner = NULL;
	}
	//TODO if alert is showing...
	if(fTimeoutAlert)
	{
		fTimeoutAlert->Lock();
		fTimeoutAlert->SetText("Task completed");
		BButton *button = fTimeoutAlert->ButtonAt(0);
		if(button)
			button->Hide();
		button = fTimeoutAlert->ButtonAt(1);
		if(button)
		{
			button->SetLabel("OK");
		//	button->SetMessage(NULL);
		}
		fTimeoutAlert->Unlock();
	}
}
