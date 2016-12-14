/* TaskLooper.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
//#include <Alert.h>
#include <Catalog.h>
#include <File.h>
#include <FindDirectory.h>
#include <MessageQueue.h>
#include <stdlib.h>
//#include <stdio.h>

#include "constants.h"
//#include "ErrorAlert.h"
#include "TaskLooper.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "TaskLooper"


TaskLooper::TaskLooper(BLooper *target)
	:BLooper(),
	fWhat(NO_TASKS),
	fMsgTarget(target),
	fQuitWasRequested(false),
	fOutfileInit(B_ERROR)
{
	// Temp file location
	status_t status = find_directory(B_USER_CACHE_DIRECTORY, &fPkgmanTaskOut);
	if (status != B_OK)
		status = find_directory(B_SYSTEM_TEMP_DIRECTORY, &fPkgmanTaskOut); // alternate location
	if (status == B_OK) {
//		fPkgmanTaskErr = fPkgmanTaskOut;
		fPkgmanTaskOut.Append("pkgman_out");
//		fPkgmanTaskErr.Append("pkgman_stderr");
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
TaskLooper::SetTask(int32 what, BString param)
{
	fWhat = what;
	fParam = param;
}


void
TaskLooper::_DoTasks()
{
	// check if quit requested
	if(fQuitWasRequested)
	{
		fMsgTarget->PostMessage(TASKS_CANCELED);// TODO what happens?
		return;
	}
	
	BString errorDetails;
	int returnResult = 0;
	switch(fWhat){
		case DISABLE_DEPOT: {
			BString nameParam(fParam);
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
				returnResult = sysResult;
				errorDetails.Append("There was an error disabling the depot ").Append(nameParam).Append("\n");
				if(fOutfileInit == B_OK)
					_AddErrorDetails(errorDetails);
			}
			break;
		}
		case ENABLE_DEPOT: {
			BString urlParam(fParam);
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
				returnResult = sysResult;
				errorDetails.Append("There was an error enabling the depot ").Append(urlParam).Append("\n");
				if(fOutfileInit == B_OK)
					_AddErrorDetails(errorDetails);
			}
			break;
		}
	}
	// Delete temp files
	if(fOutfileInit == B_OK)
	{
		BEntry tmpEntry(fPkgmanTaskOut.Path());
		if(tmpEntry.Exists())
			tmpEntry.Remove();
		BString out2Path(fPkgmanTaskOut.Path());
		out2Path.Append("2");
		tmpEntry.SetTo(out2Path);
		if(tmpEntry.Exists())
			tmpEntry.Remove();
		tmpEntry.Unset();
	}
	
	// Report completion or errors
	if(returnResult == 0)
	{
		fMsgTarget->PostMessage(TASKS_COMPLETE);
	}
	else
	{
		BMessage reply(TASKS_COMPLETE_WITH_ERRORS);
		reply.AddString(key_details, errorDetails);
		fMsgTarget->PostMessage(&reply);
	}
}


void
TaskLooper::_AddErrorDetails(BString &details)
{
	details.Append("Details:\n\n");
	BFile outFile(fPkgmanTaskOut.Path(), B_READ_ONLY);
	if(outFile.InitCheck() == B_OK)
	{
		off_t size;
		outFile.GetSize(&size);
		char buffer[size];
		outFile.Read(buffer, size);
		details.Append(buffer).Append("\n");
	}
	outFile.Unset();
	BString errorPath(fPkgmanTaskOut.Path());
	errorPath.Append("2");
	outFile.SetTo(errorPath.String(), B_READ_ONLY);
	if(outFile.InitCheck() == B_OK)
	{
		off_t size;
		outFile.GetSize(&size);
		char buffer[size];
		outFile.Read(buffer, size);
		details.Append(buffer).Append("\n");
	}
	outFile.Unset();
}
