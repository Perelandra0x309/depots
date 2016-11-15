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
		if(status == B_OK)
			settings.FindPoint(key_location, &fLocation);
		
	}
	return status;
}

status_t
DepotsSettings::_SaveToFile()
{
	BFile file;
	status_t status = file.SetTo(fFilePath.Path(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	if (status == B_OK) {
		BMessage settings;
		settings.AddPoint(key_location, fLocation);
		status = settings.Flatten(&file);
	}
	return status;
}
