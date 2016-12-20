/* TaskTimer.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef TASKTIMER_H
#define TASKTIMER_H

#include <Alert.h>
#include <Invoker.h>
#include <Looper.h>
#include <Message.h>
#include <MessageRunner.h>
#include <Messenger.h>
#include <String.h>

#include "RepoRow.h"

class TaskTimer;
class TaskLooper;


typedef struct {
		RepoRow *rowItem;
		int32 taskType;
		BString name, taskParam;
		thread_id threadId;
		TaskLooper *owner;
		BString resultName, resultErrorDetails;
		TaskTimer *fTimer;
} Task;


class TaskTimer : public BLooper {
public:
							TaskTimer(BLooper *target, Task *owner);
							~TaskTimer();
	virtual bool			QuitRequested();
	virtual void			MessageReceived(BMessage*);
	void					Start(const char *name);
	void					Stop(const char *name);
private:
	int32					fTimeoutMicroSeconds;
	bool					fTimerIsRunning;
	BString					fDepotName;
	BLooper					*fReplyTarget;
	BMessenger				fMessenger;
	BMessageRunner			*fMsgRunner;
	BMessage				fTimeoutMessage;
	BAlert					*fTimeoutAlert;
	BInvoker				fTimeoutAlertInvoker;
	Task					*fOwner;
};

#endif
