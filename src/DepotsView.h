/* DepotsView.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef DEPOTS_VIEW_H
#define DEPOTS_VIEW_H

#include <GroupView.h>
#include <View.h>

#include "Repository.h"


class DepotsView : public BView {
public:
					DepotsView();
//					~DepotsView();
//	virtual void	AllAttached();
//	virtual void	MessageReceived(BMessage*);
	status_t		Clean();
	void			AddRepository(Repository *repo);
private:
	BGroupView		*fReposView;
};

#endif
