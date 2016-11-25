/* DepotsWindow.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include <Application.h>
#include <Catalog.h>
#include <File.h>
#include <LayoutBuilder.h>
#include <Screen.h>
#include <StringList.h>
#include <stdlib.h>
#include <stdio.h>

#include "AddRepoWindow.h"
#include "constants.h"
#include "DepotsWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DepotsWindow"


DepotsWindow::DepotsWindow(BRect size)
	:
	BWindow(size, B_TRANSLATE_SYSTEM_NAME("Depots"), B_TITLED_WINDOW, /*B_NOT_RESIZABLE |*/ B_NOT_ZOOMABLE |
		B_ASYNCHRONOUS_CONTROLS /*| B_AUTO_UPDATE_SIZE_LIMITS*/)
{
	_CreateRepoList();
	
	fView = new DepotsView();
	_PopulateDepotsView();
	
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.Add(fView)
	.End();
	
	// Location on screen
	MoveTo(fSettings.GetLocation());
	BScreen screen;
	BRect screenFrame = screen.Frame();
	BRect frame = Frame();
	if(screenFrame.right < frame.right || screenFrame.left > frame.left
		|| screenFrame.top > frame.top || screenFrame.bottom < frame.bottom)
		CenterOnScreen();
	
	Show();
}


DepotsWindow::~DepotsWindow()
{
	// Empty repositories list
	int32 index, count;
	count = fReposList.CountItems();
	for(index=0; index < count; index++)
	{
		Repository *item = fReposList.ItemAt(0);
		item->urlList.MakeEmpty();
		fReposList.RemoveItem(0);
		delete item;
	}
}


bool
DepotsWindow::QuitRequested()
{
	fSettings.SetLocation(Frame().LeftTop());
	be_app->PostMessage(B_QUIT_REQUESTED);
	return BWindow::QuitRequested();
}


void
DepotsWindow::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
		case ADD_REPO_WINDOW: {
			new AddRepoWindow(Frame(), this);
			break;
		}
		case ADD_REPO_URL: {
			BString url;
			status_t result = msg->FindString(key_url, &url);
			if(result == B_OK)
			{
				
	/*			BString command("pkgman add ");
				command.Append(url);
				int sysResult = system(command.String());
				printf("Result of add:%i", sysResult);
				if(sysResult==0)
				{
					_CreateRepoList();
					_PopulateDepotsView();
				}*/
				
			}
			break;
		}
		default:
			BWindow::MessageReceived(msg);
	}
}


void
DepotsWindow::_CreateRepoList()
{
	// Empty list
	int32 index;
	int32 listCount = fReposList.CountItems();
	for(index=0; index < listCount; index++)
	{
		Repository *item = fReposList.ItemAt(0);
		fReposList.RemoveItem(0);
		delete item;
	}
			
	// Get list of known repositories from the settings file
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
	
}


void
DepotsWindow::_AddRepo(BString name, BString url, bool enabled)
{
//	printf("Adding:%s:%s\n", name.String(), url.String());
	bool foundRepo = false;
	int32 index;
	int32 listCount = fReposList.CountItems();
	for(index=0; index < listCount; index++)
	{
		Repository *repo = fReposList.ItemAt(index);
		// Found an existing repo with the same name
		if(repo->name == name)
		{
			foundRepo = true;
			// url not in list yet
			if(!repo->urlList.HasString(url));
			{
				repo->urlList.Add(url);
				// sort list, need to keep track of existing enabled url
				if(repo->selectedUrl >= 0)
				{
					BString selectedUrl = repo->urlList.StringAt(repo->selectedUrl);
					repo->urlList.Sort();
					repo->selectedUrl = repo->urlList.IndexOf(selectedUrl);
				}
				// no enabled url
				else
					repo->urlList.Sort();
			}
			if(enabled)
				repo->selectedUrl = repo->urlList.IndexOf(url);
			continue;
		}

	}
	// Did not find existing repo, add one
	if(!foundRepo)
	{
		Repository *repo = new Repository;
		repo->name = name;
		repo->urlList.Add(url);
		repo->selectedUrl = enabled ? 0 : -1;
		fReposList.AddItem(repo);
		
		//TODO Sort repo list---
		
	}
}


void
DepotsWindow::_PopulateDepotsView()
{
	// Remove all items from the view
	fView->Clean();
	
	// Repopulate view with current repositories
	int32 index;
	int32 listCount = fReposList.CountItems();
	for(index=0; index < listCount; index++)
	{
		Repository *repo = fReposList.ItemAt(index);
		fView->AddRepository(repo);
	}
	fView->Invalidate();
}
