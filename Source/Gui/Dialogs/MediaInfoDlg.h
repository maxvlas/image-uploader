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

#ifndef MEDIAINFODLG_H
#define MEDIAINFODLG_H

// MediaInfoDlg.h : Declaration of the CMediaInfoDlg
// 
// This dialog window shows technical information 
// about  video/audio file that user had selected


#pragma once
#include "maindlg.h"
#include "resource.h"       

// CMediaInfoDlg

class CMediaInfoDlg:		public CDialogImpl <CMediaInfoDlg>,
								public CDialogResize <CMediaInfoDlg>,
								public CThreadImpl <CMediaInfoDlg>
{
	public:
		CMediaInfoDlg();
		~CMediaInfoDlg();
		void ShowInfo(LPCTSTR FileName);

		enum { IDD = IDD_MEDIAINFODLG };
	protected:
		BEGIN_MSG_MAP(CMediaInfoDlg)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_HANDLER(IDOK, BN_CLICKED, OnClickedOK)
			COMMAND_HANDLER(IDCANCEL, BN_CLICKED, OnClickedCancel)
			COMMAND_HANDLER(IDC_COPYALL, BN_CLICKED, OnBnClickedCopyall)
			CHAIN_MSG_MAP(CDialogResize<CMediaInfoDlg>)
		END_MSG_MAP()

		BEGIN_DLGRESIZE_MAP(CMediaInfoDlg)
			DLGRESIZE_CONTROL(IDC_FILEINFOEDIT, DLSZ_SIZE_X|DLSZ_SIZE_Y)
			DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X|DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDC_COPYALL, DLSZ_MOVE_Y)
		END_DLGRESIZE_MAP()
		
		// Handler prototypes:
		//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
		
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnClickedOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnClickedUseIeCookies(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnBnClickedCopyall(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
		DWORD Run();

		CString m_FileName;
};




#endif // MEDIAINFODLG_H