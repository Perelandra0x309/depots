/* TaskLooper.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef TASK_LOOPER_H
#define TASK_LOOPER_H

#include <Looper.h>
#include <Path.h>
#include <String.h>
#include <StringList.h>

class TaskLooper : public BLooper {
public:
							TaskLooper(BLooper *target);
	virtual	bool			QuitRequested();
	virtual void			MessageReceived(BMessage*);
	void					SetTasks(int32 what, BStringList params);
private:
	BPath					fPkgmanTaskOut, fPkgmanTaskErr;
	BStringList				fParams;
	int32					fWhat, fOutfileInit;
	bool					fQuitWasRequested;
	void					_DoTasks();
	void					_UpdateStatus(BString text);
	BLooper					*fMsgTarget;
};

#endif
