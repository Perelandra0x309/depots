/* Repository.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef DEPOTS_REPOSITORY_H
#define DEPOTS_REPOSITORY_H

#include <String.h>
#include <StringList.h>


struct Repository{
	BString name;
	BStringList urlList;
	int32 selectedUrl;
};

#endif
