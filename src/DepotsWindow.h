/* DepotsWindow.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef DEPOTS_WINDOW_H
#define DEPOTS_WINDOW_H


#include <Node.h>
#include <Window.h>

#include "AddRepoWindow.h"
#include "DepotsSettings.h"
#include "DepotsView.h"


class DepotsWindow : public BWindow {
public:
							DepotsWindow();
							~DepotsWindow();
	virtual	bool			QuitRequested();
	virtual void			MessageReceived(BMessage*);

private:
	DepotsSettings			fSettings;
	DepotsView				*fView;
	AddRepoWindow			*fAddWindow;
	node_ref				fPackageNodeRef;
		//node_ref to watch for changes to package-repositories directory
	status_t				fPackageNodeStatus;
	bool					fWatchingPackageNode;
		//true when package-repositories directory is being watched
	void					_StartWatching();
	void					_StopWatching();
};

#endif
