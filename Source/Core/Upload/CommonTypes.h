/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2010 ZendeN <zenden2k@gmail.com>

    HomePage:    http://zenden.ws/imageuploader

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef IU_COMMONTYPES_H
#define IU_COMMONTYPES_H

#include "Core/Utils/CoreUtils.h"

struct InfoProgress
{
	int64_t Uploaded, Total;
	bool IsUploading;
	void clear()
	{
		Uploaded = 0;
		Total = 0;
		IsUploading = false;
	}
};

enum StatusType {
	stNone = 0, stUploading, stWaitingAnswer,  stAuthorization, stPerformingAction, stCreatingFolder,
	stUserDescription
};

enum ErrorType {
	etNone, etOther, etRepeating, etRetriesLimitReached, etActionRepeating, etActionRetriesLimitReached,
	etRegularExpressionError, etNetworkError, etUserError
};

struct ErrorInfo
{
	enum MessageType {
		mtError, mtWarning
	};
	Utf8String error;
	Utf8String Url;
	Utf8String ServerName;
	Utf8String FileName;
	int ActionIndex;
	MessageType messageType;
	ErrorType errorType;
	int RetryIndex;
	Utf8String sender;

	ErrorInfo()
	{
		RetryIndex = -1;
		ActionIndex = -1;
	}

	void Clear()
	{
		error.clear();
		Url.clear();
		ServerName.clear();
		FileName.clear();
		sender.clear();
		ActionIndex = -1;
		// messageType = mtNone;
		errorType = etNone;
		RetryIndex = -1;
	}
};
#endif
