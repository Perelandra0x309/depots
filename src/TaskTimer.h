/* TaskTimer.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef TASKTIMER_H
#define TASKTIMER_H

#include <Alert.h>
//#include <Button.h>
//#include <Handler.h>
#include <Invoker.h>
#include <Looper.h>
#include <Message.h>
#include <MessageRunner.h>
#include <Messenger.h>
#include <String.h>

class TaskTimer : public BLooper {
public:
							TaskTimer(BLooper *target);
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
};

#endif
