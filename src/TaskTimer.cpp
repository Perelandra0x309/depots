/* TaskTimer.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include <Alert.h>

#include "constants.h"
#include "TaskTimer.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "TaskTimer"


TaskTimer::TaskTimer(int32 seconds)
	:BLooper(),
	fSeconds(seconds),
	fMsgRunner(NULL)
{
	
}


void
TaskTimer::MessageReceived(BMessage *msg)
{
	switch(msg->what)
	{
		case TASK_TIMEOUT: {
			(new BAlert("timeout", "Task timed out.", "OK"))->Go(NULL);
			break;
		}
	}
}


void
TaskTimer::Start()
{
	if(fMsgRunner == NULL)
	{
		fMsgRunner = new BMessageRunner(this, new BMessage(TASK_TIMEOUT), fSeconds, 1);
	}
}


void
TaskTimer::Restart()
{
	
}
