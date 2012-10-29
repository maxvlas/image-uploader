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

#include "UploadSettingsPage.h"
#include <uxtheme.h>

#include "wizarddlg.h"
#include "Gui/GuiTools.h"
#include <Gui/Dialogs/ServerSelectorControl.h>
#include <Func/Common.h>
#include <Func/MyUtils.h>

// CUploadSettingsPage
CUploadSettingsPage::CUploadSettingsPage()
{
		
}

CUploadSettingsPage::~CUploadSettingsPage()
{
}

void CUploadSettingsPage::TranslateUI()
{
	TRC(IDOK, "OK");
	TRC(IDCANCEL, "������");
	TRC(IDC_CONNECTIONSETTINGS, "��������� �����������");
	TRC(IDC_USEPROXYSERVER, "������������ ������-������");
	TRC(IDC_ADDRESSLABEL, "�����:");
	TRC(IDC_PORTLABEL, "����:");
	TRC(IDC_SERVERTYPE, "��� �������:");
	TRC(IDC_NEEDSAUTH, "���������� �����������");
	TRC(IDC_LOGINLABEL, "�����:");
	TRC(IDC_PASSWORDLABEL, "������:");
	TRC(IDC_AUTOCOPYTOCLIPBOARD, "������������� ���������� ���������� � ����� ������");
	TRC(IDC_UPLOADERRORLABEL, "������ ��������");
	TRC(IDC_IGNOREERRORS, "���������� ���������� ���� � ������ ������");
	TRC(IDC_RETRIES1LABEL, "���-�� ������� �������� �����:");
	TRC(IDC_RETRIES2LABEL, "���-�� ������� ��� ����� ��������:");
	TRC(IDC_UPLOADBUFFERLABEL, "������ ������ ������:");
}
	
LRESULT CUploadSettingsPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	TabBackgroundFix(m_hWnd);
	TranslateUI();
	RECT serverSelectorRect = GuiTools::GetDialogItemRect( m_hWnd, IDC_IMAGESERVERSELECTORPLACE);
	
	imageServerSelector_ = new CServerSelectorControl();
	imageServerSelector_->Create(m_hWnd, serverSelectorRect);
	imageServerSelector_->setTitle(TR("������ ��� �������� �����������"));
	imageServerSelector_->ShowWindow( SW_SHOW );
	imageServerSelector_->SetWindowPos( 0, serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right-serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top , 0);
	imageServerSelector_->setServerProfile(Settings.imageServer);
	
	serverSelectorRect = GuiTools::GetDialogItemRect( m_hWnd, IDC_FILESERVERSELECTORPLACE);
	
	fileServerSelector_ = new CServerSelectorControl();
	fileServerSelector_->setServersMask(CServerSelectorControl::smFileServers);
	fileServerSelector_->Create(m_hWnd, serverSelectorRect);
	fileServerSelector_->ShowWindow( SW_SHOW );
	fileServerSelector_->SetWindowPos( 0, serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right-serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top , 0);
	ServerProfile fileServerProfile;
	fileServerSelector_->setServerProfile(Settings.fileServer);
	fileServerSelector_->setTitle(TR("������ ��� �������� ������ ����� ������"));

	serverSelectorRect = GuiTools::GetDialogItemRect( m_hWnd, IDC_TRAYSERVERSELECTOR);
	trayServerSelector_ = new CServerSelectorControl();
	trayServerSelector_->setShowDefaultServerItem(true);
	trayServerSelector_->Create(m_hWnd, serverSelectorRect);
	trayServerSelector_->ShowWindow( SW_SHOW );
	trayServerSelector_->SetWindowPos( 0, serverSelectorRect.left, serverSelectorRect.top, serverSelectorRect.right-serverSelectorRect.left, serverSelectorRect.bottom - serverSelectorRect.top , 0);

	trayServerSelector_->setServerProfile(Settings.fileServer);
	trayServerSelector_->setTitle(TR("������ ��� ������� �������� ����������"));

	BOOL temp;
	DoDataExchange(FALSE);
	SendDlgItemMessage(IDC_SERVERTYPECOMBO,CB_ADDSTRING,0,(WPARAM)_T("HTTP"));

	SendDlgItemMessage(IDC_SERVERTYPECOMBO,CB_ADDSTRING,0,(WPARAM)_T("SOCKS4"));
	SendDlgItemMessage(IDC_SERVERTYPECOMBO,CB_ADDSTRING,0,(WPARAM)_T("SOCKS4A"));
	SendDlgItemMessage(IDC_SERVERTYPECOMBO,CB_ADDSTRING,0,(WPARAM)_T("SOCKS5"));
	SendDlgItemMessage(IDC_SERVERTYPECOMBO,CB_ADDSTRING,0,(WPARAM)_T("SOCKS5(DNS)"));

	// ---- ������������� ��������� (����������) ----
	
	// ---- ���������� connection settings -----
	SetDlgItemText(IDC_ADDRESSEDIT, Settings.ConnectionSettings.ServerAddress);
	SendDlgItemMessage(IDC_NEEDSAUTH, BM_SETCHECK, (WPARAM) Settings.ConnectionSettings.NeedsAuth);
	SendDlgItemMessage(IDC_AUTOCOPYTOCLIPBOARD, BM_SETCHECK, (WPARAM) Settings.AutoCopyToClipboard);
	
		SendDlgItemMessage(IDC_USEPROXYSERVER, BM_SETCHECK, (WPARAM) Settings.ConnectionSettings.UseProxy);
	SetDlgItemText(IDC_PROXYLOGINEDIT, Settings.ConnectionSettings.ProxyUser);
	SetDlgItemText(IDC_PROXYPASSWORDEDIT, Settings.ConnectionSettings.ProxyPassword);
	SetDlgItemInt(IDC_UPLOADBUFFERSIZEEDIT,Settings.UploadBufferSize/1024);
	if(Settings.ConnectionSettings.ProxyPort) // ������ ���� ���� �� ����� ����
		SetDlgItemInt(IDC_PORTEDIT, Settings.ConnectionSettings.ProxyPort);


	SendDlgItemMessage(IDC_SERVERTYPECOMBO, CB_SETCURSEL, Settings.ConnectionSettings.ProxyType);
	SendDlgItemMessage(IDC_NEEDSAUTH, BM_SETCHECK, (WPARAM) Settings.ConnectionSettings.NeedsAuth);
	
	// ����������� ���������
	OnClickedUseProxy(BN_CLICKED, IDC_USEPROXYSERVER, 0, temp);

	return 1;  // Let the system set the focus
}


LRESULT CUploadSettingsPage::OnClickedUseProxy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& bHandled)
{
	bool Checked = SendDlgItemMessage(IDC_USEPROXYSERVER, BM_GETCHECK)!=0;
	GuiTools::EnableNextN(GetDlgItem(wID),Checked? 8: 11, Checked);

	if(Checked)
		OnClickedUseProxyAuth(BN_CLICKED, IDC_NEEDSAUTH, 0, bHandled);

	//::EnableWindow(GetDlgItem(IDC_ADDRESSEDIT), Checked);
	return 0;
}
	
LRESULT CUploadSettingsPage::OnClickedUseProxyAuth(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bool Checked = SendDlgItemMessage(wID, BM_GETCHECK)!=0;
	GuiTools::EnableNextN(GetDlgItem(wID), 4, Checked);
	return 0;
}

bool CUploadSettingsPage::Apply()
{
	DoDataExchange(TRUE);
	Settings.fileServer = fileServerSelector_->serverProfile();
	Settings.imageServer = imageServerSelector_->serverProfile();
	Settings.AutoCopyToClipboard = SendDlgItemMessage(IDC_AUTOCOPYTOCLIPBOARD, BM_GETCHECK)!=0;

	return true;
}