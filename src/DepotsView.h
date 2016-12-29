/* DepotsView.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef DEPOTS_VIEW_H
#define DEPOTS_VIEW_H


#include <ColumnListView.h>
#include <String.h>
#include <StringView.h>
#include <View.h>

#include "DepotsSettings.h"
#include "RepoRow.h"
#include "TaskLooper.h"


class DepotsListView : public BColumnListView {
public:
							DepotsListView(const char* name);
	virtual void			KeyDown(const char *bytes, int32 numBytes);
};


class DepotsView : public BView {
public:
							DepotsView();
							~DepotsView();
	virtual void			AllAttached();
	virtual void			AttachedToWindow();
	virtual void			MessageReceived(BMessage*);
	void					AddManualRepository(BString url);
	bool					IsTaskRunning() { return fRunningTaskCount>0; }

private:
	DepotsSettings			fSettings;
	DepotsListView			*fListView;
	BView					*fStatusContainerView;
	BStringView				*fListStatusView;
	TaskLooper				*fTaskLooper;
	bool					fShowCompletedStatus;
	int						fRunningTaskCount, fLastCompletedTimerId;
	BButton					*fAboutButton, *fAddButton, *fRemoveButton,
							*fEnableButton, *fDisableButton;
	
	// Message helpers
	void					_AddSelectedRowsToQueue();
	void					_TaskStarted(RepoRow *rowItem, int16 count);
	void					_TaskCompleted(RepoRow *rowItem, int16 count,
								BString& newName);
	void					_TaskCanceled(RepoRow *rowItem, int16 count);
	void					_ShowCompletedStatusIfDone();
	void					_UpdateFromRepoConfig(RepoRow *rowItem);
	
	// GUI functions
	BString					_GetRootUrl(BString url);
	status_t				_EmptyList();
	void					_InitList();
	void					_RefreshList();
	void					_UpdateListFromRoster();
	void					_SaveList();
	RepoRow*				_AddRepo(BString name, BString url, bool enabled);
	void					_FindSiblings();
	void					_UpdateButtons();
	void					_UpdateStatusView();
};

#endif
