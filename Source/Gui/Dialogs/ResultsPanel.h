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

#pragma once

#include "atlheaders.h"
#include "resource.h"       // main symbols
#include "Func/common.h"
#include "Core/Upload/UploadEngine.h"
#include "Gui/WizardCommon.h"
#include "3rdpart/thread.h"
#include "Func/Settings.h"

#define IDC_OPTIONSMENU 10002
#define IDC_USEDIRECTLINKS 10003
#define IDC_SHORTENURLITEM 10004
#define IDC_PREVIEWBUTTON 1006
#define IDC_OPENLINKSINNEWTAB 1007

#define IDC_COPYFOLDERURL 10040


#define IDC_RESULTSTOOLBAR 5000
class CResultsPanel;

// CResultsPanel
class CWizardDlg;
struct IU_Result_Template
{
	CString Name,Items,LineSep,LineStart,ItemSep,LineEnd,TemplateText;
};
class CWebViewWindow;

class CResultsPanel : 
	public CDialogImpl<CResultsPanel>	
{
	public:
		CResultsPanel(CWizardDlg *dlg, CAtlArray<CUrlListItem>  & urlList);
		virtual ~CResultsPanel();
		enum { IDD = IDD_RESULTSPANEL};

		BEGIN_MSG_MAP(CResultsPanel)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_HANDLER(IDC_THUMBSPERLINE,EN_CHANGE,OnEditChanged)
			COMMAND_HANDLER(IDC_CODETYPE, CBN_SELCHANGE, OnCbnSelchangeCodetype)
			COMMAND_HANDLER(IDC_COPYALL, BN_CLICKED, OnBnClickedCopyall)
			COMMAND_HANDLER(IDC_USEDIRECTLINKS, BN_CLICKED, OnUseDirectLinksClicked)
			COMMAND_HANDLER(IDC_USETEMPLATE, BN_CLICKED, OnUseTemplateClicked)
			COMMAND_HANDLER(IDC_SHORTENURLITEM, BN_CLICKED, OnShortenUrlClicked)
			COMMAND_HANDLER(IDC_PREVIEWBUTTON, BN_CLICKED, OnPreviewButtonClicked)
			
			//COMMAND_HANDLER(, BN_CLICKED, OnCopyFolderUrlClicked)
			COMMAND_RANGE_HANDLER(IDC_COPYFOLDERURL, IDC_COPYFOLDERURL + 1000, OnCopyFolderUrlClicked);
			COMMAND_HANDLER(IDC_MEDIAFILEINFO, BN_CLICKED, OnBnClickedMediaInfo)
			COMMAND_HANDLER(IDC_VIEWLOG, BN_CLICKED, OnBnClickedViewLog)
			NOTIFY_HANDLER(IDC_RESULTSTOOLBAR, TBN_DROPDOWN, OnOptionsDropDown);
		NOTIFY_HANDLER_EX(IDC_RESULTSTOOLBAR, NM_CUSTOMDRAW, OnResulttoolbarNMCustomDraw)
		END_MSG_MAP()

	fastdelegate::FastDelegate1<bool> OnShortenUrlChanged;

    // Handler prototypes:
    //  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    //  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnOptionsDropDown(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT OnUseTemplateClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnUseDirectLinksClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCopyFolderUrlClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPreviewButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	 
	
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	CToolBarCtrl Toolbar;
	void SetPage(int Index);
	void setEngineList(CMyEngineList* EngineList);
	CAtlArray<CUrlListItem>  &UrlList;
	const CString GenerateOutput();
	CWebViewWindow* webViewWindow_;
	
	bool LoadTemplate();
	LPTSTR TemplateHead,TemplateFoot; //TemplateFoot is only pointer to part of TemplateHead 
	int m_Page;
	LRESULT OnCbnSelchangeCodetype(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedCopyall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedMediaInfo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBnClickedViewLog(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShortenUrlClicked(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	
	int GetCodeType();
	void UpdateOutput();
	void SetCodeType(int Index);
	void Clear();
	void EnableMediaInfo(bool Enable);
	CWizardDlg *WizardDlg;
	CMyEngineList *m_EngineList;
	CAtlArray<IU_Result_Template> Templates;
	bool LoadTemplates(CString &Error);
	std::map<CString, CString> m_Vars;
	std::vector<ServerProfile> m_Servers;
	CString ReplaceVars(const CString& Text);
	CAutoCriticalSection UrlListCS;
	int m_nImgServer, m_nFileServer;
	void AddServer(ServerProfile server);
	RECT rectNeeded;
	bool shortenUrl_;
	void InitUpload();
	void setUrlList(CAtlArray<CUrlListItem>  * urlList);
	LRESULT OnResulttoolbarNMCustomDraw(LPNMHDR pnmh);
	void BBCode_Link(CString &Buffer, CUrlListItem &item);
	void HTML_Link(CString &Buffer, CUrlListItem &item);
};


