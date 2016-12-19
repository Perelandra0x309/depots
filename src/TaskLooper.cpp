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
//	fWhat(NO_TASKS),
	fReplyTarget(target)
//	fQuitWasRequested(false),
//	fOutfileInit(B_ERROR)
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
//	fQuitWasRequested = true;
	return MessageQueue()->IsEmpty();
}


void
TaskLooper::MessageReceived(BMessage *msg)
{
	switch(msg->what)
	{
		case DO_TASK: {
			RepoRow *rowItem;
			status_t result = msg->FindPointer(key_rowptr, (void**)&rowItem);
			if(result==B_OK)
			{
				// Initialize task
				Task *newTask = new Task();
				newTask->rowItem = rowItem;
				if(rowItem->IsEnabled())
				{
					newTask->taskType = DISABLE_DEPOT;
					newTask->taskParam = rowItem->Name();
				}
				else
				{
					newTask->taskType = ENABLE_DEPOT;
					newTask->taskParam = rowItem->Url();
				}
				newTask->owner = this;
			//	(new BAlert("task", newTask->taskParam, "OK"))->Go(NULL);
				
				// Add to queue and start
				fTaskQueue.AddItem(newTask);
				BString threadName(newTask->taskType==ENABLE_DEPOT ? "enable_task" : "disable_task");
				newTask->threadId = spawn_thread(_DoTask, threadName.String(), B_NORMAL_PRIORITY, (void*)newTask);
				if(newTask->threadId < B_OK)
				{
					// TODO?
				}
				else
				{
					resume_thread(newTask->threadId);
					// Reply to view
					BMessage reply(*msg);
					reply.what = TASK_STARTED;
					reply.AddInt16(key_count, fTaskQueue.CountItems());
					fReplyTarget->PostMessage(&reply);
				}
				
			}
			break;
		}
		case TASK_COMPLETE:
		case TASK_COMPLETE_WITH_ERRORS:
		case TASK_CANCELED: {
		//	(new BAlert("complete", "Task complete.", "OK"))->Go(NULL);
			Task *task;
			status_t result = msg->FindPointer(key_taskptr, (void**)&task);
			if(result==B_OK)
			{
				BMessage reply(*msg);
				reply.AddInt16(key_count, fTaskQueue.CountItems()-1);
				reply.AddPointer(key_rowptr, task->rowItem);
				if(msg->what == TASK_COMPLETE_WITH_ERRORS)
					reply.AddString(key_details, task->resultErrorDetails);
				if(task->taskType == ENABLE_DEPOT)
					reply.AddString(key_name, task->resultNewName);
				fReplyTarget->PostMessage(&reply);
				// Delete task
				fTaskQueue.RemoveItem(task);
				delete task;
			}
			break;
		}
	}
}

/*
void
TaskLooper::SetTask(int32 what, BString param)
{
	fWhat = what;
	fParam = param;
}*/


status_t
TaskLooper::_DoTask(void *data)
{
	// Check if quit requested
/*	if(fQuitWasRequested)
	{
		fReplyTarget->PostMessage(TASK_CANCELED);// TODO what happens?
		return;
	}*/
	
	Task *task = (Task*)data;
	BString errorDetails, repoName("");
	status_t returnResult = B_OK;
	DecisionProvider decisionProvider;
	JobStateListener listener;
	switch(task->taskType){
		case DISABLE_DEPOT: {
			BString nameParam(task->taskParam);
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
			BString urlParam(task->taskParam);
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
			repoName = addRequest.RepositoryName();
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
	BMessage reply;
	if(returnResult == B_OK)
	{
		reply.what = TASK_COMPLETE;
		// Add the repo name if we need to update the list row value
		if(task->taskType == ENABLE_DEPOT)
			task->resultNewName = repoName;
	}
	else if(returnResult == B_CANCELED)
	{
		reply.what = TASK_CANCELED;
	}
	else
	{
		reply.what = TASK_COMPLETE_WITH_ERRORS;
		task->resultErrorDetails = errorDetails;
		if(task->taskType == ENABLE_DEPOT)
			task->resultNewName = repoName;
	}
	reply.AddPointer(key_taskptr, task);
	task->owner->PostMessage(&reply);
#if DEBUGTASK
	if(returnResult == B_OK || returnResult == B_CANCELED)
	{
		BString degubDetails("Debug info:\n");
		degubDetails.Append(listener.GetJobLog());
		(new BAlert("debug", degubDetails, "OK"))->Go(NULL);
	}
#endif //DEBUGTASK
	return 0;
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
