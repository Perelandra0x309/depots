/* DepotsSettings.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "DepotsSettings.h"

#include <File.h>
#include <FindDirectory.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DepotsSettings"

const char* settingsFilename = "Depots_settings";
const char* key_location = "location";
const char* key_name = "repo_name";
const char* key_url = "repo_url";


DepotsSettings::DepotsSettings()
{
	fLocation.Set(-10,-10);
	
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, &fFilePath);
	if (status == B_OK) {
		status = fFilePath.Append(settingsFilename);
		if (status == B_OK)
			_ReadFromFile();
	}
}


DepotsSettings::~DepotsSettings()
{
	// Empty repositories list
	int32 index, count;
	count = fReposList.CountItems();
	for(index=0; index < count; index++)
	{
		Repository *item = fReposList.ItemAt(0);
		fReposList.RemoveItem(0);
		delete item;
	}
}


void
DepotsSettings::SetLocation(BPoint location)
{
	fLocation = location;
	_SaveToFile();
}


status_t
DepotsSettings::_ReadFromFile()
{
	BFile file;
	status_t status = file.SetTo(fFilePath.Path(), B_READ_ONLY);
	if (status == B_OK) {
		BMessage settings;
		status = settings.Unflatten(&file);
		if(status == B_OK){
			// Get window location
			settings.FindPoint(key_location, &fLocation);
			
			// Get repositories
			int32 index, count;
			count = fReposList.CountItems();
			for(index=0; index < count; index++)
			{
				Repository *item = fReposList.ItemAt(0);
				fReposList.RemoveItem(0);
				delete item;
			}
			fReposList.MakeEmpty();
			type_code type;
			BString name, url;
			settings.GetInfo(key_name, &type, &count);
			for(index=0; index < count; index++)
			{
				status_t result1 = settings.FindString(key_name, &name);
				status_t result2 = settings.FindString(key_url, &url);
				if(result1 == B_OK && result2 == B_OK)
				{
					Repository *repo = new Repository();
					repo->name = name;
					repo->url = url;
					fReposList.AddItem(repo);
				}
			}
		}
	}
	return status;
}


status_t
DepotsSettings::_SaveToFile()
{
	BFile file;
	BMessage settings;
	status_t status = file.SetTo(fFilePath.Path(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	if (status == B_OK) {
		// Save window location
		settings.AddPoint(key_location, fLocation);
		
		// Save repositories
		int32 count = fReposList.CountItems();
		int index;
		for(index = 0; index < count; index++)
		{
			Repository *repo = fReposList.ItemAt(index);
			settings.AddString(key_name, repo->name);
			settings.AddString(key_url, repo->url);
		}
		
		status = settings.Flatten(&file);
	}
	return status;
}
