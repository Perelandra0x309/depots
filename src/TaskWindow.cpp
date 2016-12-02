/* TaskWindow.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include <Alert.h>
#include <Catalog.h>
#include <FindDirectory.h>
#include <LayoutBuilder.h>
#include <MessageQueue.h>
#include <stdlib.h>
#include <stdio.h>

#include "TaskWindow.h"
#include "constants.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "TaskWindow"


TaskLooper::TaskLooper(int32 what, BStringList params, BLooper *target)
	:BLooper(),
	fWhat(what),
	fParams(params),
	fMsgTarget(target),
	fOkLabel(B_TRANSLATE_COMMENT("OK", "Button label")),
	fQuitWasRequested(false)
{
	// Temp file location
	status_t status = find_directory(B_SYSTEM_TEMP_DIRECTORY, &fPkgmanTaskOut);
	if (status == B_OK) {
		fPkgmanTaskOut.Append("pkgman_task");
	}//TODO alternatives?
	
	Run();
}


bool
TaskLooper::QuitRequested()
{	
	fQuitWasRequested = true;
	if(MessageQueue()->IsEmpty() && CountLockRequests() == 0)
		return true;
	else
		return false;
}


void
TaskLooper::MessageReceived(BMessage *msg)
{
	switch(msg->what)
	{
		case DO_TASKS: {
			_DoTasks();
			break;
		}
	}
}


void
TaskLooper::_DoTasks()
{
	
	// Delete existing temp file
	BEntry tmpEntry(fPkgmanTaskOut.Path());
	if(tmpEntry.Exists())
		tmpEntry.Remove();
	
	int32 count = fParams.CountStrings();
	int32 index, errorCount=0;
	for(index=0; index < count; index++)
	{
		// check if the cancel button was pressed
		if(fQuitWasRequested)
		{
			fMsgTarget->PostMessage(TASKS_CANCELED);
			return;
		}
		
		switch(fWhat){
			case DISABLE_BUTTON_PRESSED: {
				BString statusText(B_TRANSLATE_COMMENT("Task (%number% of %total%): Disabling depot", "Do not translate %number% and %total%"));
				BString indexStr, countStr;
				indexStr<<index+1;
				countStr<<count;
				statusText.ReplaceFirst("%number%", indexStr);
				statusText.ReplaceFirst("%total%", countStr);
				statusText.Append(" ");
				statusText.Append(fParams.StringAt(index));
				_UpdateStatus(statusText);
				BString command("yes | pkgman drop \"");
				command.Append(fParams.StringAt(index));
				command.Append("\" >> ").Append(fPkgmanTaskOut.Path());
				command.Append("; echo '\n' >> ").Append(fPkgmanTaskOut.Path());
				int sysResult = system(command.String());
				if(sysResult)
				{
					BString errorText(B_TRANSLATE("There was an error disabling the depot"));
					errorText.Append(" ").Append(fParams.StringAt(index));
					(new BAlert("error", errorText, fOkLabel))->Go(NULL);//TODO option to display output in temp file?
					errorCount++;
				}
				break;
			}
			case ENABLE_BUTTON_PRESSED: {
				BString statusText(B_TRANSLATE_COMMENT("Task (%number% of %total%): Enabling depot", "Do not translate %number% and %total%"));
				BString indexStr, countStr;
				indexStr<<index+1;
				countStr<<count;
				statusText.ReplaceFirst("%number%", indexStr);
				statusText.ReplaceFirst("%total%", countStr);
				statusText.Append(" ");
				statusText.Append(fParams.StringAt(index));
				_UpdateStatus(statusText);
				BString command("yes | pkgman add \"");
				command.Append(fParams.StringAt(index));
				command.Append("\" >> ").Append(fPkgmanTaskOut.Path());
				int sysResult = system(command.String());
				if(sysResult)
				{
					BString errorText(B_TRANSLATE("There was an error enabling the depot"));
					errorText.Append(" ").Append(fParams.StringAt(index));
					(new BAlert("error", errorText, fOkLabel))->Go(NULL);
					errorCount++;
				}
				break;
			}
		}
	}
	if(errorCount==0)
	{
		_UpdateStatus("Completed tasks");
		fMsgTarget->PostMessage(TASKS_COMPLETE);
	}
	else
	{
		BString finalText;
		if(errorCount==1)
			finalText.SetTo(B_TRANSLATE("Completed tasks with 1 error"));
		else
			finalText.SetTo(B_TRANSLATE("Completed tasks with %total% errors"));
		BString total;
		total<<errorCount;
		finalText.ReplaceFirst("%total%", total);
		_UpdateStatus(finalText);
		fMsgTarget->PostMessage(TASKS_COMPLETE_WITH_ERRORS);
	}
	
}

void
TaskLooper::_UpdateStatus(BString text)
{
	// send message to window
	BMessage msg(UPDATE_STATUS);
	msg.AddString(key_text, text);
	fMsgTarget->PostMessage(&msg);
}


TaskWindow::TaskWindow(BRect size, BLooper *looper, int32 what, BStringList params)
	:
	BWindow(size, "TaskWindow", B_MODAL_WINDOW, B_NOT_RESIZABLE | 
		B_ASYNCHRONOUS_CONTROLS |  B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
	msgLooper(looper),
	fOkLabel(B_TRANSLATE_COMMENT("OK", "Button label"))
{
	fTaskLooper = new TaskLooper(what, params, this);
	
	fView = new BView("view", B_WILL_DRAW | B_SUPPORTS_LAYOUT);
	fView->SetExplicitMinSize(BSize(size.Width(), B_SIZE_UNSET));
	fStatus = new BStatusBar("statusbar", "");
	fStatus->SetMaxValue(params.CountStrings()+1);
	fStatus->SetTo(0, " ");
	fCancelButton = new BButton("Cancel", new BMessage(CANCEL_BUTTON_PRESSED));
	
	BLayoutBuilder::Group<>(fView, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING)
		.Add(fStatus)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.AddGlue()
			.Add(fCancelButton);
	BLayoutBuilder::Group<>(this).Add(fView);
	
	// Location on screen
	MoveTo(size.left + 20, size.top + 20);
	Show();
}


TaskWindow::~TaskWindow()
{
	fTaskLooper->Lock();
	fTaskLooper->Quit();
}


void
TaskWindow::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case DO_TASKS: {
			Lock();
			fStatus->SetTo(0, " ");
			UpdateIfNeeded();
			Unlock();
			fTaskLooper->PostMessage(DO_TASKS);
			break;
		}
		case UPDATE_STATUS: {
			BString statusText;
			status_t status = msg->FindString(key_text, &statusText);
			if(status == B_OK)
			{
				Lock();
				fStatus->Update(1, statusText);
				UpdateIfNeeded();
				Unlock();
			}
			break;
		}
		case TASKS_COMPLETE: {
			msgLooper->PostMessage(UPDATE_LIST);
			Quit();
			break;
		}
		case TASKS_COMPLETE_WITH_ERRORS: {
			Lock();
			fCancelButton->SetLabel(fOkLabel);
			UpdateIfNeeded();
			Unlock();
			msgLooper->PostMessage(UPDATE_LIST);
			break;
		}
		case CANCEL_BUTTON_PRESSED: {
			if(fTaskLooper->QuitRequested())
			{
				msgLooper->PostMessage(UPDATE_LIST);
				Quit();
			}
			else
			{
				Lock();
				fStatus->SetTo(fStatus->MaxValue(), B_TRANSLATE_COMMENT("Canceling tasks, please wait" B_UTF8_ELLIPSIS, "Status bar text"));
				fCancelButton->SetEnabled(false);
				UpdateIfNeeded();
				Unlock();
			}
			break;
		}
		case TASKS_CANCELED: {
			Lock();
			fStatus->SetTo(fStatus->MaxValue(), B_TRANSLATE_COMMENT("Successfully canceled remaining tasks", "Status bar text"));
			fCancelButton->SetLabel(fOkLabel);
			fCancelButton->SetEnabled(true);
			UpdateIfNeeded();
			Unlock();
			msgLooper->PostMessage(UPDATE_LIST);
			break;
		}
		default:
			BWindow::MessageReceived(msg);
	}
}
