/* constants.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef DEPOTS_CONSTANTS_H
#define DEPOTS_CONSTANTS_H

#define key_location "location"
#define key_name "repo_name"
#define key_url "repo_url"

//Messages
enum {
	ADD_REPO_WINDOW = 'BHDa',
	ADD_BUTTON_PRESSED,
	CANCEL_BUTTON_PRESSED,
	ADD_REPO_URL,
	REMOVE_REPOS,
	LIST_SELECTION_CHANGED
};


#endif