/* DepotsSettings.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef DEPOTS_SETTINGS_H
#define DEPOTS_SETTINGS_H

#include <File.h>
#include <Message.h>
#include <Path.h>
#include <Point.h>
#include <Rect.h>
#include <String.h>
#include <StringList.h>


class DepotsSettings {
public:
							DepotsSettings();
	BRect					GetFrame();
	void					SetFrame(BRect frame);
	status_t				GetRepositories(int32 &repoCount, BStringList &nameList, BStringList &urlList);
	void					AddRepository(BString name, BString url);
	void					RemoveRepository(const char *url);
	
private:
	BPath					fFilePath;
	BFile					fFile;
	status_t				fInitStatus;
	BMessage				_ReadFromFile();
	status_t				_SaveToFile(BMessage settings);
};

#endif
