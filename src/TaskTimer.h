/* TaskTimer.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef TASKTIMER_H
#define TASKTIMER_H

#include <Looper.h>
#include <Message.h>
#include <MessageRunner.h>

class TaskTimer : public BLooper {
public:
							TaskTimer(int32 seconds);
	virtual void			MessageReceived(BMessage*);
	void					Start();
	void					Restart();
private:
	int32					fSeconds;
	BMessageRunner			*fMsgRunner;
	
};

#endif
