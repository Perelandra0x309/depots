/* DepotsView.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include <Button.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <ColumnTypes.h>
#include <Layout.h>
#include <LayoutBuilder.h>
#include <SeparatorView.h>
#include <StringView.h>

#include "constants.h"
#include "DepotsView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DepotsView"


DepotsView::DepotsView()
	:
	BView("depotsview", B_WILL_DRAW)
{
	fListView = new BColumnListView("list", B_NAVIGABLE, B_PLAIN_BORDER);
	float col0width = be_plain_font->StringWidth(B_TRANSLATE("Enabled")) + 15;
	float col1width = be_plain_font->StringWidth(B_TRANSLATE("Repository")) + 15;
	fListView->AddColumn(new BStringColumn(B_TRANSLATE("Enabled"), col0width, col0width, col0width,
		B_TRUNCATE_MIDDLE), kEnabledColumn);
	fListView->AddColumn(new BStringColumn(B_TRANSLATE("Repository"), 500, col1width, 5000,
		B_TRUNCATE_END), kRepositoryColumn);
		
	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(0, B_USE_WINDOW_SPACING, 0, B_USE_WINDOW_SPACING)
		.AddGroup(B_VERTICAL, B_USE_DEFAULT_SPACING)
			.SetInsets(B_USE_DEFAULT_SPACING, 0, B_USE_DEFAULT_SPACING, 0)
			.Add(new BStringView("instruction", "Select depots to use in HaikuDepot:"))
			.Add(fListView)
		.End()
		.Add(new BSeparatorView(B_HORIZONTAL))
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(new BButton(B_TRANSLATE("Add" B_UTF8_ELLIPSIS), new BMessage(ADD_REPO_WINDOW)))
			.Add(new BButton(B_TRANSLATE("Remove" B_UTF8_ELLIPSIS), new BMessage(REMOVE_REPO_WINDOW)))
		.End();
}


DepotsView::~DepotsView()
{
	Clean();
}

/*
void
DepotsView::AllAttached()
{
	BView::AllAttached();
	
}*/

/*
void
MainView::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		
		default:
			BView::MessageReceived(msg);
	}
}*/


status_t
DepotsView::Clean()
{
	BRow* row;
	while ((row = fListView->RowAt((int32)0, NULL)) != NULL) {
		fListView->RemoveRow(row);
		delete row;
	}
	return B_OK;
/*	BLayout *layout = fReposView->GetLayout();
	int32 count = layout->CountItems();
	int32 index;
	for(index=0; index < count; index++)
	{
		BLayoutItem *item = layout->RemoveItem(int32(0));
		delete item;//TODO does this free everything???
	}
	return layout->CountItems()==0 ? B_OK : B_ERROR;*/
}


void
DepotsView::AddRepository(Repository *repo)
{
	BString label(repo->name);
	label.Append(": ");
	label.Append(repo->urlList.StringAt(0));
	//TODO multiple urls
//	BCheckBox *newCB = new BCheckBox(repo->name, label, NULL);
//	BLayout *layout = fReposView->GetLayout();
	BRow *newRow = new BRow();
	if(repo->selectedUrl >= 0)
		newRow->SetField(new BStringField(B_TRANSLATE("Yes")), kEnabledColumn);
	else
		newRow->SetField(new BStringField(B_TRANSLATE("No")), kEnabledColumn);
	newRow->SetField(new BStringField(label.String()), kRepositoryColumn);
	fListView->AddRow(newRow);
	
/*	BString label(repo->name);
	label.Append(": ");
	label.Append(repo->urlList.StringAt(0));
	//TODO multiple urls
	BCheckBox *newCB = new BCheckBox(repo->name, label, NULL);
	BLayout *layout = fReposView->GetLayout();
	layout->AddView(newCB);
	if(repo->selectedUrl >= 0)
		newCB->SetValue(true);*/
}
