/* constants.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef DEPOTS_CONSTANTS_H
#define DEPOTS_CONSTANTS_H


#include <Catalog.h>
#include <String.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Constants"

static const float kAddWindowWidth = 500.0;
static const float kAddWindowOffset = 10.0;
static const int16 kTimerAlertOffset = 15;
static const int16 kTimerTimeoutSeconds = 10;

static const BString kOKLabel = B_TRANSLATE_COMMENT("OK", "Button label");
static const BString kCancelLabel = B_TRANSLATE_COMMENT("Cancel",
	"Button label");
static const BString kWebsiteUrl = "http://perelandra0x309.github.io/depots";
static const BString kNewRepoDefaultName = B_TRANSLATE_COMMENT("Unknown",
	"Unknown depot name");

typedef struct {
	const char* name;
	const char* url;
} Repository;

static const Repository kDefaultRepos[] = {
	{ "Haiku", "http://packages.haiku-os.org/haiku/master/"B_HAIKU_ABI_NAME
		"/current"},
	{ "Haikuports", "http://packages.haiku-os.org/haikuports/master/repo/"
		B_HAIKU_ABI_NAME"/current" },
	{ "BeSly Software Solutions", "http://software.besly.de/repo"},
	{ "clasqm's repo", "http://clasquin-johnson.co.za/michel/repo"},
	{ "FatElk", "http://coquillemartialarts.com/fatelk/repo"}
};

//Message keys
#define key_frame "frame"
#define key_name "repo_name"
#define key_url "repo_url"
#define key_text "text"
#define key_details "details"
#define key_rowptr "row_ptr"
#define key_taskptr "task_ptr"
#define key_count "count"
#define key_ID "ID"

//Messages
enum {
	ADD_REPO_WINDOW = 'BHDa',
	ADD_BUTTON_PRESSED,
	CANCEL_BUTTON_PRESSED,
	ADD_REPO_URL,
	ADD_WINDOW_CLOSED,
	REMOVE_REPOS,
	LIST_SELECTION_CHANGED,
	ENABLE_BUTTON_PRESSED,
	DISABLE_BUTTON_PRESSED,
	ITEM_INVOKED,
	DO_TASK,
	STATUS_VIEW_COMPLETED_TIMEOUT,
	TASK_STARTED,
	TASK_COMPLETED,
	TASK_COMPLETED_WITH_ERRORS,
	TASK_CANCELED,
	UPDATE_LIST,
	SHOW_ABOUT,
	NO_TASKS,
	ENABLE_DEPOT,
	DISABLE_DEPOT,
	TASK_TIMEOUT,
	TIMEOUT_ALERT_BUTTON_SELECTION,
	TASK_KILL_REQUEST
};

// Repo row task state
enum {
	STATE_NOT_IN_QUEUE = 0,
	STATE_IN_QUEUE_WAITING,
	STATE_IN_QUEUE_RUNNING
};

#endif
