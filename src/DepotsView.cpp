/* DepotsView.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include <Alert.h>
#include <Button.h>
#include <Catalog.h>
#include <ColumnTypes.h>
#include <File.h>
#include <FindDirectory.h>
#include <Layout.h>
#include <LayoutBuilder.h>
#include <SeparatorView.h>
#include <StringList.h>
#include <StringView.h>
#include <stdlib.h>
#include <stdio.h>

#include "constants.h"
#include "DepotsView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DepotsView"


const static BString kTitleEnabled = B_TRANSLATE_COMMENT("Enabled", "Column title");
const static BString kTitleName = B_TRANSLATE_COMMENT("Name", "Column title");
const static BString kTitleUrl = B_TRANSLATE_COMMENT("URL", "Column title");
const static BString kLabelRemove = B_TRANSLATE_COMMENT("Remove", "Button label");
const static BString kLabelRemoveAll = B_TRANSLATE_COMMENT("Remove All", "Button label");
const static BString kLabelEnable = B_TRANSLATE_COMMENT("Enable", "Button label");
const static BString kLabelEnableAll = B_TRANSLATE_COMMENT("Enable All", "Button label");
const static BString kLabelDisable = B_TRANSLATE_COMMENT("Disable", "Button label");
const static BString kLabelDisableAll = B_TRANSLATE_COMMENT("Disable All", "Button label");
const static BString kStatusViewText = B_TRANSLATE_COMMENT("Changes pending:", "Status view text");

RepoRow::RepoRow(const char* repo_name, const char* repo_url, bool enabled)
	:
	BRow(),
	fName(repo_name),
	fUrl(repo_url),
	fEnabled(enabled),
	fTaskState(STATE_NOT_IN_QUEUE)
{
	SetField(new BStringField(""), kEnabledColumn);
	SetField(new BStringField(fName.String()), kNameColumn);
	SetField(new BStringField(fUrl.String()), kUrlColumn);
	if(enabled)
		SetEnabled(enabled);	
}


void
RepoRow::SetName(const char *name)
{
	BStringField *field = (BStringField*)GetField(kNameColumn);
	field->SetString(name);
	fName.SetTo(name);
	Invalidate();
}


void
RepoRow::SetEnabled(bool enabled)
{
	fEnabled = enabled;
	RefreshEnabledField();
}


void
RepoRow::RefreshEnabledField()
{
	BStringField *field = (BStringField*)GetField(kEnabledColumn);
	field->SetString(fEnabled ? "\xE2\x9C\x94" : "");
	Invalidate();
}


void
RepoRow::SetTaskState(uint32 state)
{
	fTaskState = state;
	switch(state)
	{
		case STATE_IN_QUEUE_WAITING: {
			BStringField *field = (BStringField*)GetField(kEnabledColumn);
			field->SetString(B_UTF8_ELLIPSIS);
			Invalidate();
			break;
		}
	/*	case STATE_IN_QUEUE_RUNNING: {
			BStringField *field = (BStringField*)GetField(kEnabledColumn);
			if(IsEnabled())
				field->SetString(B_TRANSLATE("Disabling..."));
			else
				field->SetString(B_TRANSLATE("Enabling..."));
			Invalidate();
			break;
		}*/
	}
}


DepotsView::DepotsView()
	:
	BView("depotsview", B_SUPPORTS_LAYOUT),
	fTaskLooper(NULL),
	fIsTaskRunning(false)
{
	// Temp file location
	status_t status = find_directory(B_USER_CACHE_DIRECTORY, &fPkgmanListOut);
	if (status != B_OK)
		status = find_directory(B_SYSTEM_TEMP_DIRECTORY, &fPkgmanListOut); // alternate location
	if (status == B_OK) {
		fPkgmanListOut.Append("pkgman_task");
	}
	
	fListView = new BColumnListView("list", B_NAVIGABLE, B_PLAIN_BORDER);
	fListView->SetSelectionMessage(new BMessage(LIST_SELECTION_CHANGED));
	float col0width = be_plain_font->StringWidth(kTitleEnabled) + 15;
	float col1width = be_plain_font->StringWidth(kTitleName) + 15;
	float col2width = be_plain_font->StringWidth(kTitleUrl) + 15;
	fListView->AddColumn(new BStringColumn(kTitleEnabled, col0width, col0width, col0width,
		B_TRUNCATE_END, B_ALIGN_CENTER), kEnabledColumn);
	fListView->AddColumn(new BStringColumn(kTitleName, 90, col1width, 300,
		B_TRUNCATE_END), kNameColumn);
	fListView->AddColumn(new BStringColumn(kTitleUrl, 500, col2width, 5000,
		B_TRUNCATE_END), kUrlColumn);
	BMessage *invokeMsg = new BMessage(ITEM_INVOKED);
	fListView->SetInvocationMessage(invokeMsg);
	
	// Depot list status view
	BView *statusContainerView = new BView("status", B_SUPPORTS_LAYOUT);
	BString templateText(kStatusViewText);
	templateText.Append("88");
	fListStatusView = new BStringView("status", templateText);
	BFont font(be_plain_font);
	font.SetSize(10.0f);
	fListStatusView->SetFont(&font, B_FONT_SIZE);
	BSize statusViewSize = fListStatusView->PreferredSize();
	statusViewSize.width += 3;
	statusViewSize.height += 1;
	statusContainerView->SetExplicitSize(statusViewSize);
	BLayoutBuilder::Group<>(statusContainerView, B_VERTICAL, 0)
		.AddGroup(B_HORIZONTAL, 1)
			.Add(new BSeparatorView(B_VERTICAL))
			.Add(fListStatusView)
			.AddGlue()
		.End()
		.Add(new BSeparatorView(B_HORIZONTAL));
	fListView->AddStatusView(statusContainerView);
	
	
	fEnableButton = new BButton(kLabelEnable, new BMessage(ENABLE_BUTTON_PRESSED));
	fDisableButton = new BButton(kLabelDisable, new BMessage(DISABLE_BUTTON_PRESSED));
	
#if USE_MINIMAL_BUTTONS
	// ---Minimal buttons option---
	font_height fontHeight;
	GetFontHeight(&fontHeight);
	int buttonHeight = int(fontHeight.ascent + fontHeight.descent + 12); // button size determined by font size
	BSize btnSize(buttonHeight,buttonHeight);
	
	// Create button views with fixed size
	fAddButton = new BButton("plus", "+", new BMessage(ADD_REPO_WINDOW));
	fAddButton->SetExplicitSize(btnSize);
	
	fRemoveButton = new BButton("minus", "-", new BMessage(REMOVE_REPOS));
	fRemoveButton->SetExplicitSize(btnSize);
	
	fAboutButton = new BButton("about", "?", new BMessage(SHOW_ABOUT));
	fAboutButton->SetExplicitSize(btnSize);
	
	int buttonSpacing = 1;
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.SetInsets(B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING)
		.AddGroup(B_HORIZONTAL, 0, 0.0)
			.Add(new BStringView("instruction", B_TRANSLATE_COMMENT("Select depots to use in HaikuDepot:", "Label text")), 0.0)
			.AddGlue()
			.Add(fAboutButton, 0.0)
		.End()
		.AddStrut(3)
		.Add(fListView, 1)
		.AddGroup(B_HORIZONTAL, 0, 0.0)
			// Add and Remove buttons
			.AddGroup(B_VERTICAL, 0, 0.0)
				.AddGroup(B_HORIZONTAL, 0, 0.0)
					.Add(new BSeparatorView(B_VERTICAL))
					.AddGroup(B_VERTICAL, 0, 0.0)
						.AddGroup(B_HORIZONTAL, buttonSpacing, 0.0)
							.SetInsets(buttonSpacing)
							.Add(fAddButton)
							.Add(fRemoveButton)
						.End()
						.Add(new BSeparatorView(B_HORIZONTAL))
					.End()
					.Add(new BSeparatorView(B_VERTICAL))
				.End()
				.AddGlue()
			.End()
			// Enable and Disable buttons
			.AddGroup(B_HORIZONTAL)
				.SetInsets(B_USE_DEFAULT_SPACING,B_USE_DEFAULT_SPACING,B_USE_DEFAULT_SPACING,0)
				.AddGlue()
				.Add(fEnableButton)
				.Add(fDisableButton)
			.End()
		.End();
		
		/* Saved- buttons in seperate horizontal groups
		.AddGroup(B_HORIZONTAL, 0, 0.0)
			.Add(new BSeparatorView(B_VERTICAL))
			.AddGroup(B_VERTICAL, 0, 0.0)
				.AddGroup(B_HORIZONTAL, 3, 0.0)
					.SetInsets(3,3,3,3)
	//				.Add(plusButton, 0.0)
	//				.Add(minusButton, 0.0)
					//.Add(new BSeparatorView(B_VERTICAL))
					.Add(button1View)
					//.Add(new BSeparatorView(B_VERTICAL))//, B_FANCY_BORDER))
					.Add(button2View)
					//.Add(new BSeparatorView(B_VERTICAL))
	//				.AddGlue()
				.End()
				.Add(new BSeparatorView(B_HORIZONTAL))
			.End()
			.Add(new BSeparatorView(B_VERTICAL))
			.AddGroup(B_HORIZONTAL, 0, 1)
	//			.Add(new BSeparatorView(B_VERTICAL))
				.AddGlue()
			.End()
		.End()
	//	.Add(new BSeparatorView(B_HORIZONTAL))
		.AddGroup(B_HORIZONTAL)
		//	.Add(fAddButton)
		//	.Add(fRemoveButton)
			.AddGlue()
			.Add(fEnableButton)
			.Add(fDisableButton)
		.End();
		*/
		
	// ---End minimal buttons section---
#else
	
		// ---Standard buttons option---
	font_height fontHeight;
	GetFontHeight(&fontHeight);
	int buttonSize = int(fontHeight.ascent + fontHeight.descent + 12); // button size determined by font size
	
	fAddButton = new BButton(B_TRANSLATE_COMMENT("Add" B_UTF8_ELLIPSIS, "Button label"),
							new BMessage(ADD_REPO_WINDOW));
	fRemoveButton = new BButton(fLabelRemove, new BMessage(REMOVE_REPOS));
	
	fAboutButton = new BButton("about", "?", new BMessage(SHOW_ABOUT));
	fAboutButton->SetExplicitSize(BSize(buttonSize, buttonSize));
	
	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING)
		.AddGroup(B_HORIZONTAL, 0, 0.0)
			.Add(new BStringView("instruction", B_TRANSLATE_COMMENT("Select depots to use in HaikuDepot:", "Label text")), 0.0)
			.AddGlue()
			.Add(fAboutButton, 0.0)
		.End()
		.AddStrut(3)
		.Add(fListView)
		.AddGroup(B_HORIZONTAL)
			.Add(fAddButton)
			.Add(fRemoveButton)
			.AddGlue()
			.Add(fEnableButton)
			.Add(fDisableButton)
		.End();
	// -- End standard buttons section
#endif //USE_MINIMAL_BUTTONS

}


DepotsView::~DepotsView()
{
	if(fTaskLooper)
	{
		fTaskLooper->Lock();
		fTaskLooper->Quit();
	}
	_Clean();
}


void
DepotsView::AllAttached()
{
	BView::AllAttached();
	fRemoveButton->SetTarget(this);
	fEnableButton->SetTarget(this);
	fDisableButton->SetTarget(this);
	fListView->SetTarget(this);
	fRemoveButton->SetEnabled(false);
	fEnableButton->SetEnabled(false);
	fDisableButton->SetEnabled(false);
	_UpdateStatusView();
	_InitList();
}


void
DepotsView::AttachedToWindow()
{
	fTaskLooper = new TaskLooper(Window());
}


void
DepotsView::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case REMOVE_REPOS :{
			RepoRow *rowItem = dynamic_cast<RepoRow*>(fListView->CurrentSelection());
			while(rowItem)
			{
				RepoRow *oldRow = rowItem;
				rowItem = dynamic_cast<RepoRow*>(fListView->CurrentSelection(rowItem));
				fListView->RemoveRow(oldRow);
				delete oldRow;
			}
			_SaveList();
			break;
		}
		case LIST_SELECTION_CHANGED: {
			_UpdateButtons();
			break;
		}
		case ITEM_INVOKED: {
			if(fEnableButton->IsEnabled())
				MessageReceived(new BMessage(ENABLE_BUTTON_PRESSED));
			else if(fDisableButton->IsEnabled())
				MessageReceived(new BMessage(DISABLE_BUTTON_PRESSED));
			break;
		}
		case ENABLE_BUTTON_PRESSED: {
			BStringList names;
			bool paramsOK = true;
			// Check if there are multiple selections of the same depot, pkgman won't like that
			RepoRow* rowItem = dynamic_cast<RepoRow*>(fListView->CurrentSelection());
			while(rowItem)
			{
				if(names.HasString(rowItem->Name()) && kNewRepoName.Compare(rowItem->Name()) != 0)
				{
					(new BAlert("duplicate", B_TRANSLATE_COMMENT("You can only enable one URL for "
									"each depot.  Please change your selections.", "Error message"),
									kOKLabel, NULL, NULL,
									B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go(NULL);
					paramsOK = false;
					break;
				}
				else
					names.Add(rowItem->Name());
				rowItem = dynamic_cast<RepoRow*>(fListView->CurrentSelection(rowItem));
			}
			if(paramsOK)
			{
				_AddSelectedRowsToQueue();
				_StartNextTask();
				_UpdateButtons();
			}
			break;
		}
		case DISABLE_BUTTON_PRESSED: {
			_AddSelectedRowsToQueue();
			_StartNextTask();
			_UpdateButtons();
			break;
		}
		case TASKS_COMPLETE_WITH_ERRORS: {
			_CompleteRunningTask(false);
			BString errorDetails;
			status_t result = msg->FindString(key_details, &errorDetails);
			if(result == B_OK)
			{// TODO seperate details view?
				(new BAlert("error", errorDetails, kOKLabel))->Go(NULL);
			}
			_StartNextTask();
			_UpdateButtons();
			break;
		}
		case TASKS_COMPLETE: {
			_CompleteRunningTask(true);
			_StartNextTask();
			_UpdateButtons();
			break;
		}
		case UPDATE_LIST: {
			_UpdatePkgmanList(true);
			_UpdateButtons();
			break;
		}
		default:
			BView::MessageReceived(msg);
	}
}


void
DepotsView::_AddSelectedRowsToQueue()
{
	RepoRow* rowItem = dynamic_cast<RepoRow*>(fListView->CurrentSelection());
	while(rowItem)
	{
		_ModelAddToTaskQueue(rowItem);
		rowItem = dynamic_cast<RepoRow*>(fListView->CurrentSelection(rowItem));
	}
}


void
DepotsView::_StartNextTask()
{
	// If tasks are not already running, kick them off
	if(!fIsTaskRunning)
	{
		RepoRow* row = _ModelGetNextTask();
		if(row != NULL)
		{
			if(row->IsEnabled())
				fTaskLooper->SetTask(DISABLE_DEPOT, row->Name());
			else
				fTaskLooper->SetTask(ENABLE_DEPOT, row->Url());
			fIsTaskRunning = true;
			fTaskLooper->PostMessage(DO_TASKS);
		}
	}
}


void
DepotsView::_CompleteRunningTask(bool noErrors)
{
	if(fIsTaskRunning)
	{
		RepoRow* row = _ModelCompleteTask(noErrors);
		if(row != NULL)
			fIsTaskRunning = false;
	}
}


void
DepotsView::_ModelAddToTaskQueue(RepoRow* row)
{
	fTaskQueue.AddItem(row);
	// Only present a status count if there is more than one item in queue
	if(fTaskQueue.CountItems() > 1)
		_UpdateStatusView();
	row->SetTaskState(STATE_IN_QUEUE_WAITING);
}


RepoRow*
DepotsView::_ModelGetNextTask()
{
	if(!fTaskQueue.IsEmpty())
	{
		RepoRow* row = fTaskQueue.ItemAt(0);
		row->SetTaskState(STATE_IN_QUEUE_RUNNING);
		return row;
	}
	else
		return NULL;
}


RepoRow*
DepotsView::_ModelCompleteTask(bool noErrors)
{
	if(!fTaskQueue.IsEmpty())
	{
		RepoRow* row = fTaskQueue.RemoveItemAt(0);
		_UpdateStatusView();
		row->SetTaskState(STATE_NOT_IN_QUEUE);
		if(noErrors)
			row->SetEnabled(!row->IsEnabled());
		else
			row->RefreshEnabledField();
		return row;
	}
	else
		return NULL;
}


void
DepotsView::AddManualRepository(BString url)
{
	BString name(kNewRepoName);
	BString rootUrl = _GetRootUrl(url);
	bool foundRoot = false;
	int32 index;
	int32 listCount = fListView->CountRows();
	for(index=0; index < listCount; index++)
	{
		// Find duplicate
		RepoRow *repoItem = dynamic_cast<RepoRow*>((fListView->RowAt(index)));
		const char *urlPtr = repoItem->Url();
		if(url.ICompare(urlPtr) == 0)
		{
			(new BAlert("duplicate", B_TRANSLATE_COMMENT("Depot already exists.", "Error message"), kOKLabel))->Go();
			return; 
		}
		//Find same root url
		if(foundRoot == false && rootUrl.Compare(urlPtr, rootUrl.Length()) == 0)
		{
			foundRoot = true;
			name = repoItem->Name();
		}
	}
	RepoRow *newRepo = _AddRepo(name, url, false);
	fListView->DeselectAll();
	fListView->AddToSelection(newRepo);
	_UpdateButtons();
	_SaveList();
}


BString
DepotsView::_GetRootUrl(BString url)
{
	// Find the protocol if it exists
	int32 ww = url.FindFirst("://");
	if(ww == B_ERROR)
		ww = 0;
	else
		ww += 3;
	// Find second /
	int32 rootEnd = url.FindFirst("/", ww+1);
	if(rootEnd == B_ERROR)
		return url;
	rootEnd = url.FindFirst("/", rootEnd+1);
	if(rootEnd == B_ERROR)
		return url;
	else
		return url.Truncate(rootEnd);
}


status_t
DepotsView::_Clean()
{
	BRow* row;
	while ((row = fListView->RowAt((int32)0, NULL)) != NULL) {
		fListView->RemoveRow(row);
		delete row;
	}
	return B_OK;
}


void
DepotsView::_InitList()
{
	// Get list of known repositories from the settings file
	int32 index, repoCount;
	BStringList nameList, urlList;
	status_t result = fSettings.GetRepositories(repoCount, nameList, urlList);
	BString name, url;
	for(index=0; index < repoCount; index++)
	{
		name = nameList.StringAt(index);
		url = urlList.StringAt(index);
		_AddRepo(name, url, false);
	}
	_UpdatePkgmanList();
	fListView->SetSortColumn(fListView->ColumnAt(kUrlColumn), false, true);
	fListView->ResizeAllColumnsToPreferred();
}


void
DepotsView::_UpdatePkgmanList(bool updateStatusOnly)
{
	// Clear enabled status on all rows
	if(updateStatusOnly)
	{
		int32 index;
		int32 listCount = fListView->CountRows();
		// Find duplicate
		for(index=0; index < listCount; index++)
		{
			RepoRow *repoItem = dynamic_cast<RepoRow*>((fListView->RowAt(index)));
			repoItem->SetEnabled(false);
		}
	}
	
	// Get list of current enabled repositories from pkgman
	BString command("pkgman list > ");
	command.Append(fPkgmanListOut.Path());
	int sysResult = system(command.String());
//	printf("result=%i", sysResult);
	BFile listFile(fPkgmanListOut.Path(), B_READ_ONLY);
	if(listFile.InitCheck()==B_OK)
	{
		off_t size;
		listFile.GetSize(&size);
		char buffer[size];
		listFile.Read(buffer, size);
		BString text(buffer);
//		printf(text.String());
		BStringList pkgmanOutput;
		text.Split("\n", true, pkgmanOutput);
		// Read each set of enabled repos from 3 lines of pkgman output
		while(pkgmanOutput.CountStrings() > 2)
		{
			BString name = pkgmanOutput.StringAt(0);
			BString url = pkgmanOutput.StringAt(1);
			pkgmanOutput.Remove(0,3);
			// remove leading tabs and spaces
			int index = 0;
			while(name[index] == ' ' || name[index] == '\t')
				index++;
			name.RemoveChars(0, index);
			url.RemoveFirst("base-url:");
			index = 0;
			while(url[index] == ' ' || url[index] == '\t')
				index++;
			url.RemoveChars(0, index);
			_AddRepo(name, url, true);
		}
		_SaveList();
		listFile.Unset();
		BEntry tmpEntry(fPkgmanListOut.Path());
		tmpEntry.Remove();
	}
}

void
DepotsView::_SaveList()
{
	BStringList nameList, urlList;
	int32 index;
	int32 listCount = fListView->CountRows();
	for(index=0; index < listCount; index++)
	{
		RepoRow *repoItem = dynamic_cast<RepoRow*>((fListView->RowAt(index)));
		nameList.Add(repoItem->Name());
		urlList.Add(repoItem->Url());
	}
	fSettings.SetRepositories(nameList, urlList);
}


RepoRow*
DepotsView::_AddRepo(BString name, BString url, bool enabled)
{
	// URL must have a protocol
	if(url.FindFirst("://") == B_ERROR)
		return NULL;
//	printf("Adding:%s:%s\n", name.String(), url.String());
	RepoRow *addedRow=NULL;
	int32 index;
	int32 listCount = fListView->CountRows();
	// Find matching URL or names
	for(index=0; index < listCount; index++)
	{
		RepoRow *repoItem = dynamic_cast<RepoRow*>((fListView->RowAt(index)));
		if(url.ICompare(repoItem->Url()) == 0)
		{
			// update name and enabled values
			if(name.Compare(repoItem->Name()) != 0)
				repoItem->SetName(name.String());
			repoItem->SetEnabled(enabled);
			addedRow = repoItem;
		}
		else if(name.Compare(repoItem->Name()) == 0)
		{	
			repoItem->SetEnabled(false);
		}

	}
	if(addedRow == NULL)
	{
		addedRow = new RepoRow(name, url, enabled);
		fListView->AddRow(addedRow);
	}
	return addedRow;
}


void
DepotsView::_UpdateButtons()
{
	RepoRow *rowItem = dynamic_cast<RepoRow*>(fListView->CurrentSelection());
	// At least one row is selected
	if(rowItem)
	{
		bool someAreEnabled = false, someAreDisabled = false, someAreInQueue = false;
		int32 selectedCount=0;
		RepoRow *rowItem = dynamic_cast<RepoRow*>(fListView->CurrentSelection());
		while(rowItem)
		{
			selectedCount++;
			switch(rowItem->TaskState())
			{
				case STATE_IN_QUEUE_WAITING:
				case STATE_IN_QUEUE_RUNNING: {
					someAreInQueue = true;
					break;
				}
			}
			if(rowItem->IsEnabled())
				someAreEnabled = true;
			else
				someAreDisabled = true;
			rowItem = dynamic_cast<RepoRow*>(fListView->CurrentSelection(rowItem));
		}
		// Change button labels depending on which rows are selected
		if(selectedCount>1)
		{
#if !USE_MINIMAL_BUTTONS
			fRemoveButton->SetLabel(kLabelRemoveAll);
#endif
			fEnableButton->SetLabel(kLabelEnableAll);
			fDisableButton->SetLabel(kLabelDisableAll);
		}
		else
		{
#if !USE_MINIMAL_BUTTONS
			fRemoveButton->SetLabel(kLabelRemove);
#endif
			fEnableButton->SetLabel(kLabelEnable);
			fDisableButton->SetLabel(kLabelDisable);
		}
		// Set which buttons should be enabled
		fRemoveButton->SetEnabled(!someAreEnabled && !someAreInQueue);
		if((someAreEnabled && someAreDisabled) || someAreInQueue)
		{
			// there are a mix of enabled and disabled depots selected
			fEnableButton->SetEnabled(false);
			fDisableButton->SetEnabled(false);	
		}
		else
		{
			fEnableButton->SetEnabled(someAreDisabled);
			fDisableButton->SetEnabled(someAreEnabled);
		}
		
	}
	// No selected rows
	else
	{
#if !USE_MINIMAL_BUTTONS
		fRemoveButton->SetLabel(kLabelRemove);
#endif
		fEnableButton->SetLabel(kLabelEnable);
		fDisableButton->SetLabel(kLabelDisable);
		fEnableButton->SetEnabled(false);
		fDisableButton->SetEnabled(false);
		fRemoveButton->SetEnabled(false);
	}
}


void
DepotsView::_UpdateStatusView()
{
	int count = fTaskQueue.CountItems();
	if(count)
	{
		BString text(kStatusViewText);
		text<<count;
		fListStatusView->SetText(text);
	}
	else
		fListStatusView->SetText("");
}
