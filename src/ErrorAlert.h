/* ErrorAlert.h
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#ifndef ERRORALERT_H
#define ERRORALERT_H

#include <Alert.h>
#include <Path.h>

class ErrorAlert : public BAlert {
public:
						ErrorAlert(BPath file, const char* title, const char* text,
								const char* button0Label, const char* button1Label = NULL,
								const char* button2Label = NULL,
								button_width widthStyle = B_WIDTH_AS_USUAL,
								alert_type alert = B_INFO_ALERT);
	virtual	void		MessageReceived(BMessage *msg);
private:
	BPath				fOutputPath;
};

#endif
