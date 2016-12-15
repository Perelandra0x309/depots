/* TaskLooper.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef TASK_LOOPER_H
#define TASK_LOOPER_H

#include <Looper.h>
#include <Path.h>
#include <String.h>

class TaskLooper : public BLooper {
public:
							TaskLooper(BLooper *target);
	virtual	bool			QuitRequested();
	virtual void			MessageReceived(BMessage*);
	void					SetTask(int32 what, BString param);
private:
	BPath					fPkgmanTaskOut;
	BString					fParam;
	int32					fWhat, fOutfileInit;
	bool					fQuitWasRequested;
	void					_DoTask();
	void					_AddErrorDetails(BString &details);
	BLooper					*fMsgTarget;
};

#endif
