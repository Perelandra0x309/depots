/* DepotsSettings.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef DEPOTS_SETTINGS_H
#define DEPOTS_SETTINGS_H

#include <Path.h>
#include <Point.h>

class DepotsSettings {
public:
						DepotsSettings();
	BPoint				GetLocation(){ return fLocation; }
	void				SetLocation(BPoint location);
						
private:
	BPoint				fLocation;
	BPath				fFilePath;
	status_t			_ReadFromFile();
	status_t			_SaveToFile();
};

#endif
