/* TaskWindow.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
//#include <Application.h>
#include <Alert.h>
#include <Catalog.h>
//#include <File.h>
#include <LayoutBuilder.h>
//#include <Screen.h>
//#include <Size.h>
//#include <StringList.h>
#include <stdlib.h>
#include <stdio.h>


#include "TaskWindow.h"
#include "constants.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "TaskWindow"


TaskWindow::TaskWindow(BRect size, BLooper *looper, int32 what, BStringList params)
	:
	BWindow(size, "TaskWindow", B_MODAL_WINDOW, B_NOT_RESIZABLE | 
		B_ASYNCHRONOUS_CONTROLS |  B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
	msgLooper(looper),
	fWhat(what),
	fParams(params),
	fOkLabel(B_TRANSLATE_COMMENT("OK", "Button label"))
{
	fView = new BView("view", B_WILL_DRAW | B_SUPPORTS_LAYOUT);
	fView->SetExplicitMinSize(BSize(size.Width(), B_SIZE_UNSET));
	fStatus = new BStatusBar("statusbar", "");
	fStatus->SetMaxValue(fParams.CountStrings()+1);
	fStatus->SetTo(0, " ");
//	fText = new BStringView("statustext", "Test");
	fCancelButton = new BButton("Cancel", new BMessage(CANCEL_BUTTON_PRESSED));
	
	BLayoutBuilder::Group<>(fView, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING)
		.Add(fStatus)
//		.Add(fText)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.AddGlue()
			.Add(fCancelButton);
	BLayoutBuilder::Group<>(this).Add(fView);
	
	// Location on screen
	MoveTo(size.left + 20, size.top + 20);
	Show();
}


void
TaskWindow::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case CANCEL_BUTTON_PRESSED: {
			if(QuitRequested())
				Quit();
			break;
		}
		case DO_TASKS: {
			_DoTasks();
			break;
		}
		default:
			BWindow::MessageReceived(msg);
	}
}

void
TaskWindow::_DoTasks()
{
	int32 count = fParams.CountStrings();
	int32 index, errorCount=0;
	fStatus->SetTo(0, " ");
	UpdateIfNeeded();
	for(index=0; index < count; index++)
	{
		switch(fWhat){
			case DISABLE_BUTTON_PRESSED: {
				BString statusText(B_TRANSLATE("Task (%number% of %total%): Disabling depot"));
				BString indexStr, countStr;
				indexStr<<index+1;
				countStr<<count;
				statusText.ReplaceFirst("%number%", indexStr);
				statusText.ReplaceFirst("%total%", countStr);
				statusText.Append(" ");
				statusText.Append(fParams.StringAt(index));
			//	Lock();
				fStatus->Update(1, statusText);
				UpdateIfNeeded();
			//	Unlock();
				BString command("yes | pkgman drop \"");
				command.Append(fParams.StringAt(index));
				command.Append("\" >> /boot/home/pkgout");
				int sysResult = system(command.String());
				if(sysResult)
				{
					BString errorText(B_TRANSLATE("There was an error disabling the depot"));
					errorText.Append(" ").Append(fParams.StringAt(index));
					(new BAlert("error", errorText, fOkLabel))->Go();
					errorCount++;
				}
				break;
			}
			case ENABLE_BUTTON_PRESSED: {
				BString statusText(B_TRANSLATE("Task (%number% of %total%): Enabling depot"));
				BString indexStr, countStr;
				indexStr<<index+1;
				countStr<<count;
				statusText.ReplaceFirst("%number%", indexStr);
				statusText.ReplaceFirst("%total%", countStr);
				statusText.Append(" ");
				statusText.Append(fParams.StringAt(index));
			//	Lock();
				fStatus->Update(1, statusText);
				UpdateIfNeeded();
			//	Unlock();
				BString command("yes | pkgman add \"");
				command.Append(fParams.StringAt(index));
				command.Append("\" >> /boot/home/pkgout");
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
//	Lock();
	if(errorCount==0)
		fStatus->Update(1, "Completed tasks");
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
		fStatus->Update(1, finalText);
	}
	fCancelButton->SetLabel(fOkLabel);
	UpdateIfNeeded();
//	Unlock();
	
	msgLooper->PostMessage(UPDATE_LIST);
//	if(errorCount==0)
		Quit();
}
