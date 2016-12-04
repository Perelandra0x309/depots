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

#include "constants.h"
#include "ErrorAlert.h"
#include "TaskWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "TaskWindow"


TaskLooper::TaskLooper(int32 what, BStringList params, BLooper *target)
	:BLooper(),
	fWhat(what),
	fParams(params),
	fMsgTarget(target),
	fQuitWasRequested(false),
	fOutfileInit(B_ERROR)
{
	// Temp file location
	status_t status = find_directory(B_USER_CACHE_DIRECTORY, &fPkgmanTaskOut);
	if (status != B_OK)
		status = find_directory(B_SYSTEM_TEMP_DIRECTORY, &fPkgmanTaskOut); // alternate location
	if (status == B_OK) {
		fPkgmanTaskErr = fPkgmanTaskOut;
		fPkgmanTaskOut.Append("pkgman_out");
		fPkgmanTaskErr.Append("pkgman_stderr");
		fOutfileInit = B_OK;
	}
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
	BEntry tmpEntry(fPkgmanTaskErr.Path());
	if(tmpEntry.Exists())
		tmpEntry.Remove();
	tmpEntry.Unset();
	
	BStringList erroredParams;
	int32 index, count = fParams.CountStrings();
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
				BString nameParam(fParams.StringAt(index));
				// Set status bar text
				BString statusText(B_TRANSLATE_COMMENT("Task (%number% of %total%): Disabling depot", "Do not translate %number% and %total%"));
				BString indexStr, countStr;
				indexStr<<index+1;
				countStr<<count;
				statusText.ReplaceFirst("%number%", indexStr);
				statusText.ReplaceFirst("%total%", countStr);
				statusText.Append(" ").Append(nameParam);
				_UpdateStatus(statusText);
				// Create command
				BString command("yes | pkgman drop \"");
				command.Append(nameParam).Append("\" > ").Append(fPkgmanTaskOut.Path());
				command.Append(" 2> ").Append(fPkgmanTaskOut.Path()).Append("2");
				int sysResult = system(command.String());
				if(sysResult)
				{
					erroredParams.Add(nameParam);
					command.SetTo("echo \"[");
					command.Append(nameParam).Append("]\"").Append(" >> ").Append(fPkgmanTaskErr.Path());
					system(command.String());
					command.SetTo("cat ");
					command.Append(fPkgmanTaskOut.Path()).Append(" >> ").Append(fPkgmanTaskErr.Path());
					command.Append("; cat ").Append(fPkgmanTaskOut.Path()).Append("2 >> ").Append(fPkgmanTaskErr.Path());
					system(command.String());
					command.SetTo("echo '\n' >> ").Append(fPkgmanTaskErr.Path());
					system(command.String());
				}
				break;
			}
			case ENABLE_BUTTON_PRESSED: {
				BString urlParam(fParams.StringAt(index));
				// Set status bar text
				BString statusText(B_TRANSLATE_COMMENT("Task (%number% of %total%): Enabling depot", "Do not translate %number% and %total%"));
				BString indexStr, countStr;
				indexStr<<index+1;
				countStr<<count;
				statusText.ReplaceFirst("%number%", indexStr);
				statusText.ReplaceFirst("%total%", countStr);
				statusText.Append(" ").Append(urlParam);
				_UpdateStatus(statusText);
				// Create command
				BString command("yes | pkgman add \"");
				command.Append(urlParam).Append("\" > ").Append(fPkgmanTaskOut.Path());
				command.Append(" 2> ").Append(fPkgmanTaskOut.Path()).Append("2");
				int sysResult = system(command.String());
				if(sysResult)
				{
					erroredParams.Add(urlParam);
					command.SetTo("echo \"[");
					command.Append(urlParam).Append("]\"").Append(" >> ").Append(fPkgmanTaskErr.Path());
					system(command.String());
					command.SetTo("cat ");
					command.Append(fPkgmanTaskOut.Path()).Append(" >> ").Append(fPkgmanTaskErr.Path());
					command.Append("; cat ").Append(fPkgmanTaskOut.Path()).Append("2 >> ").Append(fPkgmanTaskErr.Path());
					system(command.String());
					command.SetTo("echo '\n' >> ").Append(fPkgmanTaskErr.Path());
					system(command.String());
				}
				break;
			}
		}
	}
	// Delete temp files
	tmpEntry.SetTo(fPkgmanTaskOut.Path());
	if(tmpEntry.Exists())
		tmpEntry.Remove();
	BString out2Path(fPkgmanTaskOut.Path());
	out2Path.Append("2");
	tmpEntry.SetTo(out2Path);
	if(tmpEntry.Exists())
		tmpEntry.Remove();
	tmpEntry.Unset();
	
	// Report completion or errors
	int32 errorCount = erroredParams.CountStrings();
	if(errorCount==0)
	{
	//	_UpdateStatus(B_TRANSLATE_COMMENT("Completed tasks", "Status message"));
		fMsgTarget->PostMessage(TASKS_COMPLETE);
	}
	else
	{
	/*	BString finalText;
		if(errorCount==1)
			finalText.SetTo(B_TRANSLATE_COMMENT("Completed tasks with 1 error", "Status message"));
		else
			finalText.SetTo(B_TRANSLATE_COMMENT("Completed tasks with %total% errors", "Status message, do not translate %total%"));
		BString total;
		total<<errorCount;
		finalText.ReplaceFirst("%total%", total);
		_UpdateStatus(finalText);*/
		
		// Error alert
		BString errorText;
		switch(fWhat){
			case DISABLE_BUTTON_PRESSED: {
				errorText.SetTo(B_TRANSLATE_COMMENT("There was an error disabling the depot", "Error message"));
				break;
			}
			case ENABLE_BUTTON_PRESSED: {
				errorText.SetTo(B_TRANSLATE_COMMENT("There was an error enabling the depot", "Error message"));
				break;
			}
		}
		errorText.Append(errorCount > 1 ? "s:\n\n" : " ");
		errorText.Append(erroredParams.Join("\n"));
		(new ErrorAlert(fPkgmanTaskErr, "error", errorText, B_TRANSLATE_COMMENT("View Details", " Button label"), kOKLabel))->Go(NULL);
		// TODO add a details text view to this?
		
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
	msgLooper(looper)
{
	fTaskLooper = new TaskLooper(what, params, this);
	
	fView = new BView("view", B_WILL_DRAW | B_SUPPORTS_LAYOUT);
	fView->SetExplicitMinSize(BSize(size.Width(), B_SIZE_UNSET));
	fStatus = new BStatusBar("statusbar", "");
	fStatus->SetMaxValue(params.CountStrings()+1);
	fStatus->SetTo(0, " ");
	fCancelButton = new BButton(kCancelLabel, new BMessage(CANCEL_BUTTON_PRESSED));
	
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
				// Disable button if we are on the last task
			//	if(fStatus->CurrentValue() == fStatus->MaxValue() - 1)
			//		fCancelButton->SetEnabled(false);
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
		//	Lock();
		//	fCancelButton->SetLabel(kOKLabel);
		//	UpdateIfNeeded();
		//	Unlock();
			msgLooper->PostMessage(UPDATE_LIST);
			Quit();
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
			fCancelButton->SetLabel(kOKLabel);
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
