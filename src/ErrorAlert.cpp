/* ErrorAlert.cpp
 * Copyright 2016 Brian Hill
 * All rights reserved. Distributed under the terms of the BSD License.
 */
#include "ErrorAlert.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ErrorAlert.cpp"

ErrorAlert::ErrorAlert(const char* title, const char* text,
								const char* button0Label, const char* button1Label,
								const char* button2Label,
								button_width widthStyle,
								alert_type type)
	:
	BAlert(title, text, button0Label, button1Label, button2Label, widthStyle, type)
{
	float minWidth;
	GetSizeLimits(&minWidth, NULL, NULL, NULL);
	minWidth += 450;
	SetSizeLimits(minWidth, 9999, 0, 9999);
}
