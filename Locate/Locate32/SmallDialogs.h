#if !defined(SMALLDIALOGS_H)
#define SMALLDIALOGS_H

#if _MSC_VER >= 1000
#pragma once
#endif

class CSelectColumnsDlg : public CDialog  
{
public:
	CSelectColumnsDlg();

	virtual BOOL OnInitDialog(HWND hwndFocus);
	virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
	virtual BOOL OnClose();
	virtual void OnDestroy();
	virtual BOOL OnNotify(int idCtrl,LPNMHDR pnmh);
	virtual void OnHelp(LPHELPINFO lphi);
		

	void OnOK();
	void OnCancel();

	BOOL ListNotifyHandler(NMLISTVIEW *pNm);
	void EnableItems();
	BOOL ItemUpOrDown(BOOL bUp);

	void OnReset();
	void OnResetActions();


	struct ColumnItem 
	{
		
		DetailType m_nType;
		int m_nCol;
		int m_nWidth;
		CStringW m_strName;

		enum Align {
			Left=LVCFMT_LEFT,
			Right=LVCFMT_RIGHT,
			Center=LVCFMT_CENTER
		} m_nAlign;

		ColumnItem(int nCol,DetailType nType,int nResourceID,
			int nWidth,Align nAlign,CSubAction** pActions);

		CSubAction** m_pActions;

	};

	void SaveActionFields(ColumnItem* pColumn);
	void SetActionFields(ColumnItem* pColumn);
	
public:	
	CIntArray m_aSelectedCols; // In order
	CIntArray m_aIDs;
	CIntArray m_aWidths;
	CArray<ColumnItem::Align> m_aAligns;
	CArray<CSubAction**> m_aActions;

private:
	CListCtrl* m_pList;
	CComboBox m_ActionCombo,m_WhenCombo,m_WhichFileCombo,m_VerbCombo;
};



#define CUSTOM_PRESET		0
#define GLOBAL_PRESET		1
#define LATEST_PRESET		2
#define SELECTED_PRESET		3

class CSelectDatabasesDlg : public CDialog  
{
private:
	class CSavePresetDlg: public ::CSavePresetDlg  
	{
	public:
		CSavePresetDlg(CSelectDatabasesDlg* pParent);
		virtual void OnOK();
		virtual BOOL OnInitDialog(HWND hwndFocus);


		CSelectDatabasesDlg* m_pParent;

	};

public:
	CSelectDatabasesDlg(const CArray<PDATABASE>& rOrigDatabases,CArray<PDATABASE>& rSelectedDatabases,BYTE bFlags,LPCSTR szRegKey);
	virtual ~CSelectDatabasesDlg();

	virtual BOOL OnInitDialog(HWND hwndFocus);
	virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
	virtual BOOL OnClose();
	virtual void OnDestroy();
	virtual BOOL OnNotify(int idCtrl,LPNMHDR pnmh);
	virtual void OnHelp(LPHELPINFO lphi);
		
	BOOL ListNotifyHandler(NMLISTVIEW *pNm);
    
	void OnOK();
	void OnThreads();
	void OnPresetCombo();
	void OnDeletePreset();

	
	BOOL InsertDatabases();
	BOOL InsertDatabases(WORD wCount,WORD wThreads,const WORD* pwDatabaseIDs,const WORD* pwThreads,
		WORD wSelectedCount,const WORD* pwSelectedIds);
	BOOL InsertSelected();
	
	void LoadPresets();
	BOOL LoadPreset(LPCWSTR szName);
	BOOL SavePreset(LPCWSTR szName,BOOL bAskOverwrite=TRUE);

	static BOOL CheckRegistryIntegrity(CRegKey& RegKey);


	void EnableThreadGroups(int nThreadGroups);
	void RemoveThreadGroups();
	void ChangeNumberOfThreads(int nThreads);
	BOOL ItemUpOrDown(BOOL bUp);
	BOOL IncreaseThread(int nItem,CDatabase* pDatabase,BOOL bDecrease=FALSE);

	void EnableButtons();
	BOOL IsItemEnabled(const CDatabase* pDatabase);
	BOOL EnableItem(CDatabase* pDatabase,BOOL bEnable);

	void SelectDatabases(LPCWSTR pNames); // pNames is double zero terminated array of strins
		
	void SetThreadPriority(int nThreadPriority);
	int GetThreadPriority() const;

public:
	enum Flags {
		flagDisablePresets=0x1,
		flagShowThreads=0x2,
		flagSetUpdateState=0x4,
		flagReturnNotSelected=0x8,
		flagGlobalIsSelected=0x10,
		flagCustomIsSelected=0x20,
		flagLasestIsSelected=0x30,
		flagSelectedMask=0x30,
		flagEnablePriority=0x40
	};
	const CArray<PDATABASE>& m_rOrigDatabases;
	CArray<PDATABASE>& m_rSelectedDatabases;
	LPSTR m_pRegKey;
	BYTE m_bFlags;
	LPWSTR m_pSelectDatabases;
    
	int m_nThreadsCurrently;
    CListCtrl m_List;
	CComboBox m_PresetCombo;
	
	CString m_Buffer;
	CStringW m_BufferW;

	int m_nThreadPriority;

};

class CChangeCaseDlg: public CDialog  
{
public:
	CChangeCaseDlg();

	virtual BOOL OnInitDialog(HWND hwndFocus);
	virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
	virtual BOOL OnClose();
	virtual void OnDestroy();


	enum {
		Sentence,
		Lowercase,
		Uppercase,
		Title,
		Toggle
	} nSelectedCase;
	BOOL bForExtension;
};

class CChangeFilenameDlg: public CDialog  
{
public:
	CChangeFilenameDlg();

	virtual BOOL OnInitDialog(HWND hwndFocus);
	virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
	virtual BOOL OnClose();
	virtual void OnDestroy();

	CStringW m_sParent;
	CStringW m_sFileName;
	enum Flags {
		fNoExtension = 0x0001
	};
	DWORD m_dwFlags;
};

typedef BOOL (WINAPI* GETVOLUMEPATHNAMEW)(LPCWSTR,LPWSTR,DWORD);
typedef BOOL (WINAPI* GETDISKFREESPACEEX)(LPCSTR,PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);


class CPropertiesSheet : public CPropertySheet  
{
public:
	CPropertiesSheet(int nItems,CLocatedItem** pItems);
	virtual ~CPropertiesSheet();

	class CPropertiesPage : public CPropertyPage 
	{
	public:
		CPropertiesPage(int nItems,CLocatedItem** pItems);
		virtual ~CPropertiesPage();

		virtual BOOL OnInitDialog(HWND hwndFocus);
		virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
		virtual BOOL OnApply();
		virtual void OnCancel();
		virtual void OnTimer(DWORD wTimerID); 
		virtual void OnDestroy();
				
		void UpdateFields();
		void CheckFiles();
		void CheckDirectory(LPCWSTR szDirectory,BOOL bIsFirst);
		void CheckFile(LPCWSTR szFile,BOOL bIsFirst);
		
		static void FormatSizes(ULONGLONG ulSize,WCHAR* szBestFit,WCHAR* szBytes);
		static DWORD WINAPI CountingThreadProc(LPVOID lpParameter);
		static BOOL WINAPI GetVolumePathNameAlt(LPCWSTR lpszFileName,LPWSTR lpszVolumePathName,DWORD cchBufferLength);
		
		
	private:
		HANDLE m_hThread;
		int m_nItems;
		LPWSTR* m_ppFiles;
		BOOL* m_pbIsDirectory;

		int m_nFiles;
		int m_nDirectories;
		ULONGLONG m_nSize;
		ULONGLONG m_nSizeOnDisk;
		BOOL m_bIsSameType;

		int m_bReadOnly;
		int m_bHidden;
		int m_bArchive;

		int m_nExPos0;

		GETVOLUMEPATHNAMEW m_pGetVolumePathName;
		
	};

	CPropertyPage* m_pPropertiesPage;
	

	static DWORD WINAPI StartThreadProc(LPVOID lpParameter);
	void Open();
	
	HANDLE m_hThread;



};

#include "SmallDialogs.inl"

#endif