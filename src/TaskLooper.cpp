/* TaskLooper.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include <Alert.h>
#include <Catalog.h>
#include <File.h>
#include <FindDirectory.h>
#include <MessageQueue.h>
//#include <stdlib.h>
#include <package/AddRepositoryRequest.h>
#include <package/DropRepositoryRequest.h>
#include <package/RefreshRepositoryRequest.h>
#include <package/PackageRoster.h>
#include <package/RepositoryConfig.h>

#include "constants.h"
#include "TaskLooper.h"

#define DEBUGTASK 0

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "TaskLooper"

static const BString kLogResultIndicator = "***";
static const BString kCompletedText = B_TRANSLATE_COMMENT("Completed", "Completed task status message");
static const BString kFailedText = B_TRANSLATE_COMMENT("Failed", "Failed task status message");
static const BString kAbortedText = B_TRANSLATE_COMMENT("Aborted", "Aborted task status message");
static const BString kDescriptionText = B_TRANSLATE_COMMENT("Description", "Failed task error description");
static const BString kDetailsText = B_TRANSLATE_COMMENT("Details", "Job log details header");

using BSupportKit::BJob;


void
JobStateListener::JobStarted(BJob* job)
{
	fJobLog.Add(job->Title());
}


void
JobStateListener::JobSucceeded(BJob* job)
{
	BString resultText(kLogResultIndicator);
	fJobLog.Add(resultText.Append(kCompletedText));
}


void
JobStateListener::JobFailed(BJob* job)
{
	BString resultText(kLogResultIndicator);
	resultText.Append(kFailedText).Append(": ").Append(strerror(job->Result()));
	fJobLog.Add(resultText);
	if(job->ErrorString().Length() > 0)
	{
		resultText.SetTo(kLogResultIndicator);
		resultText.Append(kDescriptionText).Append(": ").Append(job->ErrorString());
		fJobLog.Add(resultText);
	}
}


void
JobStateListener::JobAborted(BJob* job)
{
	BString resultText(kLogResultIndicator);
	resultText.Append(kAbortedText).Append(": ").Append(strerror(job->Result()));
	fJobLog.Add(resultText);
	if(job->ErrorString().Length() > 0)
	{
		resultText.SetTo(kLogResultIndicator);
		resultText.Append(kDescriptionText).Append(": ").Append(job->ErrorString());
		fJobLog.Add(resultText);
	}
}


BString
JobStateListener::GetJobLog()
{
	return fJobLog.Join("\n");
}


TaskLooper::TaskLooper(BLooper *target)
	:BLooper(),
	fWhat(NO_TASKS),
	fMsgTarget(target),
	fQuitWasRequested(false),
	fOutfileInit(B_ERROR)
{
	// Temp file location
/*	status_t status = find_directory(B_USER_CACHE_DIRECTORY, &fPkgmanTaskOut);
	if (status != B_OK)
		status = find_directory(B_SYSTEM_TEMP_DIRECTORY, &fPkgmanTaskOut); // alternate location
	if (status == B_OK) {
		fPkgmanTaskOut.Append("pkgman_out");
		fOutfileInit = B_OK;
	}*/
	Run();
}


bool
TaskLooper::QuitRequested()
{	
	fQuitWasRequested = true;
	return MessageQueue()->IsEmpty();
}


void
TaskLooper::MessageReceived(BMessage *msg)
{
	switch(msg->what)
	{
		case DO_TASK: {
			_DoTask();
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
TaskLooper::_DoTask()
{
	// Check if quit requested
	if(fQuitWasRequested)
	{
		fMsgTarget->PostMessage(TASK_CANCELED);// TODO what happens?
		return;
	}
	
	BString errorDetails;
	status_t returnResult = B_OK;
	DecisionProvider decisionProvider;
	JobStateListener listener;
	switch(fWhat){
		case DISABLE_DEPOT: {
			BString nameParam(fParam);
			BPackageKit::BContext context(decisionProvider, listener);
			BPackageKit::DropRepositoryRequest dropRequest(context, nameParam);
			status_t result = dropRequest.Process();
			if (result != B_OK) {
				returnResult = result;
				if (result != B_CANCELED) {
					errorDetails.Append("There was an error disabling the depot ").Append(nameParam);
					_AppendErrorDetails(errorDetails, &listener);
					
				}
			}
			
			// Create command
	/*		BString command("yes | pkgman drop \"");
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
			}*/
			break;
		}
		case ENABLE_DEPOT: {
			BString urlParam(fParam);
			BPackageKit::BContext context(decisionProvider, listener);
			// Add repository
			bool asUserRepository = false; //TODO does this ever change?
			BPackageKit::AddRepositoryRequest addRequest(context, urlParam, asUserRepository);
			status_t result = addRequest.Process();
			if (result != B_OK) {
				returnResult = result;
				if (result != B_CANCELED) {
					errorDetails.Append("There was an error enabling the depot ").Append(urlParam);
					_AppendErrorDetails(errorDetails, &listener);
				}
				break;
			}
			// Continue on to refresh repo cache
			BString repoName = addRequest.RepositoryName();
			BPackageKit::BPackageRoster roster;
			BPackageKit::BRepositoryConfig repoConfig;
			roster.GetRepositoryConfig(repoName, &repoConfig);
			BPackageKit::BRefreshRepositoryRequest refreshRequest(context, repoConfig);
			result = refreshRequest.Process();
			if (result != B_OK) {
				returnResult = result;
				if (result != B_CANCELED) {
					errorDetails.Append("There was an error refreshing the depot cache for ").Append(repoName);
					_AppendErrorDetails(errorDetails, &listener);
				}
			}
			
			// Create command
	/*		BString command("yes | pkgman add \"");
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
			}*/
			break;
		}
	}
	// Delete temp files
/*	if(fOutfileInit == B_OK)
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
	}*/
	
	// Report completion status
	if(returnResult == B_OK)
	{
		fMsgTarget->PostMessage(TASK_COMPLETE);
	}
	else if(returnResult == B_CANCELED)
	{
		fMsgTarget->PostMessage(TASK_CANCELED);
	}
	else
	{
		BMessage reply(TASK_COMPLETE_WITH_ERRORS);
		reply.AddString(key_details, errorDetails);
		fMsgTarget->PostMessage(&reply);
	}
#if DEBUGTASK
	if(returnResult == B_OK || returnResult == B_CANCELED)
	{
		BString degubDetails("Debug info:\n");
		degubDetails.Append(listener.GetJobLog());
		(new BAlert("debug", degubDetails, "OK"))->Go(NULL);
	}
#endif //DEBUGTASK
}


void
TaskLooper::_AppendErrorDetails(BString &details, JobStateListener *listener)
{
	details.Append("\n\n").Append(kDetailsText).Append(":\n");
	details.Append(listener->GetJobLog());
}
/*
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
		size_t bytes = outFile.Read(buffer, size);
		details.Append(buffer, bytes).Append("\n");
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
		size_t bytes = outFile.Read(buffer, size);
		details.Append(buffer, bytes);
	}
	outFile.Unset();
}*/
