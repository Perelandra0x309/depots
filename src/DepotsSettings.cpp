/* DepotsSettings.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "constants.h"
#include "DepotsSettings.h"

#include <FindDirectory.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DepotsSettings"

const char* settingsFilename = "Depots_settings";


DepotsSettings::DepotsSettings()
{
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, &fFilePath);
	if (status == B_OK) {
		status = fFilePath.Append(settingsFilename);
	}
	fInitStatus = status;
}


BRect
DepotsSettings::GetFrame()
{
	BMessage settings(_ReadFromFile());
	BRect frame;
	settings.FindRect(key_frame, &frame);
	return frame;
}


void
DepotsSettings::SetFrame(BRect frame)
{
	BMessage settings(_ReadFromFile());
	settings.RemoveData(key_frame);
	settings.AddRect(key_frame, frame);
	_SaveToFile(settings);
}


status_t
DepotsSettings::GetRepositories(int32 &repoCount, BStringList &nameList, BStringList &urlList)
{
	BMessage settings(_ReadFromFile());
	type_code type;
	int32 count;
	settings.GetInfo(key_name, &type, &count);
	
	status_t result = B_OK;
	int32 index, total=0;
	BString foundName, foundUrl;
	// get each repository and add to lists
	for(index = 0; index < count; index++)
	{
		status_t result1 = settings.FindString(key_name, index, &foundName);
		status_t result2 = settings.FindString(key_url, index, &foundUrl);
		if(result1 == B_OK && result2 == B_OK)
		{
			nameList.Add(foundName);
			urlList.Add(foundUrl);
			total++;
		}
		else
			result = B_ERROR;
	}
	repoCount = total;
	return result;
}


void
DepotsSettings::AddRepository(BString name, BString url)
{
	BMessage settings(_ReadFromFile());
	type_code type;
	int32 count;
	settings.GetInfo(key_name, &type, &count);
	
	bool exists = false;
	int32 index;
	BString foundName, foundUrl;
	// determine if the repository already exists
	for(index = 0; index < count; index++)
	{
		status_t result1 = settings.FindString(key_name, index, &foundName);
		status_t result2 = settings.FindString(key_url, index, &foundUrl);
		if(result1 == B_OK && result2 == B_OK)
		{
			if(foundName == name && foundUrl == url)
				exists = true;
		}
	}
	if(!exists)
	{
		settings.AddString(key_name, name);
		settings.AddString(key_url, url);
		_SaveToFile(settings);
	}
}


void
DepotsSettings::RemoveRepository(const char *url)
{
	BMessage settings(_ReadFromFile());
	type_code type;
	int32 count;
	settings.GetInfo(key_name, &type, &count);
	
	int32 index;
	BString foundName, foundUrl;
	// determine if the repository already exists
	for(index = 0; index < count; index++)
	{
		status_t result1 = settings.FindString(key_name, index, &foundName);
		status_t result2 = settings.FindString(key_url, index, &foundUrl);
		if(result1 == B_OK && result2 == B_OK)
		{
			if(foundUrl.ICompare(url) == 0)
			{
				settings.RemoveData(key_name, index);
				settings.RemoveData(key_url, index);
				_SaveToFile(settings);
			}
		}
	}
}


BMessage
DepotsSettings::_ReadFromFile()
{
	BMessage settings;
	status_t status = fFile.SetTo(fFilePath.Path(), B_READ_ONLY);
	if (status == B_OK)
		status = settings.Unflatten(&fFile);
	fFile.Unset();
	return settings;
}


status_t
DepotsSettings::_SaveToFile(BMessage settings)
{
	status_t status = fFile.SetTo(fFilePath.Path(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	if (status == B_OK)
		status = settings.Flatten(&fFile);
	fFile.Unset();
	return status;
}
