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
			enum {
				EXIT_ON_ERROR	= 0x01,
				EXIT_ON_ABORT	= 0x02,
			};


public:
								JobStateListener(
									uint32 flags = EXIT_ON_ERROR
										| EXIT_ON_ABORT);

//	virtual	void				JobStarted(BSupportKit::BJob* job);
//	virtual	void				JobSucceeded(BSupportKit::BJob* job);
	virtual	void				JobFailed(BSupportKit::BJob* job);
	virtual	void				JobAborted(BSupportKit::BJob* job);
	BString						GetResult(){ return fResultText; }

private:
			uint32				fFlags;
			BString				fResultText;
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
//	void					_AddErrorDetails(BString &details);
	BLooper					*fMsgTarget;
};

#endif
