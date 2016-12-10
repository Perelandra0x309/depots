/* Depots.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include <Alert.h>
#include <Catalog.h>
#include <Cursor.h>
#include <LayoutBuilder.h>
#include <Roster.h>

#include "constants.h"
#include "Depots.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DepotsApplication"

const char* kAppSignature = "application/x-vnd.BH-Depots";


URLView::URLView(const char *name, const char *url)
	:
	BStringView(name, url),
	fUrl(url),
	fMouseOver(false)
{ }


void
URLView::MouseMoved(BPoint where, uint32 code, const BMessage *dragMessage)
{
	switch(code)
	{
		case B_ENTERED_VIEW: {
			BCursor cursor(B_CURSOR_ID_FOLLOW_LINK);
			be_app->SetCursor(&cursor);
			fMouseOver = true;
			Invalidate();
			break;
		}
		case B_EXITED_VIEW: {
			BCursor cursor(B_CURSOR_ID_SYSTEM_DEFAULT);
			be_app->SetCursor(&cursor);
			fMouseOver = false;
			Invalidate();
			break;
		}
	}
}


void
URLView::Draw(BRect bounds)
{
	SetHighColor(ui_color(fMouseOver ? B_LINK_HOVER_COLOR : B_LINK_TEXT_COLOR));
	//TODO for some reason underlining is not working
	BFont font(be_plain_font);
	font.SetFace(B_UNDERSCORE_FACE);
	SetFont(&font, B_FONT_FACE);
	
	BStringView::Draw(bounds);
}


void
URLView::MouseDown(BPoint point)
{
	BRoster roster;
	BMessage msg(B_REFS_RECEIVED);
	msg.AddString("url", fUrl.String());
	roster.Launch("text/html", &msg);
}


DepotsApplication::DepotsApplication()
	:
	BApplication(kAppSignature)
{
	BRect mainWindowRect(50,50,80,80);
	fWindow = new DepotsWindow(mainWindowRect);
}


void
DepotsApplication::AboutRequested()
{
	BString text(B_TRANSLATE_COMMENT("Depots preflet version 1.0.0", "About box line 1"));
	text.Append("\n").Append(B_TRANSLATE_COMMENT("Copyright © 2016 by Brian Hill", "About box line 2"));
	text.Append("\n\n").Append(B_TRANSLATE_COMMENT("This preflet will enable and disable depots "\
								"used in the HaikuDepot application.", "About box line 3"));
	BAlert *aboutAlert = new BAlert("About", text, kOKLabel);
	aboutAlert->SetFlags(aboutAlert->Flags() | B_CLOSE_ON_ESCAPE);
	// Add clickable URL
	int strutSize = 5;
	URLView *urlLink = new URLView("url", kWebsiteUrl);
	float urlHeight;
	urlLink->GetPreferredSize(NULL, &urlHeight);
	BTextView *textView = aboutAlert->TextView();
	BLayoutBuilder::Group<>(textView, B_VERTICAL, 0)
		.AddGlue()
		.Add(urlLink)
		.AddStrut(strutSize);
	BSize viewSize = textView->MinSize();
	viewSize.height += 7*urlHeight + strutSize;
	textView->SetExplicitMinSize(viewSize);
	
	aboutAlert->Go();
}


int
main()
{
	DepotsApplication myApp;
	myApp.Run();
	return 0;
}
