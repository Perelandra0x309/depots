/* DepotsSettings.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef DEPOTS_SETTINGS_H
#define DEPOTS_SETTINGS_H

#include <ObjectList.h>
#include <Path.h>
#include <Point.h>
#include <String.h>


struct Repository{
	BString name, url;
};


class DepotsSettings {
public:
							DepotsSettings();
							~DepotsSettings();
	BPoint					GetLocation(){ return fLocation; }
	void					SetLocation(BPoint location);
						
private:
	BPoint					fLocation;
	BPath					fFilePath;
	BObjectList<Repository>		fReposList;
	status_t				_ReadFromFile();
	status_t				_SaveToFile();
};

#endif
