/* DepotsView.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include <Button.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <GroupView.h>
#include <LayoutBuilder.h>
#include <SeparatorView.h>

#include "DepotsView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DepotsView"


DepotsView::DepotsView()
	:
	BView("depotsview", B_WILL_DRAW)
{
	BGroupView *reposView = new BGroupView();
	BCheckBox *testCB = new BCheckBox("reponame", "Repo", NULL);
	BLayoutBuilder::Group<>(reposView, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.Add(testCB)
	.End();
	
	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.Add(reposView)
		.AddStrut(B_USE_DEFAULT_SPACING)
		.Add(new BSeparatorView(B_HORIZONTAL))
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.SetInsets(B_USE_WINDOW_SPACING, 0, B_USE_WINDOW_SPACING,
			B_USE_WINDOW_SPACING)
			.Add(new BButton(B_TRANSLATE("Add" B_UTF8_ELLIPSIS)))
			.Add(new BButton(B_TRANSLATE("Remove" B_UTF8_ELLIPSIS)))
		.End();
}

/*
DepotsView::~DepotsView()
{
	
}*/

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
