/* DepotsSettings.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef DEPOTS_SETTINGS_H
#define DEPOTS_SETTINGS_H

#include <Message.h>
#include <Path.h>
#include <Point.h>
#include <String.h>


class DepotsSettings {
public:
							DepotsSettings();
	BPoint					GetLocation();
	void					SetLocation(BPoint location);
	int32					CountRepositories();
	status_t				GetRepository(int32 index, BString *name, BString *url);
	void					AddRepository(BString name, BString url);
	void					RemoveRepository(const char *url);
	
private:
	BPath					fFilePath;
	BMessage				fSettings;
	status_t				_ReadFromFile();
	status_t				_SaveToFile();
};

#endif
