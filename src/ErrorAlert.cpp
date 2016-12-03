/* ErrorAlert.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */

#include <Catalog.h>
#include <LayoutBuilder.h>
#include <Roster.h>

#include "constants.h"
#include "ErrorAlert.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ErrorAlert.cpp"

ErrorAlert::ErrorAlert(BPath file, const char* title, const char* text,
								const char* button0Label, const char* button1Label,
								const char* button2Label,
								button_width widthStyle,
								alert_type type)
	:
	BAlert(title, text, button0Label, button1Label, button2Label, widthStyle, type),
	fOutputPath(file)
{ }


void
ErrorAlert::MessageReceived(BMessage* msg)
{
	if (msg->what == 'ALTB')
	{
		// find which button was pressed
		int32 selection;
		msg->FindInt32("which", &selection);
		if(selection == 0)
		{
			BEntry entry(fOutputPath.Path());
			entry_ref ref;
			entry.GetRef(&ref);
			be_roster->Launch(&ref);
		}
	}
	BAlert::MessageReceived(msg);
}
