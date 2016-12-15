/* DepotsView.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef DEPOTS_VIEW_H
#define DEPOTS_VIEW_H

#include <ColumnListView.h>
#include <GroupView.h>
#include <ObjectList.h>
#include <String.h>
#include <StringView.h>
#include <View.h>

#include "DepotsSettings.h"
#include "TaskLooper.h"

enum {
	kEnabledColumn,
	kNameColumn,
	kUrlColumn
};

class RepoRow : public BRow {
public:
								RepoRow(const char* repo_name,
									const char* repo_url, bool enabled);
		
			const char*			Name() const { return fName.String(); }
			void				SetName(const char *name);
			const char*			Url() const { return fUrl.String(); }
			void				SetEnabled(bool enabled);
			void				RefreshEnabledField();
			bool				IsEnabled() { return fEnabled; }
			void				SetTaskState(uint32 state);
			uint32				TaskState() { return fTaskState; }
private:
			BString				fName;
			BString				fUrl;
			bool				fEnabled;
			uint32				fTaskState;
};


class DepotsView : public BView {
public:
							DepotsView();
							~DepotsView();
	virtual void			AllAttached();
	virtual void			AttachedToWindow();
	virtual void			MessageReceived(BMessage*);
	void					AddManualRepository(BString url);
	bool					IsTaskRunning() { return fIsTaskRunning; }
private:
	BPath					fPkgmanListOut;
	DepotsSettings			fSettings;
	BColumnListView			*fListView;
	BStringView				*fListStatusView;
	TaskLooper				*fTaskLooper;
	bool					fIsTaskRunning, fShowCompletedStatus;
	BButton					*fAboutButton, *fAddButton, *fRemoveButton, *fEnableButton, *fDisableButton;
	
	// Message helpers
	void					_AddSelectedRowsToQueue();
	void					_StartNextTask();
	void					_CompleteRunningTask(bool noErrors);
	
	// Task queue model
	BObjectList<RepoRow>	fTaskQueue;
	void					_ModelAddToTaskQueue(RepoRow* row);
	RepoRow*				_ModelGetNextTask();
	RepoRow*				_ModelCompleteTask(bool noErrors);
	
	// GUI functions
	BString					_GetRootUrl(BString url);
	status_t				_Clean();
	void					_InitList();
	void					_UpdatePkgmanList(bool updateStatusOnly=false);
	void					_SaveList();
	RepoRow*				_AddRepo(BString name, BString url, bool enabled);
	void					_UpdateButtons();
	void					_UpdateStatusView();
};

#endif
