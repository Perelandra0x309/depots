/* TaskLooper.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include <Alert.h>
#include <Catalog.h>
#include <FindDirectory.h>
#include <MessageQueue.h>
#include <stdlib.h>
#include <stdio.h>

#include "constants.h"
#include "ErrorAlert.h"
#include "TaskLooper.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "TaskLooper"


TaskLooper::TaskLooper(BLooper *target)
	:BLooper(),
	fWhat(NO_TASKS),
	fParams(),
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

// TODO remove?
void
TaskLooper::SetTasks(int32 what, BStringList params)
{
	fWhat = what;
	fParams = params;
}


void
TaskLooper::SetTask(int32 what, BString param)
{
	fWhat = what;
	fParams.MakeEmpty();
	fParams.Add(param);
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
				command.Append(nameParam).Append("\"");
				if(fOutfileInit == B_OK)
				{
					command.Append(" > ").Append(fPkgmanTaskOut.Path());
					command.Append(" 2> ").Append(fPkgmanTaskOut.Path()).Append("2");
				}
				int sysResult = system(command.String());
				if(sysResult)
				{
					erroredParams.Add(nameParam);
					if(fOutfileInit == B_OK)
					{
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
				command.Append(urlParam).Append("\"");
				if(fOutfileInit == B_OK)
				{
					command.Append(" > ").Append(fPkgmanTaskOut.Path());
					command.Append(" 2> ").Append(fPkgmanTaskOut.Path()).Append("2");
				}
				int sysResult = system(command.String());
				if(sysResult)
				{
					erroredParams.Add(urlParam);
					if(fOutfileInit == B_OK)
					{
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
		fMsgTarget->PostMessage(TASKS_COMPLETE);
	}
	else
	{
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
		if(fOutfileInit == B_OK)
			(new ErrorAlert(fPkgmanTaskErr, "error", errorText, B_TRANSLATE_COMMENT("View Details", " Button label"), kOKLabel))->Go(NULL);
		else
			(new BAlert("error", errorText, kOKLabel))->Go(NULL);
		
		fMsgTarget->PostMessage(TASKS_COMPLETE_WITH_ERRORS);
	}
	
}

void
TaskLooper::_UpdateStatus(BString text)
{
	// TODO send message to window
	
	/*BMessage msg(UPDATE_STATUS);
	msg.AddString(key_text, text);
	fMsgTarget->PostMessage(&msg);*/
}
