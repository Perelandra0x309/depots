/* DepotsView.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef DEPOTS_VIEW_H
#define DEPOTS_VIEW_H

#include <ColumnListView.h>
#include <GroupView.h>
#include <String.h>
#include <View.h>

#include "DepotsSettings.h"

enum {
	kEnabledColumn,
	kNameColumn,
	kUrlColumn
};

class RepoRow : public BRow {
public:
								RepoRow(const char* repo_name,
									const char* repo_url, bool enabled);
//	virtual						~RepoRow();
		
			const char*			Name() const { return fName.String(); }
			void				SetName(const char *name);
			const char*			Url() const { return fUrl.String(); }
			void				SetHasSibling(bool value) { fHasSibling = value; }
			void				SetEnabled(bool enabled);
			bool				IsEnabled() { return fEnabled; }
private:
			BString				fName;
			BString				fUrl;
			bool				fHasSibling, fEnabled;
};


class DepotsView : public BView {
public:
							DepotsView();
							~DepotsView();
	virtual void			AllAttached();
	virtual void			MessageReceived(BMessage*);
	status_t				Clean();
	void					AddManualRepository(BString url);
private:
	BPath					fPkgmanListOut;
	DepotsSettings			fSettings;
	BColumnListView			*fListView;
	BButton					*fAddButton, *fRemoveButton, *fEnableButton, *fDisableButton;
	BString					fTitleEnabled, fTitleName, fTitleUrl,
							fLabelRemove, fLabelRemoveAll,
							fLabelEnable, fLabelDisable, fLabelEnableAll, fLabelDisableAll;
	bool					fUsingMinimalButtons;//TODO remove when confident this works
	void					_InitList();
	void					_UpdatePkgmanList(bool updateStatusOnly=false);
	void					_SaveList();
	RepoRow*				_AddRepo(BString name, BString url, bool enabled);
	void					_UpdateButtons();
};

#endif
