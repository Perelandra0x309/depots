/* AddRepoWindow.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef ADD_REPO_WINDOW_H
#define ADD_REPO_WINDOW_H

#include <Button.h>
#include <TextControl.h>
#include <View.h>
#include <Window.h>


class AddRepoWindow : public BWindow {
public:
							AddRepoWindow(BRect size, BLooper *looper);
	virtual void			MessageReceived(BMessage*);
	virtual void			Quit();
	void					SetWidth(float width);
private:
	BView					*fView;
	BTextControl			*fText;
	BButton					*fAddButton, *fCancelButton;
	BLooper					*fReplyLooper;
//	BInvoker				fDummyInvoker;
};

#endif
