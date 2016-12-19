/* DepotsWindow.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
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
#include "DepotsWindow.h"

#include <stdio.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DepotsWindow"


DepotsWindow::DepotsWindow()
	:
	BWindow(BRect(50,50,500,400), B_TRANSLATE_SYSTEM_NAME("Depots"), B_TITLED_WINDOW,
					B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS),
	fPackageNodeStatus(B_ERROR)
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
	status_t status = find_directory(B_SYSTEM_SETTINGS_DIRECTORY, &packagePath);
	if (status == B_OK) {
		status = packagePath.Append("package-repositories");
	}
	else
	{
		// /boot/system/cache/package-repositories
		status = find_directory(B_SYSTEM_CACHE_DIRECTORY, &packagePath);
		if (status == B_OK)
			status = packagePath.Append("package-repositories");
	}
	if (status == B_OK)
	{
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
	if(fPackageNodeStatus == B_OK)
	{
		status_t result = watch_node(&fPackageNodeRef, B_WATCH_DIRECTORY, this);
		fWatchingPackageNode = (result==B_OK);
//		printf("Watching node was %ssuccessful. Result = %i\n", fWatchingPackageNode ? "": "not ", result);
	}
}


void
DepotsWindow::_StopWatching()
{
	if(fPackageNodeStatus == B_OK && fWatchingPackageNode)// package-repositories directory is being watched
	{	
		watch_node(&fPackageNodeRef, B_STOP_WATCHING, this);
//		printf("Stopped watching node\n");
		fWatchingPackageNode = false;
	}
}


bool
DepotsWindow::QuitRequested()
{
	if(fView->IsTaskRunning())
	{
		int32 result = (new BAlert("tasks", B_TRANSLATE_COMMENT("Tasks are still running. Stop tasks "
										"and quit?", "Alert message"),
										"No", "Yes", NULL,
										B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
		if(result == 0)
			return false;
	}
	fSettings.SetFrame(Frame());
	be_app->PostMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}


void
DepotsWindow::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case ADD_REPO_WINDOW: {
			BRect frame = Frame();
			frame.right -= 2*kAddWindowOffset;
			new AddRepoWindow(frame, this);
			break;
		}
		case ADD_REPO_URL: {
			BString url;
			status_t result = msg->FindString(key_url, &url);
			if(result == B_OK)
				fView->AddManualRepository(url);
			break;
		}
//		case UPDATE_LIST:
		case TASK_STARTED:
		case TASK_COMPLETE_WITH_ERRORS:
		case TASK_COMPLETE:
		case TASK_CANCELED: {
			fView->MessageReceived(msg);
			break;
		}
		case SHOW_ABOUT: {
			be_app->AboutRequested();
			break;
		}
		case B_NODE_MONITOR: { // captures pkgman changes while Depots application is running
			// This app is making changes, so ignore this message
			if(fView->IsTaskRunning()) // TODO how to handle this now?
				break;
			
			int32 opcode;
			if (msg->FindInt32("opcode", &opcode) == B_OK)
			{	switch (opcode)
				{	case B_ATTR_CHANGED:
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
			BWindow::MessageReceived(msg);
	}
}

