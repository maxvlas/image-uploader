/*
    Image Uploader - program for uploading images/files to Internet
    Copyright (C) 2007-2011 ZendeN <zenden2k@gmail.com>
	 
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

#ifndef _NETWORK_MANAGER_H_
#define _NETWORK_MANAGER_H_

#include <string>
#include <vector>

#include <curl/curl.h>
//#include <curl/types.h>
#ifndef IU_CLI
#include <zthread/Mutex.h>
#endif

#include "Core/Utils/CoreUtils.h"

#define NString std::string

std::string nm_trimStr(const std::string& str);
void nm_splitString(const std::string& str, const std::string& delimiters, std::vector<std::string>& tokens, int maxCount = -1);

class NetworkManager
{
	public:
		enum ActionType {
			atNone = 0, atPost, atUpload, atGet
		};
		NetworkManager(void);
		~NetworkManager(void);
		void addQueryParam(const NString& name, const NString& value);
		void addQueryParamFile(const NString& name, const NString& fileName, const NString& displayName = "", const NString& contentType = "");
		void addQueryHeader(const NString& name, const NString& value);
		void setUrl(const NString& url);
		bool doPost(const NString& data="");
		bool doUploadMultipartData();
		bool doUpload(const NString& fileName,const NString &data);
		bool doGet(const std::string &url="");
		const std::string responseBody();
		int responseCode();
		const NString errorString();
		void setUserAgent(const NString& userAgentStr);
		NString responseHeaderText();
		NString responseHeaderByName(const NString& name);
		NString responseHeaderByIndex(const int index, NString& name);
		int responseHeaderCount();
		void setProgressCallback(curl_progress_callback func, void *data);
		const NString urlEncode(const NString& str);
		const NString getCurlResultString();
		void setCurlOption(int option, const NString &value);
		void setCurlOptionInt(int option, long value);
		void setMethod(const NString &str);
		void setProxy(const NString &host, int port, int type);
		void setProxyUserPassword(const NString &username, const NString password);
		void setReferer(const NString &str);
		void setOutputFile(const NString &str);
		void setUploadBufferSize(const int size);
		void setChunkOffset(double offset);
		void setChunkSize(double size);
		int getCurlResult();
		CURL* getCurlHandle();
		static void Uninitialize();
		void enableResponseCodeChecking(bool enable);
	private:
		enum CallBackFuncType{funcTypeBody,funcTypeHeader};

		struct CallBackData
		{
			CallBackFuncType funcType;
			NetworkManager* nmanager;
		};

		struct CustomHeaderItem
		{
			NString name;
			NString value;
		};

		struct QueryParam
		{
			bool isFile;
			NString name;
			NString value; // also filename
			NString displayName; 
			NString contentType;
		};

		static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream);
		static int ProgressFunc(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
		static int private_static_writer(char *data, size_t size, size_t nmemb, void *buffer_in);
		int private_writer(char *data, size_t size, size_t nmemb);
		int private_header_writer(char *data, size_t size, size_t nmemb);
		size_t private_read_callback(void *ptr, size_t size, size_t nmemb, void *stream);
		static int set_sockopts(void * clientp, curl_socket_t sockfd, curlsocktype purpose);
		bool private_apply_method();
		void private_parse_headers();
		void private_cleanup_before();
		void private_cleanup_after();
		bool private_on_finish_request();
		void private_initTransfer();
		void private_checkResponse();

		int m_UploadBufferSize;
		CURL *curl_handle;
		FILE *m_hOutFile;
		std::string m_OutFileName;
		FILE *m_uploadingFile;
		std::string m_uploadData;
		ActionType m_currentActionType;
		int m_nUploadDataOffset;
		CallBackData m_bodyFuncData;
		curl_progress_callback m_progressCallbackFunc;
		CallBackData m_headerFuncData;
		NString m_url;
		void* m_progressData;
		CURLcode curl_result;
		int64_t m_CurrentFileSize;
		int64_t m_currentUploadDataSize;
		std::vector<QueryParam> m_QueryParams;
		std::vector<CustomHeaderItem> m_QueryHeaders;
		std::vector<CustomHeaderItem> m_ResponseHeaders;
		std::string internalBuffer;
		std::string m_headerBuffer;
		NString m_userAgent;
		char m_errorBuffer[CURL_ERROR_SIZE];;
		std::string m_method;
		struct curl_slist * chunk_;
		bool enableResponseCodeChecking_;
		int64_t chunkOffset_;
		int64_t chunkSize_;
               #ifndef IU_CLI
		static ZThread::Mutex _mutex;
#endif
		static bool _curl_init;
		static bool _is_openssl;
		static char CertFileName[1024];
};

#endif
