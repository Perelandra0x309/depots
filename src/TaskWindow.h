/* TaskWindow.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef TASK_WINDOW_H
#define TASK_WINDOW_H

#include <Button.h>
#include <Looper.h>
#include <Path.h>
#include <StatusBar.h>
#include <String.h>
#include <StringList.h>
#include <StringView.h>
#include <View.h>
#include <Window.h>

class TaskLooper : public BLooper {
public:
							TaskLooper(int32 what, BStringList params, BLooper *target);
	virtual	bool			QuitRequested();
	virtual void			MessageReceived(BMessage*);
private:
	BPath					fPkgmanTaskOut;
	BStringList				fParams;
	int32					fWhat, fOutfileInit;
	bool					fQuitWasRequested;
	void					_DoTasks();
	void					_UpdateStatus(BString text);
	BLooper					*fMsgTarget;
};

class TaskWindow : public BWindow {
public:
							TaskWindow(BRect size, BLooper *looper, int32 what,
											BStringList params);
							~TaskWindow();
	virtual void			MessageReceived(BMessage*);
private:
	BView					*fView;
	BStatusBar				*fStatus;
	BButton					*fCancelButton;
	TaskLooper				*fTaskLooper;
	BLooper					*msgLooper;
};

#endif
