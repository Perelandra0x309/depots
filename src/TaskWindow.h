/* TaskWindow.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef TASK_WINDOW_H
#define TASK_WINDOW_H

#include <Button.h>
#include <StatusBar.h>
#include <String.h>
#include <StringList.h>
#include <StringView.h>
#include <View.h>
#include <Window.h>


class TaskWindow : public BWindow {
public:
							TaskWindow(BRect size, BLooper *looper, int32 what,
											BStringList params);
	virtual void			MessageReceived(BMessage*);
	void					DoTasks();
private:
	BView					*fView;
	BStatusBar				*fStatus;
//	BStringView				*fText;
	BButton					*fCancelButton;
	BLooper					*msgLooper;
	BStringList				fParams;
	int32					fWhat;
	BString					fOkLabel;
};

#endif
