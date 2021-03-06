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

#ifndef NOMINMAX
#define NOMINMAX
#endif 

#include "NetworkManager.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <memory.h>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include "Core/Utils/CoreUtils.h"
#include <Core/Logging.h>

char NetworkManager::CertFileName[1024]= "";

size_t simple_read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	return  fread(ptr, size, nmemb, (FILE*)stream);
}

int NetworkManager::set_sockopts(void * clientp, curl_socket_t sockfd, curlsocktype purpose) 
{
	#ifdef _WIN32
		// See http://support.microsoft.com/kb/823764
		NetworkManager* nm = reinterpret_cast<NetworkManager*>(clientp);
		int val = nm->m_UploadBufferSize + 32;
		setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (const char *)&val, sizeof(val));
	#endif
	return 0;
}

int NetworkManager::private_static_writer(char *data, size_t size, size_t nmemb, void *buffer_in)
{
	CallBackData* cbd = reinterpret_cast<CallBackData*>(buffer_in);
	NetworkManager* nm = cbd->nmanager;
	if(nm)
	{
		if(cbd->funcType == funcTypeBody)
		{
			return nm->private_writer(data, size, nmemb);
		}
		else
			return nm->private_header_writer(data, size, nmemb);
	}
	return 0;
}

void NetworkManager::setProxy(const NString &host, int port, int type)
{
	curl_easy_setopt(curl_handle, CURLOPT_PROXY, host.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_PROXYPORT, (long)port);	
	curl_easy_setopt(curl_handle, CURLOPT_PROXYTYPE, (long)type);
	curl_easy_setopt(curl_handle, CURLOPT_NOPROXY, "localhost,127.0.0.1"); // test
} 

void NetworkManager::setProxyUserPassword(const NString &username, const NString password)
{
	if(username.empty());
		//curl_easy_setopt(curl_handle, CURLOPT_PROXYUSERPWD,"");
	else
	{
		std::string authStr = username+":"+password;
		curl_easy_setopt(curl_handle, CURLOPT_PROXYUSERPWD, authStr.c_str());
	}
}

int NetworkManager::private_writer(char *data, size_t size, size_t nmemb)
{
	if(!m_OutFileName.empty())
	{
		if(!m_hOutFile)
			if(!(m_hOutFile = IuCoreUtils::fopen_utf8(m_OutFileName.c_str(), "wb")))
				return 0;
		fwrite(data, size,nmemb, m_hOutFile);
	}
	else
		internalBuffer.append(data, size * nmemb);
	return size * nmemb;
}

int NetworkManager::private_header_writer(char *data, size_t size, size_t nmemb)
{
	m_headerBuffer.append(data, size * nmemb);
	return size * nmemb;
}

int NetworkManager::ProgressFunc(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	NetworkManager *nm = reinterpret_cast<NetworkManager*>(clientp);
	if(nm && nm->m_progressCallbackFunc)
	{
		if  (nm->chunkOffset_>=0 && nm->chunkSize_>0 && nm->m_currentActionType == atUpload) {
			ultotal = nm->m_CurrentFileSize;
			ulnow = nm->chunkOffset_ + ulnow;
		} else if( ((ultotal<=0 && nm->m_CurrentFileSize>0)) && nm->m_currentActionType == atUpload)
			ultotal = double(nm->m_CurrentFileSize);
		return nm->m_progressCallbackFunc(nm->m_progressData, dltotal, dlnow, ultotal, ulnow);
	}
	return 0;
}

void NetworkManager::setMethod(const NString &str)
{
	m_method = str;
}

bool  NetworkManager::_curl_init = false;
bool  NetworkManager::_is_openssl = false;
#ifndef IU_CLI
ZThread::Mutex NetworkManager::_mutex;
#endif
NetworkManager::NetworkManager(void)
{
    #ifndef IU_CLI
	_mutex.acquire();
#endif
	if(!_curl_init)
	{
		enableResponseCodeChecking_ = true;
		curl_global_init(CURL_GLOBAL_ALL);
		curl_version_info_data * infoData = curl_version_info(CURLVERSION_NOW);
		_is_openssl =  strstr(infoData->ssl_version, "WinSSL")!=infoData->ssl_version;
#ifdef WIN32
		GetModuleFileNameA(0, CertFileName, 1023);
		int i, len = lstrlenA(CertFileName);
		for(i=len; i>=0; i--)
		{
			if(CertFileName[i] == _T('\\')) {
				CertFileName[i+1] = 0;
				break;
			}
		}
		StrCatA(CertFileName, "curl-ca-bundle.crt");
#endif
		_curl_init = true;
	}
    #ifndef IU_CLI
	_mutex.release();
#endif
	m_hOutFile = 0;
	chunkOffset_ = -1;
	chunkSize_ = -1;
	chunk_ = 0;
	m_CurrentFileSize = -1;
	m_uploadingFile = NULL;
	*m_errorBuffer = 0;
	m_progressCallbackFunc = NULL;
	curl_handle = curl_easy_init(); // Initializing libcurl
	m_bodyFuncData.funcType = funcTypeBody;
	m_bodyFuncData.nmanager = this;
	m_UploadBufferSize = 65536;
	m_headerFuncData.funcType = funcTypeHeader;
	m_headerFuncData.nmanager = this;
	m_nUploadDataOffset = 0;
	curl_easy_setopt(curl_handle, CURLOPT_COOKIELIST, "");
	setUserAgent("Mozilla/5.0");


	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, private_static_writer);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &m_bodyFuncData);	
	curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, &m_headerFuncData);
	curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, m_errorBuffer);
	
	curl_easy_setopt(curl_handle, CURLOPT_PROGRESSFUNCTION, &ProgressFunc);
	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(curl_handle, CURLOPT_PROGRESSDATA, this);
        curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl_handle, CURLOPT_ENCODING, "");
	curl_easy_setopt(curl_handle, CURLOPT_SOCKOPTFUNCTION, &set_sockopts);
	curl_easy_setopt(curl_handle, CURLOPT_SOCKOPTDATA, this);
	 
#ifdef _WIN32
	curl_easy_setopt(curl_handle, CURLOPT_CAINFO, CertFileName);
#endif
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 1L); 
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 2L);
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L); 
	curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);

	//We want the referrer field set automatically when following locations
	curl_easy_setopt(curl_handle, CURLOPT_AUTOREFERER, 1L); 
	curl_easy_setopt(curl_handle, CURLOPT_BUFFERSIZE, 32768L);
	   curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);
}

NetworkManager::~NetworkManager(void)
{
	curl_easy_setopt(curl_handle, CURLOPT_PROGRESSFUNCTION, (long)NULL);
	curl_easy_cleanup(curl_handle);
}

void NetworkManager::addQueryParam(const NString& name, const NString& value)
{
	QueryParam newParam;
	newParam.name = name;
	newParam.value = value;
	newParam.isFile = false;
	m_QueryParams.push_back(newParam);
}

void NetworkManager::addQueryParamFile(const NString& name, const NString& fileName, const NString& displayName, const NString& contentType)
{
	QueryParam newParam;
	newParam.name = name;
	newParam.value = fileName;
	newParam.isFile = true;
	newParam.contentType = contentType;
	newParam.displayName= displayName;
	m_QueryParams.push_back(newParam);
}

void NetworkManager::setUrl(const NString& url)
{
	m_url = url;
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
}

void CloseFileList(std::vector<FILE *>& files)
{
	for(size_t i=0; i<files.size(); i++)
	{
		fclose(files[i]);
	}
	files.clear();
}

bool NetworkManager::doUploadMultipartData()
{
	private_initTransfer();
	std::vector<FILE *> openedFiles;

	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;

	{
		std::vector<QueryParam>::iterator it, end = m_QueryParams.end();

		for(it=m_QueryParams.begin(); it!=end; it++)
		{
			if(it->isFile)
			{
					curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, simple_read_callback);
                                        std::string fileName = it->value;
					FILE * curFile = IuCoreUtils::fopen_utf8(it->value.c_str(), "rb"); /* open file to upload */
					if(!curFile) 
					{
						CloseFileList(openedFiles);
						return false; /* can't continue */
					}
					openedFiles.push_back(curFile);
					// FIXME: 64bit file size support!
                    long  curFileSize = IuCoreUtils::getFileSize(fileName);

				if(it->contentType.empty())
					curl_formadd(&formpost,
						&lastptr,
						CURLFORM_COPYNAME, it->name.c_str(),
						CURLFORM_FILENAME, it->displayName.c_str(),
						CURLFORM_STREAM, /*it->value.c_str()*/curFile,
						CURLFORM_CONTENTSLENGTH, curFileSize,
						CURLFORM_END);
				else 
					curl_formadd(&formpost,
						&lastptr,
						CURLFORM_COPYNAME, it->name.c_str(),
						CURLFORM_FILENAME, it->displayName.c_str(),
						CURLFORM_STREAM, /*it->value.c_str()*/curFile,
						CURLFORM_CONTENTSLENGTH, curFileSize,
						CURLFORM_CONTENTTYPE, it->contentType.c_str(),
						CURLFORM_END);
			}
			else
			{
				 curl_formadd(&formpost,
						&lastptr,
						CURLFORM_COPYNAME, it->name.c_str(),
						CURLFORM_COPYCONTENTS, it->value.c_str(),
						CURLFORM_END);
			}
		}
	}

	curl_easy_setopt(curl_handle, CURLOPT_HTTPPOST, formpost);
	m_currentActionType = atUpload;
	curl_result = curl_easy_perform(curl_handle);
	CloseFileList(openedFiles);
	curl_formfree(formpost);
	return private_on_finish_request();
}

bool NetworkManager::private_on_finish_request()
{
	private_checkResponse();
	private_cleanup_after();
	private_parse_headers();
	if (curl_result != CURLE_OK)
	{
		return false; //fail
	}

	return true;
}

const std::string NetworkManager::responseBody()
{
	return internalBuffer;
}

int NetworkManager::responseCode()
{
    long result=-1;
	curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &result);
	return result;
}

void NetworkManager::addQueryHeader(const NString& name, const NString& value)
{
	CustomHeaderItem chi;
	chi.name = name;
	chi.value = nm_trimStr(value);
	m_QueryHeaders.push_back(chi);
}

bool NetworkManager::doGet(const std::string & url)
{
	if(!url.empty())
		setUrl(url);

	private_initTransfer();
	if(!private_apply_method())
		curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
	m_currentActionType = atGet;
	curl_result = curl_easy_perform(curl_handle);
	return private_on_finish_request();

}

bool NetworkManager::doPost(const NString& data)
{
	private_initTransfer();
	if(!private_apply_method())
   curl_easy_setopt(curl_handle, CURLOPT_POST, 1L);
	std::string postData;
	std::vector<QueryParam>::iterator it, end = m_QueryParams.end();

		for(it=m_QueryParams.begin(); it!=end; it++)
		{
			if(!it->isFile)
			{
				postData+= urlEncode(it->name)+"="+urlEncode(it->value)+"&";
			}
		}

	if(data.empty()) {
		curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, postData.c_str());
	}
	else {
		curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, data.c_str());
	}

	m_currentActionType = atPost;	
	curl_result = curl_easy_perform(curl_handle);
	return private_on_finish_request();
}

const NString NetworkManager::urlEncode(const NString& str)
{
	char * encoded = curl_easy_escape(curl_handle, str.c_str() , str.length() );
	std::string res = encoded;
	res+="";
	curl_free(encoded);
	return res;
}
const NString NetworkManager::errorString()
{
	return m_errorBuffer;
}

void NetworkManager::setUserAgent(const NString& userAgentStr)
{
	m_userAgent = userAgentStr;
}

void NetworkManager::private_initTransfer()
{
	private_cleanup_before();
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, m_userAgent.c_str());

	std::vector<CustomHeaderItem>::iterator it, end = m_QueryHeaders.end();
	chunk_ = NULL;

	for(it = m_QueryHeaders.begin(); it!=end; it++)
	{
		chunk_ = curl_slist_append(chunk_, (it->name + ": " + it->value).c_str());
	}

	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, chunk_);
}

void NetworkManager::private_checkResponse()
{
	if ( !enableResponseCodeChecking_ )  {
		return;
	}
	int code = responseCode();
	if ( ( !code || (code>= 400 && code<=499)) && errorString() != "Callback aborted" ) {
		LOG(ERROR) << "Request to the URL '"<<m_url<<"' failed. \r\nResponse code: "<<code<<"\r\n"<<errorString()<<"\r\n"<<internalBuffer;
	}
}

NString NetworkManager::responseHeaderText()
{
	return m_headerBuffer;
}

void nm_splitString(const std::string& str, const std::string& delimiters, std::vector<std::string>& tokens, int maxCount)
{
    // Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);
	int counter =0;
    while (std::string::npos != pos || std::string::npos != lastPos)
    {
		 counter++;
		 if(counter == maxCount){
			 tokens.push_back(str.substr(lastPos, str.length()));break;
		 }
		 else

        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}
void NetworkManager::setProgressCallback(curl_progress_callback func, void *data)
{
	m_progressCallbackFunc = func;
	m_progressData = data;
}

std::string nm_trimStr(const std::string& str)
{
	std::string res;
	// Trim Both leading and trailing spaces
	size_t startpos = str.find_first_not_of(" \t\r\n"); // Find the first character position after excluding leading blank spaces
	size_t endpos = str.find_last_not_of(" \t\r\n"); // Find the first character position from reverse af

	// if all spaces or empty return an empty string
	if(( std::string::npos == startpos ) || ( std::string::npos == endpos))
	{
       res = "";
	}
   else
       res = str.substr( startpos, endpos-startpos+1 );
	return res;
}

#include <iostream>
void NetworkManager::private_parse_headers()
{
	std::vector<std::string> headers;
	nm_splitString(m_headerBuffer, "\n",headers);
	std::vector<std::string>::iterator it;

	for(it=headers.begin(); it!=headers.end(); it++)
	{
		std::vector<std::string> thisHeader;
		nm_splitString(*it, ":",thisHeader, 2);

		if(thisHeader.size() == 2)
		{
			CustomHeaderItem chi;
			chi.name = nm_trimStr(thisHeader[0]);
			chi.value = nm_trimStr(thisHeader[1]);
			m_ResponseHeaders.push_back(chi);
		}
	}
}

NString NetworkManager::responseHeaderByName(const NString& name)
{
	std::vector<CustomHeaderItem>::iterator it, end = m_ResponseHeaders.end();
	
	for(it = m_ResponseHeaders.begin(); it!=end; it++)
	{
		if(it->name == name)
			return it->value;
	}
	return "";

}

int NetworkManager::responseHeaderCount()
{
	return m_ResponseHeaders.size();
}

NString NetworkManager::responseHeaderByIndex(const int index, NString& name)
{
	name = m_ResponseHeaders[index].name;
	return m_ResponseHeaders[index].value;
}

void NetworkManager::private_cleanup_before()
{
	std::vector<CustomHeaderItem>::iterator it, end = m_QueryHeaders.end();

	bool add = true;
	for(it = m_QueryHeaders.begin(); it!=end; it++)
	{
		if(it->name == "Expect" ) { add = false; break; }
	}
	addQueryHeader("Expect", "");
	m_ResponseHeaders.clear();
	internalBuffer.clear();
	m_headerBuffer.clear();
}

void NetworkManager::private_cleanup_after()
{
	m_currentActionType = atNone;
	m_QueryHeaders.clear();
	m_QueryParams.clear();
	if(m_hOutFile)
	{
		fclose(m_hOutFile);
		m_hOutFile = 0;
	}
	m_OutFileName.clear();
	m_method = "";
	curl_easy_setopt(curl_handle, CURLOPT_INFILESIZE_LARGE, (curl_off_t )-1);

	m_uploadData.clear();
	m_uploadingFile = NULL;
	chunkOffset_ = -1;
	chunkSize_ = -1;
	enableResponseCodeChecking_ = true;
	m_nUploadDataOffset = 0;
	/*curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, 0L);
	curl_easy_setopt(curl_handle, CURLOPT_READDATA, 0L);*/
	if(chunk_)
	{
		curl_slist_free_all(chunk_);
		chunk_ = 0;
	}
}

size_t NetworkManager::read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	NetworkManager* nm = reinterpret_cast<NetworkManager*>(stream);;
	if(!nm) return 0;
	return nm->private_read_callback(ptr, size, nmemb, stream);
} 

size_t NetworkManager::private_read_callback(void *ptr, size_t size, size_t nmemb, void *)
{
	size_t retcode;
	int wantsToRead = size * nmemb;
	if(m_uploadingFile)
		retcode = fread(ptr, size, nmemb, m_uploadingFile);
	else
	{
		// dont event try to remove "<>" brackets!!
		int canRead = std::min<>((int)m_uploadData.size()-m_nUploadDataOffset, (int)wantsToRead);
		memcpy(ptr, m_uploadData.c_str(),canRead);
		m_nUploadDataOffset+=canRead;
		retcode = canRead;
	}
	return retcode;
}


bool NetworkManager::doUpload(const NString& fileName, const NString &data)
{
	if(!fileName.empty())
	{
		m_uploadingFile = IuCoreUtils::fopen_utf8(fileName.c_str(), "rb"); /* open file to upload */
		if(!m_uploadingFile) 
		{
			LOG(ERROR)<< "Failed to open file '" << fileName << "'";
			return false; /* can't continue */
		}
		m_CurrentFileSize = IuCoreUtils::getFileSize(fileName);
		m_currentUploadDataSize = m_CurrentFileSize;
		if(m_CurrentFileSize < 0) 
			return false;
		//LOG(ERROR)<<"chunkSize_="<<chunkSize_<<" chunkOffset_="<<chunkOffset_;
		if ( chunkSize_  >0 && chunkOffset_ >= 0 ) {
			m_currentUploadDataSize = chunkSize_;
			if ( fseek(m_uploadingFile, chunkOffset_, SEEK_SET)) {
				LOG(ERROR) << "Cannot seek to offset " << chunkOffset_ << " in source file.";
			}
		} 
		m_currentActionType = atUpload;
	}
	else
	{
		m_nUploadDataOffset =0;
		m_uploadData = data;
		m_CurrentFileSize = data.length();
		m_currentUploadDataSize = m_CurrentFileSize;
		m_currentActionType = atPost;
	}
	curl_easy_setopt(curl_handle, CURLOPT_READFUNCTION, read_callback);
		if(!private_apply_method())
	curl_easy_setopt(curl_handle, CURLOPT_POST, 1L);
	curl_easy_setopt(curl_handle,CURLOPT_POSTFIELDS, NULL);
	curl_easy_setopt(curl_handle, CURLOPT_READDATA, this);
	
	if ( m_method != "PUT" ) {
		addQueryHeader("Content-Length", IuCoreUtils::int64_tToString(m_currentUploadDataSize));
	}
	private_initTransfer();
	curl_easy_setopt(curl_handle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)m_currentUploadDataSize);
	
	curl_result = curl_easy_perform(curl_handle);
	if(m_uploadingFile)
		 fclose(m_uploadingFile);
	bool res = private_on_finish_request();
	return res;
}

bool NetworkManager::private_apply_method()
{
	curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST,NULL);
	curl_easy_setopt(curl_handle, CURLOPT_UPLOAD, 0L);
	if(m_method == "POST")
		curl_easy_setopt(curl_handle, CURLOPT_POST, 1L);
	else 	if(m_method == "GET")
		curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1L);
	else if (m_method == "PUT")
			curl_easy_setopt(curl_handle, CURLOPT_UPLOAD, 1L);
	else if (!m_method.empty())
	{
		curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST,m_method.c_str());
	}
	else 
	{
		curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST,NULL);
		return false;
	}
	return true;

}

void NetworkManager::setReferer(const NString &str)
{
	curl_easy_setopt(curl_handle, CURLOPT_REFERER, str.c_str());
}

int NetworkManager::getCurlResult()
{
	return curl_result;
}
CURL* NetworkManager::getCurlHandle()
{
	return curl_handle;
}

void NetworkManager::setOutputFile(const NString &str)
{
	m_OutFileName = str;
}

void NetworkManager::setUploadBufferSize(const int size)
{
	m_UploadBufferSize = size;
}

void NetworkManager::setChunkOffset(double offset)
{
	chunkOffset_ = offset;
}

void NetworkManager::setChunkSize(double size)
{
	chunkSize_ = size;
}

const NString  NetworkManager::getCurlResultString()
{
	const char * str = curl_easy_strerror(curl_result);
	std::string res = str;
	res+="";
	//curl_free(str);
	return res;
}

void NetworkManager::Uninitialize()
{
	if(_curl_init)
	{
		curl_global_cleanup();	
	}
}


void NetworkManager::enableResponseCodeChecking(bool enable)
{
	enableResponseCodeChecking_ = enable;
}

void NetworkManager::setCurlOption(int option, const NString &value) {
	curl_easy_setopt(curl_handle, (CURLoption)option, value.c_str());
}

void NetworkManager::setCurlOptionInt(int option, long value) {
	curl_easy_setopt(curl_handle, (CURLoption)option, value);
}