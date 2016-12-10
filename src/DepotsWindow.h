/* DepotsWindow.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef DEPOTS_WINDOW_H
#define DEPOTS_WINDOW_H

#include <Looper.h>
#include <Node.h>
#include <String.h>
#include <StringList.h>
#include <Window.h>

#include "DepotsSettings.h"
#include "DepotsView.h"


class DepotsWindow : public BWindow {
public:
							DepotsWindow(BRect size);
							~DepotsWindow();
	virtual	bool			QuitRequested();
	virtual void			MessageReceived(BMessage*);
private:
	DepotsSettings			fSettings;
	DepotsView				*fView;
	node_ref				fPackageNodeRef;//node_ref to watch for changes to package-repositories directory
	status_t				fPackageNodeStatus;
//	BLooper					*fWatchingLooper;
	bool					fWatchingPackageNode;//true when package-repositories directory is being watched
	void					_StartWatching();
	void					_StopWatching();
};

#endif
