/* DepotsView.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include <Alert.h>
#include <Button.h>
#include <Catalog.h>
#include <ColumnTypes.h>
#include <File.h>
#include <Layout.h>
#include <LayoutBuilder.h>
#include <SeparatorView.h>
#include <StringList.h>
#include <StringView.h>
#include <stdlib.h>
#include <stdio.h>

#include "constants.h"
#include "DepotsView.h"
#include "TaskWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DepotsView"


RepoRow::RepoRow(const char* repo_name, const char* repo_url, bool enabled)
	:
	BRow(),
	fName(repo_name),
	fUrl(repo_url),
	fEnabled(enabled),
	fHasSibling(false)
{
	SetField(new BStringField(""), kEnabledColumn);
	SetField(new BStringField(fName.String()), kNameColumn);
	SetField(new BStringField(fUrl.String()), kUrlColumn);
	if(enabled)
		SetEnabled(enabled);	
}


void
RepoRow::SetEnabled(bool enabled)
{
	BStringField *field = (BStringField*)GetField(kEnabledColumn);
	field->SetString(enabled ? "√" : "");
	fEnabled = enabled; 
}


void
RepoRow::SetName(const char *name)
{
	BStringField *field = (BStringField*)GetField(kNameColumn);
	field->SetString(name);
	fName.SetTo(name); 
}


DepotsView::DepotsView()
	:
	BView("depotsview", B_SUPPORTS_LAYOUT),
	fTitleEnabled(B_TRANSLATE_COMMENT("Enabled", "Column title")),
	fTitleName(B_TRANSLATE_COMMENT("Name", "Column title")),
	fTitleUrl(B_TRANSLATE_COMMENT("URL", "Column title")),
	fLabelRemove(B_TRANSLATE_COMMENT("Remove", "Button label")),
	fLabelRemoveAll(B_TRANSLATE_COMMENT("Remove All", "Button label")),
	fLabelEnable(B_TRANSLATE_COMMENT("Enable", "Button label")),
	fLabelEnableAll(B_TRANSLATE_COMMENT("Enable All", "Button label")),
	fLabelDisable(B_TRANSLATE_COMMENT("Disable", "Button label")),
	fLabelDisableAll(B_TRANSLATE_COMMENT("Disable All", "Button label")),
	fUsingMinimalButtons(true)
{
	fListView = new BColumnListView("list", B_NAVIGABLE, B_PLAIN_BORDER);
	fListView->SetSelectionMessage(new BMessage(LIST_SELECTION_CHANGED));
	float col0width = be_plain_font->StringWidth(fTitleEnabled) + 15;
	float col1width = be_plain_font->StringWidth(fTitleName) + 15;
	float col2width = be_plain_font->StringWidth(fTitleUrl) + 15;
	fListView->AddColumn(new BStringColumn(fTitleEnabled, col0width, col0width, col0width,
		B_TRUNCATE_END, B_ALIGN_CENTER), kEnabledColumn);
	fListView->AddColumn(new BStringColumn(fTitleName, 90, col1width, 300,
		B_TRUNCATE_END), kNameColumn);
	fListView->AddColumn(new BStringColumn(fTitleUrl, 500, col2width, 5000,
		B_TRUNCATE_END), kUrlColumn);
	
	fEnableButton = new BButton(fLabelEnable, new BMessage(ENABLE_BUTTON_PRESSED));
	fDisableButton = new BButton(fLabelDisable, new BMessage(DISABLE_BUTTON_PRESSED));
	fEnableButton->SetEnabled(false);
	fDisableButton->SetEnabled(false);
	

if(fUsingMinimalButtons) {
	// ---Minimal buttons option---
	BView *button1View = new BView("button1View", B_WILL_DRAW);
	int buttonSize = 25;
	BRect btnSize(0,0,buttonSize,buttonSize);
	fAddButton = new BButton(btnSize, "plus", "+", new BMessage(ADD_REPO_WINDOW));
//	fAddButton->SetExplicitMaxSize(BSize(25,25));
//	fAddButton->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_VERTICAL_CENTER));
//	fAddButton->SetFlat(true);
	button1View->SetExplicitMinSize(BSize(buttonSize, buttonSize));
	
	BView *button2View = new BView("button2View", B_WILL_DRAW);
	fRemoveButton = new BButton(btnSize, "minus", "-", new BMessage(REMOVE_REPOS));
//	fRemoveButton->SetExplicitMaxSize(BSize(25,25));
//	fRemoveButton->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_VERTICAL_CENTER));
//	fRemoveButton->SetFlat(true);
	fRemoveButton->SetEnabled(false);
	button2View->SetExplicitMinSize(BSize(buttonSize, buttonSize));
	
	button1View->AddChild(fAddButton);
	button2View->AddChild(fRemoveButton);
	
	int buttonSpacing = 1;
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.SetInsets(B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING)
		.Add(new BStringView("instruction", B_TRANSLATE("Select depots to use in HaikuDepot:")))
		.AddStrut(B_USE_DEFAULT_SPACING)
		.Add(fListView, 1)
		
		.AddGroup(B_HORIZONTAL, 0, 0.0)
			// Add and Remove buttons
			.AddGroup(B_VERTICAL, 0, 0.0)
				.AddGroup(B_HORIZONTAL, 0, 0.0)
					.Add(new BSeparatorView(B_VERTICAL))
					.AddGroup(B_VERTICAL, 0, 0.0)
						.AddGroup(B_HORIZONTAL, buttonSpacing, 0.0)
							.SetInsets(buttonSpacing,buttonSpacing,buttonSpacing,buttonSpacing+1)
							.Add(button1View)
							.Add(button2View)
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
}
else {
	
		// ---Standard buttons option---
	fAddButton = new BButton(B_TRANSLATE_COMMENT("Add" B_UTF8_ELLIPSIS, "Button label"),
							new BMessage(ADD_REPO_WINDOW));
	fRemoveButton = new BButton(fLabelRemove, new BMessage(REMOVE_REPOS));
	fRemoveButton->SetEnabled(false);
	
	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING)
		.Add(new BStringView("instruction", B_TRANSLATE("Select depots to use in HaikuDepot:")))
		.Add(fListView)
		.AddGroup(B_HORIZONTAL)
			.Add(fAddButton)
			.Add(fRemoveButton)
			.AddGlue()
			.Add(fEnableButton)
			.Add(fDisableButton)
		.End();
	// -- End standard buttons section
}

}


DepotsView::~DepotsView()
{
	Clean();
}


void
DepotsView::AllAttached()
{
	BView::AllAttached();
	fRemoveButton->SetTarget(this);
	fEnableButton->SetTarget(this);
	fDisableButton->SetTarget(this);
	_InitList();
}


void
DepotsView::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case REMOVE_REPOS :{
			RepoRow *rowItem = (RepoRow*)fListView->CurrentSelection();
			while(rowItem)
			{
				//TODO check if repo is enabled- drop it first?
				fListView->RemoveRow(rowItem);
				fSettings.RemoveRepository(rowItem->Url());
				delete rowItem;
				rowItem = (RepoRow*)fListView->CurrentSelection();
			}
			break;
		}
		case LIST_SELECTION_CHANGED: {
			_UpdateButtons();
			break;
		}
		case ENABLE_BUTTON_PRESSED: {
			BStringList params, names;
			int32 index;
			int32 count = fListView->CountRows();
			bool paramsOK = true;
			// Add repository name of each selected item that is disabled in pkgman
			for(index=0; index < count; index++)
			{
				RepoRow* rowItem = (RepoRow*)fListView->RowAt(index);
				if(rowItem->IsSelected() && !rowItem->IsEnabled())
				{
					params.Add(rowItem->Url());
					// Check if there are multiple selections of the same depot, pkgman won't like that
					if(names.HasString(rowItem->Name()))
					{
						(new BAlert("duplicate", B_TRANSLATE_COMMENT("You can only enable one URL for "
										"each depot.  Please change your selections.", "Error message"),
										B_TRANSLATE_COMMENT("OK", "Alert button label"), NULL, NULL,
										B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go(NULL);
						paramsOK = false;
						break;
					}
					else
						names.Add(rowItem->Name());
				}
			}
			if(paramsOK)
			{
				TaskWindow *win = new TaskWindow(Window()->Frame(), this->Looper(), msg->what, params);
				win->PostMessage(DO_TASKS);
			}
			break;
		}
		case DISABLE_BUTTON_PRESSED: {
			BStringList params;
			int32 index;
			int32 count = fListView->CountRows();
			// Add repository name of each selected item that is enabled in pkgman
			for(index=0; index < count; index++)
			{
				RepoRow* rowItem = (RepoRow*)fListView->RowAt(index);
				if(rowItem->IsSelected() && rowItem->IsEnabled())
					params.Add(rowItem->Name());
			}
			TaskWindow *win = new TaskWindow(Window()->Frame(), this->Looper(), msg->what, params);
			win->PostMessage(DO_TASKS);
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


status_t
DepotsView::Clean()
{
	BRow* row;
	while ((row = fListView->RowAt((int32)0, NULL)) != NULL) {
		fListView->RemoveRow(row);
		delete row;
	}
	return B_OK;
}


void
DepotsView::AddManualRepository(BString url)
{
	int32 index;
	int32 listCount = fListView->CountRows();
	// Find duplicate
	for(index=0; index < listCount; index++)
	{
		RepoRow *repoItem = (RepoRow*)(fListView->RowAt(index));
		if(url.ICompare(repoItem->Url()) == 0)
		{
			(new BAlert("duplicate", B_TRANSLATE("Depot already exists."), B_TRANSLATE("OK")))->Go();
			return; 
		}
	}
	BString name(B_TRANSLATE("Unknown"));
	RepoRow *newRepo = _AddRepo(name, url, false);
	fListView->DeselectAll();
	fListView->AddToSelection(newRepo);
	_UpdateButtons();
	fSettings.AddRepository(name, url);
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
			RepoRow *repoItem = (RepoRow*)(fListView->RowAt(index));
			repoItem->SetEnabled(false);
		}
	}
	
	// Get list of current enabled repositories from pkgman
	int sysResult = system("pkgman list > /boot/home/pkglist");//TODO where to save temp file?  Delete after.
//	printf("result=%i", sysResult);
	BFile listFile("/boot/home/pkglist", B_READ_ONLY);
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
			if(!updateStatusOnly)
				fSettings.AddRepository(name, url);
		}
	}
	//TODO move this
	fListView->SetSortColumn(fListView->ColumnAt(kUrlColumn), false, true);
}


RepoRow*
DepotsView::_AddRepo(BString name, BString url, bool enabled)
{
//	printf("Adding:%s:%s\n", name.String(), url.String());
//	RepoRow *newRepo = new RepoRow(name, url, enabled);
	RepoRow *addedRow=NULL;
//	bool foundSibling = false;
	int32 index;
	int32 listCount = fListView->CountRows();
	// Find siblings
	for(index=0; index < listCount; index++)
	{
		RepoRow *repoItem = (RepoRow*)(fListView->RowAt(index));
		if(url.ICompare(repoItem->Url()) == 0)
		{
			// update name and enabled values
			if(name.Compare(repoItem->Name()) != 0)
			{
				repoItem->SetName(name.String());
				fSettings.AddRepository(name, url);
			}
			repoItem->SetEnabled(enabled);
			addedRow = repoItem;
		}
		else if(name.Compare(repoItem->Name()) == 0)
		{	
			repoItem->SetEnabled(false);
//			repoItem->SetHasSibling(true);
//			if(addedRow)
//				addedRow->SetHasSibling(true);
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
	RepoRow *rowItem = (RepoRow*)fListView->CurrentSelection();
	// At least one row is selected
	if(rowItem)
	{
		bool someAreEnabled = false, someAreDisabled = false;
		int32 index, selectedCount=0;
		int32 count = fListView->CountRows();
		for(index=0; index < count; index++)
		{
			rowItem = (RepoRow*)fListView->RowAt(index);
			if(rowItem->IsSelected())
			{
				selectedCount++;
				if(rowItem->IsEnabled())
					someAreEnabled = true;
				else
					someAreDisabled = true;
			}
		}
		// Change button labels depending on which rows are selected
		if(selectedCount>1)
		{
			if(!fUsingMinimalButtons)
				fRemoveButton->SetLabel(fLabelRemoveAll);
			fEnableButton->SetLabel(fLabelEnableAll);
			fDisableButton->SetLabel(fLabelDisableAll);
		}
		else
		{
			if(!fUsingMinimalButtons)
				fRemoveButton->SetLabel(fLabelRemove);
			fEnableButton->SetLabel(fLabelEnable);
			fDisableButton->SetLabel(fLabelDisable);
		}
		// Set which buttons should be enabled
		fRemoveButton->SetEnabled(!someAreEnabled);
		if(someAreEnabled && someAreDisabled)
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
		if(!fUsingMinimalButtons)
			fRemoveButton->SetLabel(fLabelRemove);
		fEnableButton->SetLabel(fLabelEnable);
		fDisableButton->SetLabel(fLabelDisable);
		fEnableButton->SetEnabled(false);
		fDisableButton->SetEnabled(false);
		fRemoveButton->SetEnabled(false);
	}
}
