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

//#include "CheckboxColumn.h"
#include "constants.h"
#include "DepotsView.h"

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


DepotsView::DepotsView()
	:
	BView("depotsview", B_SUPPORTS_LAYOUT),
	fTitleEnabled(B_TRANSLATE("Enabled")),
	fTitleName(B_TRANSLATE("Name")),
	fTitleUrl(B_TRANSLATE("URL")),
	fLabelRemove(B_TRANSLATE("Remove")),
	fLabelRemoveAll(B_TRANSLATE("Remove All")),
	fLabelEnable(B_TRANSLATE("Enable")),
	fLabelEnableAll(B_TRANSLATE("Enable All")),
	fLabelDisable(B_TRANSLATE("Disable")),
	fLabelDisableAll(B_TRANSLATE("Disable All"))
{
	fListView = new BColumnListView("list", B_NAVIGABLE, B_PLAIN_BORDER);
	fListView->SetSelectionMessage(new BMessage(LIST_SELECTION_CHANGED));
	float col0width = be_plain_font->StringWidth(fTitleEnabled) + 15;
	float col1width = be_plain_font->StringWidth(fTitleName) + 15;
	float col2width = be_plain_font->StringWidth(fTitleUrl) + 15;
//	fListView->AddColumn(new CheckboxColumn(B_TRANSLATE("Check"), 90, col0width, 100), kCheckboxColumn);
	fListView->AddColumn(new BStringColumn(fTitleEnabled, col0width, col0width, col0width,
		B_TRUNCATE_END, B_ALIGN_CENTER), kEnabledColumn);
	fListView->AddColumn(new BStringColumn(fTitleName, 90, col1width, 300,
		B_TRUNCATE_END), kNameColumn);
	fListView->AddColumn(new BStringColumn(fTitleUrl, 500, col2width, 5000,
		B_TRUNCATE_END), kUrlColumn);
	
	fEnableButton = new BButton(fLabelEnable, NULL);
	fDisableButton = new BButton(fLabelDisable, NULL);
	fAddButton = new BButton(B_TRANSLATE("Add" B_UTF8_ELLIPSIS), new BMessage(ADD_REPO_WINDOW));
	fRemoveButton = new BButton(fLabelRemove, new BMessage(REMOVE_REPOS));
	fEnableButton->SetEnabled(false);
	fDisableButton->SetEnabled(false);
	fRemoveButton->SetEnabled(false);
		
	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING)
//		.SetInsets(0, B_USE_WINDOW_SPACING, 0, B_USE_WINDOW_SPACING)
//		.AddGroup(B_VERTICAL, B_USE_DEFAULT_SPACING)
//			.SetInsets(B_USE_DEFAULT_SPACING, 0, B_USE_DEFAULT_SPACING, 0)
			.Add(new BStringView("instruction", B_TRANSLATE("Select depots to use in HaikuDepot:")))
			.Add(fListView)
			.AddGroup(B_HORIZONTAL)
				.Add(fAddButton)
				.Add(fRemoveButton)
				.AddGlue()
				.Add(fEnableButton)
				.Add(fDisableButton)
				.End();
	/*	.End()
		.Add(new BSeparatorView(B_HORIZONTAL))
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(fAddButton)
			.Add(fRemoveButton)
		.End();*/
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
		if(repoItem->Url() == url)
		{
			(new BAlert("duplicate", B_TRANSLATE("Depot already exists."), B_TRANSLATE("OK")))->Go();
			return; 
		}
	}
	BString name(B_TRANSLATE("Unknown"));
	_AddRepo(name, url, false);
	fSettings.AddRepository(name, url);
}


void
DepotsView::_InitList()
{
	// Get list of known repositories from the settings file
	int32 index;
	int32 repoCount = fSettings.CountRepositories();
	BString name, url;
	for(index=0; index < repoCount; index++)
	{
		status_t result = fSettings.GetRepository(index, &name, &url);
		if(result == B_OK)
			_AddRepo(name, url, false);
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
			name = pkgmanOutput.StringAt(0);
			url = pkgmanOutput.StringAt(1);
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
			fSettings.AddRepository(name, url);
		}
	}
	fListView->SetSortColumn(fListView->ColumnAt(kUrlColumn), false, true);
}


void
DepotsView::_AddRepo(BString name, BString url, bool enabled)
{
//	printf("Adding:%s:%s\n", name.String(), url.String());
	RepoRow *newRepo = new RepoRow(name, url, enabled);
	bool foundSibling = false;
	int32 index;
	int32 listCount = fListView->CountRows();
	// Find siblings
	for(index=0; index < listCount; index++)
	{
		RepoRow *repoItem = (RepoRow*)(fListView->RowAt(index));
		if(repoItem->Name() == name)
		{
			// duplicate url- match found, update enabled value
			if(repoItem->Url() == url)
			{
				repoItem->SetEnabled(enabled);
				return; 
			}
			if(enabled)
				repoItem->SetEnabled(false);
			repoItem->SetHasSibling(true);
			newRepo->SetHasSibling(true);
		}

	}
	fListView->AddRow(newRepo);
	
}


void
DepotsView::_UpdateButtons()
{
	RepoRow *rowItem = (RepoRow*)fListView->CurrentSelection();
	// At least one selected row
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
		// Change button labels depending on how many rows are selected
		if(selectedCount>1)
		{
			fRemoveButton->SetLabel(fLabelRemoveAll);
			fEnableButton->SetLabel(fLabelEnableAll);
			fDisableButton->SetLabel(fLabelDisableAll);
		}
		else
		{
			fRemoveButton->SetLabel(fLabelRemove);
			fEnableButton->SetLabel(fLabelEnable);
			fDisableButton->SetLabel(fLabelDisable);
		}
		// Set which buttons should be enabled
		fEnableButton->SetEnabled(someAreDisabled);
		fDisableButton->SetEnabled(someAreEnabled);
		fRemoveButton->SetEnabled(true);
	}
	// No selected rows
	else
	{
		fRemoveButton->SetLabel(fLabelRemove);
		fEnableButton->SetLabel(fLabelEnable);
		fDisableButton->SetLabel(fLabelDisable);
		fEnableButton->SetEnabled(false);
		fDisableButton->SetEnabled(false);
		fRemoveButton->SetEnabled(false);
	}
}
