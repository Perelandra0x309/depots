/* RepoRow.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include <ColumnTypes.h>

#include "constants.h"
#include "RepoRow.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "RepoRow"


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