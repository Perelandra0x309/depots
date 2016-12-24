/* DepotsView.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */


#include "DepotsView.h"

#include <stdlib.h>
#include <Alert.h>
#include <Button.h>
#include <Catalog.h>
#include <ColumnTypes.h>
#include <LayoutBuilder.h>
#include <MessageRunner.h>
#include <ScrollBar.h>
#include <SeparatorView.h>
#include <package/PackageRoster.h>
#include <package/RepositoryConfig.h>

#include "constants.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DepotsView"


static const BString kTitleEnabled = B_TRANSLATE_COMMENT("Enabled",
	"Column title");
static const BString kTitleName = B_TRANSLATE_COMMENT("Name", "Column title");
static const BString kTitleUrl = B_TRANSLATE_COMMENT("URL", "Column title");
static const BString kLabelRemove = B_TRANSLATE_COMMENT("Remove",
	"Button label");
static const BString kLabelRemoveAll = B_TRANSLATE_COMMENT("Remove All",
	"Button label");
static const BString kLabelEnable = B_TRANSLATE_COMMENT("Enable",
	"Button label");
static const BString kLabelEnableAll = B_TRANSLATE_COMMENT("Enable All",
	"Button label");
static const BString kLabelDisable = B_TRANSLATE_COMMENT("Disable",
	"Button label");
static const BString kLabelDisableAll = B_TRANSLATE_COMMENT("Disable All",
	"Button label");
static const BString kStatusViewText = B_TRANSLATE_COMMENT("Changes pending:",
	"Status view text");
static const BString kStatusCompletedText = B_TRANSLATE_COMMENT("Changes completed",
	"Status view text");


DepotsView::DepotsView()
	:
	BView("depotsview", B_SUPPORTS_LAYOUT),
	fTaskLooper(NULL),
	fRunningTaskCount(0),
	fLastCompletedTimerId(0),
	fShowCompletedStatus(false)
{
	// Column list view with 3 columns
	fListView = new BColumnListView("list", B_NAVIGABLE, B_PLAIN_BORDER);
	fListView->SetSelectionMessage(new BMessage(LIST_SELECTION_CHANGED));
	float col0width = be_plain_font->StringWidth(kTitleEnabled) + 15;
	float col1width = be_plain_font->StringWidth(kTitleName) + 15;
	float col2width = be_plain_font->StringWidth(kTitleUrl) + 15;
	fListView->AddColumn(new BStringColumn(kTitleEnabled, col0width, col0width,
		col0width, B_TRUNCATE_END, B_ALIGN_CENTER), kEnabledColumn);
	fListView->AddColumn(new BStringColumn(kTitleName, 90, col1width, 300,
		B_TRUNCATE_END), kNameColumn);
	fListView->AddColumn(new BStringColumn(kTitleUrl, 500, col2width, 5000,
		B_TRUNCATE_END), kUrlColumn);
	fListView->SetInvocationMessage(new BMessage(ITEM_INVOKED));
	
	// Depot list status view
	BView *statusContainerView = new BView("status", B_SUPPORTS_LAYOUT);
	BString templateText(kStatusViewText);
	templateText.Append(" 88");
		// Simulate a status text with two digit queue count
	fListStatusView = new BStringView("status", templateText);
	
	// Set a smaller fixed font size and slightly lighten text color
	BFont font(be_plain_font);
	font.SetSize(10.0f);
	fListStatusView->SetFont(&font, B_FONT_SIZE);
	fListStatusView->SetHighUIColor(fListStatusView->HighUIColor(), .9f);
	
	// Set appropriate explicit view sizes
	float viewWidth = max_c(fListStatusView->StringWidth(templateText),
		fListStatusView->StringWidth(kStatusCompletedText));
	BSize statusViewSize(viewWidth + 3, B_H_SCROLL_BAR_HEIGHT - 2);
	fListStatusView->SetExplicitSize(statusViewSize);
	statusViewSize.height += 1;
	statusContainerView->SetExplicitSize(statusViewSize);
	BLayoutBuilder::Group<>(statusContainerView, B_HORIZONTAL, 0)
		.Add(new BSeparatorView(B_VERTICAL))
		.AddGroup(B_VERTICAL, 0)
			.AddGlue()
			.AddGroup(B_HORIZONTAL, 0)
				.SetInsets(2,0,0,0)
				.Add(fListStatusView)
				.AddGlue()
			.End()
			.Add(new BSeparatorView(B_HORIZONTAL))
		.End();
	fListView->AddStatusView(statusContainerView);
	
	// Standard buttons
	fEnableButton = new BButton(kLabelEnable,
		new BMessage(ENABLE_BUTTON_PRESSED));
	fDisableButton = new BButton(kLabelDisable, 
		new BMessage(DISABLE_BUTTON_PRESSED));
	
	// Create buttons with fixed size
	font_height fontHeight;
	GetFontHeight(&fontHeight);
	int16 buttonHeight = int16(fontHeight.ascent + fontHeight.descent + 12);
		// button size determined by font size
	BSize btnSize(buttonHeight,buttonHeight);
	
	fAddButton = new BButton("plus", "+", new BMessage(ADD_REPO_WINDOW));
	fAddButton->SetExplicitSize(btnSize);
	fRemoveButton = new BButton("minus", "-", new BMessage(REMOVE_REPOS));
	fRemoveButton->SetExplicitSize(btnSize);
	fAboutButton = new BButton("about", "?", new BMessage(SHOW_ABOUT));
	fAboutButton->SetExplicitSize(btnSize);
	
	// Layout
	int16 buttonSpacing = 1;
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.SetInsets(B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING,
			B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING)
		.AddGroup(B_HORIZONTAL, 0, 0.0)
			.Add(new BStringView("instruction",
				B_TRANSLATE_COMMENT("Select depots to use with Haiku package "\
					"management:", "Label text")), 0.0)
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
				.SetInsets(B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING,
					B_USE_DEFAULT_SPACING, 0)
				.AddGlue()
				.Add(fEnableButton)
				.Add(fDisableButton)
			.End()
		.End();
}


DepotsView::~DepotsView()
{
	if(fTaskLooper) {
		fTaskLooper->Lock();
		fTaskLooper->Quit();
	}
	_EmptyList();
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
DepotsView::MessageReceived(BMessage* message)
{
	switch(message->what)
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
			// Simulates pressing whichever is the enabled button
			if(fEnableButton->IsEnabled())
			{
				BMessage invokeMessage(ENABLE_BUTTON_PRESSED);
				MessageReceived(&invokeMessage);
			}
			else if(fDisableButton->IsEnabled())
			{
				BMessage invokeMessage(DISABLE_BUTTON_PRESSED);
				MessageReceived(&invokeMessage);
			}
			break;
		}
		case ENABLE_BUTTON_PRESSED: {
			BStringList names;
			bool paramsOK = true;
			// Check if there are multiple selections of the same depot,
			// pkgman won't like that
			RepoRow* rowItem = dynamic_cast<RepoRow*>(fListView->CurrentSelection());
			while(rowItem)
			{
				if(names.HasString(rowItem->Name())
					&& kNewRepoDefaultName.Compare(rowItem->Name()) != 0) {
					(new BAlert("duplicate",
						B_TRANSLATE_COMMENT("Only one URL for each depot can "
							"be enabled.  Please change your selections.",
							"Error message"),
						kOKLabel, NULL, NULL,
						B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go(NULL);
					paramsOK = false;
					break;
				}
				else
					names.Add(rowItem->Name());
				rowItem = dynamic_cast<RepoRow*>(fListView->CurrentSelection(rowItem));
			}
			if(paramsOK) {
				_AddSelectedRowsToQueue();
				_UpdateButtons();
			}
			break;
		}
		case DISABLE_BUTTON_PRESSED: {
			_AddSelectedRowsToQueue();
			_UpdateButtons();
			break;
		}
		case TASK_STARTED: {
			int16 count;
			status_t result1 = message->FindInt16(key_count, &count);
			RepoRow *rowItem;
			status_t result2 = message->FindPointer(key_rowptr, (void**)&rowItem);
			if(result1 == B_OK && result2 == B_OK)
				_TaskStarted(rowItem, count);
			break;
		}
		case TASK_COMPLETE_WITH_ERRORS: {
			BString errorDetails;
			status_t result = message->FindString(key_details, &errorDetails);
			if(result == B_OK) {
				// TODO seperate details view?
				(new BAlert("error", errorDetails, kOKLabel, NULL, NULL,
					B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go(NULL);
			}
			// Fall through
		}
		case TASK_COMPLETE: {
			BString repoName = message->GetString(key_name,
				kNewRepoDefaultName.String());
			int16 count;
			status_t result1 = message->FindInt16(key_count, &count);
			RepoRow *rowItem;
			status_t result2 = message->FindPointer(key_rowptr, (void**)&rowItem);
			if(result1 == B_OK && result2 == B_OK)
				_TaskCompleted(rowItem, count, message->what==TASK_COMPLETE,
					repoName);
			// If a repo was enabled, it is possible a repo on the same server
			// was disabled- need to refresh all
			_UpdatePkgmanList(true);
			_UpdateButtons();
			break;
		}
		case TASK_CANCELED: {
		//	(new BAlert("timeout", "Task canceled.", "OK"))->Go(NULL);
			int16 count;
			status_t result1 = message->FindInt16(key_count, &count);
			RepoRow *rowItem;
			status_t result2 = message->FindPointer(key_rowptr, (void**)&rowItem);
			if(result1 == B_OK && result2 == B_OK)
			{
				_TaskCanceled(rowItem, count);
			}
			// Update the list since it is unsure what the final status is
			_UpdatePkgmanList(true);
			_UpdateButtons();
			break;
		}
		case UPDATE_LIST: {
			_UpdatePkgmanList(true);
			_UpdateButtons();
			break;
		}
		case STATUS_VIEW_COMPLETED_TIMEOUT: {
			int32 timerID;
			status_t result = message->FindInt32(key_ID, &timerID);
			if(result == B_OK && timerID == fLastCompletedTimerId)
				_UpdateStatusView();
			break;
		}
		default:
			BView::MessageReceived(message);
	}
}


void
DepotsView::_AddSelectedRowsToQueue()
{
	RepoRow* rowItem = dynamic_cast<RepoRow*>(fListView->CurrentSelection());
	while(rowItem)
	{
		rowItem->SetTaskState(STATE_IN_QUEUE_WAITING);
		BMessage taskMessage(DO_TASK);
		taskMessage.AddPointer(key_rowptr, rowItem);
		fTaskLooper->PostMessage(&taskMessage);
		rowItem = dynamic_cast<RepoRow*>(fListView->CurrentSelection(rowItem));
	}
}


void
DepotsView::_TaskStarted(RepoRow *rowItem, int16 count)
{
	fRunningTaskCount = count;
	rowItem->SetTaskState(STATE_IN_QUEUE_RUNNING);
	// Only present a status count if there is more than one task in queue
	if(count > 1)
	{
		_UpdateStatusView();
		fShowCompletedStatus = true;
	}
}


void
DepotsView::_TaskCompleted(RepoRow *rowItem, int16 count, bool noErrors,
	BString& newName)
{
	fRunningTaskCount = count;
	// If this is the last task show completed status text for 3 seconds
	if(count==0 && fShowCompletedStatus)
	{
		fListStatusView->SetText(kStatusCompletedText);
		fLastCompletedTimerId = rand();
		BMessage timerMessage(STATUS_VIEW_COMPLETED_TIMEOUT);
		timerMessage.AddInt32(key_ID, fLastCompletedTimerId);
		new BMessageRunner(this, &timerMessage, 3000000, 1);
		fShowCompletedStatus = false;
	}
	else
		_UpdateStatusView();
	
	// Update row values
	rowItem->SetTaskState(STATE_NOT_IN_QUEUE);
	if(noErrors)
		rowItem->SetEnabled(!rowItem->IsEnabled());
	else
		rowItem->RefreshEnabledField();
	// Update a new repository name
	if(kNewRepoDefaultName.Compare(rowItem->Name()) == 0
		&& newName.Compare("") != 0)
		rowItem->SetName(newName.String());
}


void
DepotsView::_TaskCanceled(RepoRow *rowItem, int16 count)
{
	fRunningTaskCount = count;
	// If this is the last task show completed status text for 3 seconds
	if(count==0 && fShowCompletedStatus)
	{
		fListStatusView->SetText(kStatusCompletedText);
		fLastCompletedTimerId = rand();
		BMessage timerMessage(STATUS_VIEW_COMPLETED_TIMEOUT);
		timerMessage.AddInt32(key_ID, fLastCompletedTimerId);
		new BMessageRunner(this, &timerMessage, 3000000, 1);
		fShowCompletedStatus = false;
	}
	else
		_UpdateStatusView();
	// Update row values
	rowItem->SetTaskState(STATE_NOT_IN_QUEUE);
}


void
DepotsView::AddManualRepository(BString url)
{
	BString name(kNewRepoDefaultName);
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
			(new BAlert("duplicate",
				B_TRANSLATE_COMMENT("This depot URL already exists.",
					"Error message"),
				kOKLabel))->Go(NULL);
			return; 
		}
		//Find same root url
		if(foundRoot == false && rootUrl.Compare(urlPtr,
			rootUrl.Length()) == 0) {
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
DepotsView::_EmptyList()
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
			if(repoItem->TaskState() == STATE_NOT_IN_QUEUE)
				repoItem->SetEnabled(false);
		}
	}
	
	// Get list of current enabled repositories
	BStringList repositoryNames;
	BPackageKit::BPackageRoster pRoster;
	status_t result = pRoster.GetRepositoryNames(repositoryNames);
	if(result != B_OK) {
		(new BAlert("error",
			B_TRANSLATE_COMMENT("Depots could not retrieve the names of the "
				"currently enabled depots.", "Alert error message"),
			"OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT))->Go(NULL);
		return;
	}
	BPackageKit::BRepositoryConfig repoConfig;
	int16 index, count = repositoryNames.CountStrings();
	for(index=0; index < count; index++) {
		const BString& repoName = repositoryNames.StringAt(index);
		result = pRoster.GetRepositoryConfig(repoName, &repoConfig);
		if(result == B_OK)
			_AddRepo(repoName, repoConfig.BaseURL(), true);
	}
	_SaveList();
}

void
DepotsView::_SaveList()
{
	BStringList nameList, urlList;
	int32 index;
	int32 listCount = fListView->CountRows();
	for(index=0; index < listCount; index++) {
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
	RepoRow *addedRow=NULL;
	int32 index;
	int32 listCount = fListView->CountRows();
	// Find if the repo already exists in list
	for(index=0; index < listCount; index++) {
		RepoRow *repoItem = dynamic_cast<RepoRow*>((fListView->RowAt(index)));
		if(url.ICompare(repoItem->Url()) == 0) {
			// update name and enabled values
			if(name.Compare(repoItem->Name()) != 0)
				repoItem->SetName(name.String());
			repoItem->SetEnabled(enabled);
			addedRow = repoItem;
		}
	}
	if(addedRow == NULL) {
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
	if(rowItem) {
		bool someAreEnabled = false,
			someAreDisabled = false,
			someAreInQueue = false;
		int32 selectedCount=0;
		RepoRow *rowItem = dynamic_cast<RepoRow*>(fListView->CurrentSelection());
		while(rowItem) {
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
		if(selectedCount>1) {
			fEnableButton->SetLabel(kLabelEnableAll);
			fDisableButton->SetLabel(kLabelDisableAll);
		}
		else {
			fEnableButton->SetLabel(kLabelEnable);
			fDisableButton->SetLabel(kLabelDisable);
		}
		// Set which buttons should be enabled
		fRemoveButton->SetEnabled(!someAreEnabled && !someAreInQueue);
		if((someAreEnabled && someAreDisabled) || someAreInQueue) {
			// there are a mix of enabled and disabled depots selected
			fEnableButton->SetEnabled(false);
			fDisableButton->SetEnabled(false);	
		}
		else {
			fEnableButton->SetEnabled(someAreDisabled);
			fDisableButton->SetEnabled(someAreEnabled);
		}
		
	}
	// No selected rows
	else {
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
	if(fRunningTaskCount) {
		BString text(kStatusViewText);
		text.Append(" ");
		text<<fRunningTaskCount;
		fListStatusView->SetText(text);
	}
	else
		fListStatusView->SetText("");
}
