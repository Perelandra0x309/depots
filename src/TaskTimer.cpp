/* TaskTimer.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include <Button.h>
#include <Catalog.h>
#include <LayoutBuilder.h>

#include "constants.h"
#include "TaskTimer.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "TaskTimer"


TaskTimer::TaskTimer(BLooper *target)
	:
	BLooper(),
	fReplyTarget(target),
	fTimeoutMicroSeconds(kTimerTimeoutSeconds*1000000),
	fTimerIsRunning(false),
	fMsgRunner(NULL),
	fTimeoutMessage(TASK_TIMEOUT),
	fTimeoutAlert(NULL)
{
	Run();
	
	fTimeoutMicroSeconds = 100000;//TODO remove debug code
	// Messenger for the Message Runner to use to send its message to the timer
	fMessenger.SetTo(this);
	// Invoker for the Alerts to use to send their messages to the timer
	fTimeoutAlertInvoker.SetMessage(new BMessage(TIMEOUT_ALERT_BUTTON_SELECTION));
	fTimeoutAlertInvoker.SetTarget(this);
}


TaskTimer::~TaskTimer()
{
	if(fTimeoutAlert)
	{
		fTimeoutAlert->Lock();
		fTimeoutAlert->Quit();
	}
	if(fMsgRunner)
		fMsgRunner->SetCount(0);
}


bool
TaskTimer::QuitRequested()
{
	return true;
}


void
TaskTimer::MessageReceived(BMessage *msg)
{
	switch(msg->what)
	{
		case TASK_TIMEOUT: {
			fMsgRunner = NULL;
			if(fTimerIsRunning)
			{
				BString text(B_TRANSLATE_COMMENT("Task for repository %name% is taking a long time to complete.",
								"Alert message.  Do not translate %name%"));
				BString nameString("\"");
				nameString.Append(fDepotName).Append("\"");
				text.ReplaceFirst("%name%", nameString);
				fTimeoutAlert = new BAlert("timeout", text, B_TRANSLATE_COMMENT("Keep trying", "Button label"),
											B_TRANSLATE_COMMENT("Cancel task", "Button label"), NULL,
												B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
				fTimeoutAlert->SetShortcut(0, B_ESCAPE);
				fTimeoutAlert->Go(&fTimeoutAlertInvoker);
			}
			break;
		}
		case TIMEOUT_ALERT_BUTTON_SELECTION: {
			fTimeoutAlert = NULL;
			// Timeout alert was invoked by user and timer still has not been stopped
			if(fTimerIsRunning)
			{
				// TODO send message to stop task
				int32 selection;
				msg->FindInt32("which", &selection);
					//find which button was pressed
			//	BString text("Button pressed: ");
			//	text<<selection;
			//	(new BAlert("test", text, "OK"))->Go(NULL);
				if(selection==1)
				{	
					BMessage reply(TASK_KILL_REQUEST);
					reply.AddString(key_name, fDepotName);
					fReplyTarget->PostMessage(&reply);
				}
			}
			break;
		}
	}
}


void
TaskTimer::Start(const char *name)
{
	fTimerIsRunning = true;
	fDepotName.SetTo(name);
	
	// Create a message runner that will send a TASK_TIMEOUT message if the timer is not stopped
	if(fMsgRunner == NULL)
		fMsgRunner = new BMessageRunner(fMessenger, &fTimeoutMessage, fTimeoutMicroSeconds, 1);
	else
		fMsgRunner->SetInterval(fTimeoutMicroSeconds);
}


void
TaskTimer::Stop(const char *name)
{
	fTimerIsRunning = false;
	
	// Reset max timeout so we can re-use the runner at the next Start call
	if(fMsgRunner != NULL)
		fMsgRunner->SetInterval(LLONG_MAX);
		
	// If timeout alert is showing replace it
	if(fTimeoutAlert)
	{
		// Remove current alert
		BRect frame = fTimeoutAlert->Frame();
		fTimeoutAlert->Quit();
		fTimeoutAlert = NULL;
		
		// Display new alert that won't send a message
		BString text(B_TRANSLATE_COMMENT("Good news! Task for repository %name% completed.", "Alert message.  Do not translate %name%"));
		BString nameString("\"");
		nameString.Append(name).Append("\"");
		text.ReplaceFirst("%name%", nameString);
		BAlert *newAlert = new BAlert("timeout", text, kOKLabel, NULL, NULL,
											B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		newAlert->SetShortcut(0, B_ESCAPE);
		newAlert->MoveTo(frame.left, frame.top);
		newAlert->Go(NULL);
	}
}
