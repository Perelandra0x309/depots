/* DepotsSettings.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "constants.h"
#include "DepotsSettings.h"

#include <File.h>
#include <FindDirectory.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DepotsSettings"

const char* settingsFilename = "Depots_settings";


DepotsSettings::DepotsSettings()
{
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, &fFilePath);
	if (status == B_OK) {
		status = fFilePath.Append(settingsFilename);
		if (status == B_OK)
			_ReadFromFile();
	}
}


BPoint
DepotsSettings::GetLocation()
{
	BPoint location;
	fSettings.FindPoint(key_location, &location);
	return location;
}


void
DepotsSettings::SetLocation(BPoint location)
{
	fSettings.RemoveData(key_location);
	fSettings.AddPoint(key_location, location);
	_SaveToFile();
}


int32
DepotsSettings::CountRepositories()
{
	type_code type;
	int32 count;
	fSettings.GetInfo(key_name, &type, &count);
	return count;
}


status_t
DepotsSettings::GetRepository(int32 index, BString *name, BString *url)
{
	status_t result1 = fSettings.FindString(key_name, index, name);
	status_t result2 = fSettings.FindString(key_url, index, url);
	return result1 == B_OK && result2 == B_OK ? B_OK : B_ERROR;
}


void
DepotsSettings::AddRepository(BString name, BString url)
{
	int32 count = CountRepositories();
	bool exists = false;
	int index;
	BString foundName, foundUrl;
	// determine if the repository already exists
	for(index = 0; index < count; index++)
	{
		if(GetRepository(index, &foundName, &foundUrl) == B_OK)
		{
			if(foundName == name && foundUrl == url)
				exists = true;
		}
	}
	if(!exists)
	{
		fSettings.AddString(key_name, name);
		fSettings.AddString(key_url, url);
		_SaveToFile();
	}
}


status_t
DepotsSettings::_ReadFromFile()
{
	BFile file;
	status_t status = file.SetTo(fFilePath.Path(), B_READ_ONLY);
	if (status == B_OK) {
		fSettings.MakeEmpty();
		status = fSettings.Unflatten(&file);
	}
	return status;
}


status_t
DepotsSettings::_SaveToFile()
{
	BFile file;
	status_t status = file.SetTo(fFilePath.Path(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	if (status == B_OK)
		status = fSettings.Flatten(&file);
	return status;
}
