/* DepotsWindow.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */


#include "DepotsWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Catalog.h>
#include <FindDirectory.h>
#include <LayoutBuilder.h>
#include <NodeMonitor.h>
#include <Region.h>
#include <Screen.h>

#include "AddRepoWindow.h"
#include "constants.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DepotsWindow"


DepotsWindow::DepotsWindow()
	:
	BWindow(BRect(50,50,500,400), B_TRANSLATE_SYSTEM_NAME("Depots"),
		B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS),
	fPackageNodeStatus(B_ERROR),
	fAddWindow(NULL)
{
	fView = new DepotsView();
	BLayoutBuilder::Group<>(this, B_VERTICAL).Add(fView);
	
	// Size and location on screen
	BSize viewSize(fView->MinSize());
	SetSizeLimits(viewSize.Width(), 9999, viewSize.Height(), 9999);
	BRect frame = fSettings.GetFrame();
	ResizeTo(frame.Width(), frame.Height());
	BScreen screen;
	BRect screenFrame = screen.Frame();
	if(screenFrame.right < frame.right || screenFrame.left > frame.left
		|| screenFrame.top > frame.top || screenFrame.bottom < frame.bottom)
		CenterOnScreen();
	else
		MoveTo(frame.left, frame.top);
	Show();
	
	// Find the pkgman settings or cache directory
	BPath packagePath;
	// /boot/system/settings/package-repositories
	status_t status = find_directory(B_SYSTEM_SETTINGS_DIRECTORY,
		&packagePath);
	if (status == B_OK)
		status = packagePath.Append("package-repositories");
	else {
		// /boot/system/cache/package-repositories
		status = find_directory(B_SYSTEM_CACHE_DIRECTORY, &packagePath);
		if (status == B_OK)
			status = packagePath.Append("package-repositories");
	}
	if (status == B_OK) {
		BNode packageNode(packagePath.Path());
		if(packageNode.InitCheck()==B_OK && packageNode.IsDirectory())
			fPackageNodeStatus = packageNode.GetNodeRef(&fPackageNodeRef);
	}

	// watch the pkgman settings or cache directory for changes
	_StartWatching();
}


DepotsWindow::~DepotsWindow()
{
	_StopWatching();
}


void
DepotsWindow::_StartWatching()
{
	if(fPackageNodeStatus == B_OK) {
		status_t result = watch_node(&fPackageNodeRef, B_WATCH_DIRECTORY, this);
		fWatchingPackageNode = (result==B_OK);
	}
}


void
DepotsWindow::_StopWatching()
{
	if(fPackageNodeStatus == B_OK && fWatchingPackageNode) {	
		watch_node(&fPackageNodeRef, B_STOP_WATCHING, this);
		fWatchingPackageNode = false;
	}
}


bool
DepotsWindow::QuitRequested()
{
	if(fView->IsTaskRunning()) {
		BAlert *alert = new BAlert("tasks",
			B_TRANSLATE_COMMENT("Some tasks are still running. Stop these "
				"tasks and quit?", "Application quit alert message"),
			B_TRANSLATE_COMMENT("Stop and quit", "Button label"),
			kCancelLabel, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
		alert->SetFlags(alert->Flags() | B_CLOSE_ON_ESCAPE);
		int32 result = alert->Go();
		if(result != 0)
			return false;
	}
	fSettings.SetFrame(Frame());
	be_app->PostMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}


void
DepotsWindow::MessageReceived(BMessage* message)
{
	switch(message->what)
	{
		case ADD_REPO_WINDOW: {
			BRect frame = Frame();
		//	frame.right -= 2*kAddWindowOffset;
			fAddWindow = new AddRepoWindow(frame, this);
			break;
		}
		case ADD_REPO_URL: {
			BString url;
			status_t result = message->FindString(key_url, &url);
			if(result == B_OK)
				fView->AddManualRepository(url);
			break;
		}
		case ADD_WINDOW_CLOSED: {
			fAddWindow = NULL;
			break;
		}
		case TASK_STARTED:
		case TASK_COMPLETED_WITH_ERRORS:
		case TASK_COMPLETED:
		case TASK_CANCELED: {
			fView->MessageReceived(message);
			break;
		}
		case SHOW_ABOUT: {
			be_app->AboutRequested();
			break;
		}
		// captures pkgman changes while the Depots application is running
		case B_NODE_MONITOR: {
			// This preflet is making the changes, so ignore this message
			if(fView->IsTaskRunning())
				break;
			
			int32 opcode;
			if (message->FindInt32("opcode", &opcode) == B_OK) {
				switch (opcode)
				{
					case B_ATTR_CHANGED:
					case B_ENTRY_CREATED:
					case B_ENTRY_REMOVED: {
						PostMessage(UPDATE_LIST, fView);
						break;
					}
				}
			}
			break;
		}
		default:
			BWindow::MessageReceived(message);
	}
}
