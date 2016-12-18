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
							TaskTimer(int32 seconds);
	virtual bool			QuitRequested();
	virtual void			MessageReceived(BMessage*);
	void					Init();
	void					Start(const char *name);
	void					Stop(const char *name);
private:
	int32					fSeconds;
	bool					fIsStopped;
	BString					fDepotName;
//	BButton					*fSkipButton, *fCancelAllButton;
	BMessenger				fMessenger;
	BMessageRunner			*fMsgRunner;
	BMessage				fTimeoutMessage;
	BAlert					*fTimeoutAlert;
	BInvoker				fTimeoutAlertInvoker;
};

#endif
