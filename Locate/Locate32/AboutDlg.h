#if !defined(ABOUTDLG_H)
#define ABOUTDLG_H

#if _MSC_VER >= 1000
#pragma once
#endif


class CAboutDlg : public CDialog  
{
public:
	CAboutDlg();
	virtual BOOL OnInitDialog(HWND hwndFocus);
	virtual BOOL OnCommand(WORD wID, WORD wNotifyCode, HWND hControl);
	virtual void OnDrawItem(UINT idCtl,LPDRAWITEMSTRUCT lpdis);
	virtual BOOL WindowProc(UINT msg,WPARAM wParam,LPARAM lParam);
};

inline CAboutDlg::CAboutDlg()
:	CDialog(IDD_ABOUT)
{
}


#endif
