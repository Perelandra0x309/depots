/* TaskLooper.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef TASK_LOOPER_H
#define TASK_LOOPER_H

#include <Job.h>
#include <Looper.h>
#include <Path.h>
#include <String.h>
#include <StringList.h>
#include <package/Context.h>


class DecisionProvider : public BPackageKit::BDecisionProvider {
public:
								DecisionProvider(){}

	virtual	bool				YesNoDecisionNeeded(const BString& description,
									const BString& question, const BString& yes,
									const BString& no,
									 const BString& defaultChoice) { return true; }
};


class JobStateListener : public BSupportKit::BJobStateListener {
public:
								JobStateListener(){}

	virtual	void				JobStarted(BSupportKit::BJob* job);
	virtual	void				JobSucceeded(BSupportKit::BJob* job);
	virtual	void				JobFailed(BSupportKit::BJob* job);
	virtual	void				JobAborted(BSupportKit::BJob* job);
	BString						GetJobLog();

private:
			BStringList			fJobLog;
};


class TaskLooper : public BLooper {
public:
							TaskLooper(BLooper *target);
	virtual	bool			QuitRequested();
	virtual void			MessageReceived(BMessage*);
	void					SetTask(int32 what, BString param);
private:
//	BPath					fPkgmanTaskOut;
	BString					fParam;
	int32					fWhat, fOutfileInit;
	bool					fQuitWasRequested;
	void					_DoTask();
	void					_AppendErrorDetails(BString &details, JobStateListener *listener);
//	void					_AddErrorDetails(BString &details);
	BLooper					*fMsgTarget;
};

#endif
