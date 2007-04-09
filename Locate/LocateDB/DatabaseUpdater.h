/* Copyright (c) 1997-2007 Janne Huttunen
   database updater v3.0.7.3250                 */

#if !defined(DATABASEUPDATER_H)
#define DATABASEUPDATER_H

#if _MSC_VER >= 1000
#pragma once
#endif



class CLocater;
class CDatabaseUpdater;

typedef BOOL (CALLBACK* UPDATEPROC)(DWORD_PTR dwParam,CallingReason crReason,UpdateError ueCode,CDatabaseUpdater* pUpdater);

#define BFSIZE 200000

typedef	HANDLE HFIND;
typedef WIN32_FIND_DATAA FIND_DATA;
typedef WIN32_FIND_DATAW FIND_DATAW;

#define VALID_HFIND(h)		((h)!=INVALID_HANDLE_VALUE)



// Maybe usefull
#define IS_UPDATER_EXITED(ptr)		((((ULONG_PTR)(ptr))&0xFFFF0000)==0xFFFF0000?TRUE:FALSE)
#define GET_UPDATER_CODE(ptr)		(IS_UPDATER_EXITED(ptr)?(UpdateError)(~((ULONG_PTR)(ptr))):ueStillWorking)
#define UPDATER_EXITED(code)		((CDatabaseUpdater*)(~((ULONG_PTR)(code))))

// String copyers
#define sMemCopy(dst,src,len)	CopyMemory(dst,src,len)
#define sMemZero(dst,len)		ZeroMemory(dst,len)
#define sMemSet(dst,val,len)	FillMemory(dst,len,val)
#define sstrlen(str,len)		dstrlen(str,len)

#define sstrlenW				dwstrlen





class CDatabaseUpdater
{
public:
	class DBArchive;

	class CRootDirectory
	{
	private:
		CRootDirectory(LPCWSTR szPath,LPCWSTR szPathInDb,int iPathInDbLen);
		CRootDirectory(LPCWSTR szPath,int iPathLen,LPCWSTR szPathInDb,int iPathInDbLen);
		
		~CRootDirectory();
	
		UpdateError ScanRoot(volatile LONG& lForceQuit);
		UpdateError ScanFolder(LPSTR szFolder,DWORD nFolderNameLength,volatile LONG& lForceQuit);
		UpdateError Write(CFile* dbFile);

		UpdateError ScanRootW(volatile LONG& lForceQuit);
		UpdateError ScanFolder(LPWSTR szFolder,DWORD nFolderNameLength,volatile LONG& lForceQuit);
		UpdateError WriteW(CFile* dbFile);

		
		
	protected:
		HFIND _FindFirstFile(LPCSTR szFolder,FIND_DATA* fd);
		HFIND _FindFirstFile(LPCWSTR szFolder,FIND_DATAW * fd);
		static BOOL _FindNextFile(HFIND hFind,FIND_DATA* fd);
		static BOOL _FindNextFile(HFIND hFind,FIND_DATAW* fd);
		void _FindClose(HFIND hFind);
		static BOOL _FindIsFolder(FIND_DATA* fd);
		static BOOL _FindIsFolder(FIND_DATAW* fd);
		static LPCSTR _FindGetName(FIND_DATA* fd);
		static LPCWSTR _FindGetName(FIND_DATAW* fd);
		static BYTE _FindGetAttribFlag(FIND_DATA* fd);
		static BYTE _FindGetAttribFlag(FIND_DATAW* fd);
		static void _FindGetLastWriteDosDateTime(FIND_DATA* fd,WORD* pwDate,WORD* pwTime);
		static void _FindGetCreationDosDate(FIND_DATA* fd,WORD* pwDate);
		static void _FindGetLastAccessDosDate(FIND_DATA* fd,WORD* pwDate);
		static void _FindGetLastWriteDosDateTime(FIND_DATAW* fd,WORD* pwDate,WORD* pwTime);
		static void _FindGetCreationDosDate(FIND_DATAW* fd,WORD* pwDate,WORD* pwTime);
		static void _FindGetLastAccessDosDate(FIND_DATAW* fd,WORD* pwDate,WORD* pwTime);
		static DWORD _FindGetFileSizeLo(FIND_DATA* fd);
		static DWORD _FindGetFileSizeLo(FIND_DATAW* fd);
		static DWORD _FindGetFileSizeHi(FIND_DATA* fd);
		static DWORD _FindGetFileSizeHi(FIND_DATAW* fd);

		CList<HFIND> m_aOpenHandles;

	public:
		CStringW m_Path,m_PathInDatabase;
		DWORD m_dwFiles;
		DWORD m_dwDirectories;
	
	private:
		class CBuffer
		{
		public:
			CBuffer() { pNext=NULL; pData=new BYTE[BFSIZE]; }
			~CBuffer() { delete[] pData; }

		public:
			operator BYTE*() {return pData; }
			
			BYTE* pData;
			DWORD nLength;
			CBuffer* pNext;
		};
		
		CBuffer *m_pFirstBuffer,*pCurrentBuffer;
		BYTE* pPoint;
		

		// Excluded directories
		CArrayFAP<LPWSTR> m_aExcludedDirectories; 
		
		union
		{
			LPSTR* m_aExcludeFilesPatternsA; 
			LPWSTR* m_aExcludeFilesPatternsW; 
		};

		friend CDatabaseUpdater;
		friend DBArchive;
	
	public:
		CRootDirectory* m_pNext;

	};

	class DBArchive {
	public:
		DBArchive();
		DBArchive(LPCWSTR szArchiveName,CDatabase::ArchiveType nArchiveType,
			LPCWSTR szAuthor,LPCWSTR szComment,LPCWSTR* pszRoots,DWORD nNumberOfRoots,BYTE nFlags,
			LPCWSTR szExcludedFiles,LPCWSTR* ppExcludedDirectories,int nExcludedDirectories,
			LPCWSTR szMaps);
		DBArchive(const CDatabase* pDatabase);
		~DBArchive();

	public:
		enum DBFlags {
			StopIfUnuavailable = 0x1, // Stops updating if root is unavailable
			IncrementalUpdate = 0x2, // Incremental update
			Unicode = 0x4
		};

		BOOL IsFlagged(DBFlags flag);
		void SetFlag(DBFlags flag,BOOL bSet=TRUE);
		void ParseExcludedFilesAndDirectories(LPCWSTR szExcludedFiles,
			const LPCWSTR* ppExcludedDirectories,int nExcludedDirectories);
		void CreateRootDirectories(CRootDirectory*& pCurrent,LPCWSTR pRoot,DWORD dwLength,LPCWSTR szMaps);		

	protected:

		LPWSTR m_szArchive;
		CDatabase::ArchiveType m_nArchiveType;
		
		LPWSTR m_szName;
		DWORD m_dwNameLength;

		CStringW m_sAuthor;
		CStringW m_sComment;
		CRootDirectory* m_pFirstRoot;

		BYTE m_nFlags;

		friend CDatabaseUpdater;

		LPWSTR m_szExtra1;
		LPWSTR m_szExtra2;
		
		// For progress estimation
		DWORD m_dwExpectedDirectories;
		DWORD m_dwExpectedFiles;

		union
		{
			LPSTR* m_aExcludeFilesPatternsA; 
			LPWSTR* m_aExcludeFilesPatternsW; 
		};
	};

protected:
	// Thread procs
#ifdef WIN32
	static DWORD WINAPI UpdateThreadProc(LPVOID lpParameter);
#endif
	UpdateError UpdatingProc();

public:
	CDatabaseUpdater(LPCWSTR szDatabaseFile,LPCWSTR szAuthor,LPCWSTR szComment,
		LPCWSTR* pszRoots,DWORD nNumberOfRoots,
		UPDATEPROC pProc,DWORD_PTR dwParam=0);	
					// Scan strings of pszRoots

	CDatabaseUpdater(const PDATABASE* ppDatabases,
		int nDatabases,UPDATEPROC pProc,DWORD_PTR dwParam=0);
	
	// Takes databases with thread ID wThread
	CDatabaseUpdater(const PDATABASE* ppDatabases,
		int nDatabases,UPDATEPROC pProc,WORD wThread,DWORD_PTR dwParam=0);

	~CDatabaseUpdater();
	
	void SetExtras(LPCSTR szExtra1,LPCSTR szExtra2);

#ifdef WIN32
	UpdateError Update(BOOL bThreaded=FALSE,int nThreadPriority=THREAD_PRIORITY_NORMAL);

	BOOL StopUpdating(BOOL bForce=TRUE);
	void CouldStop();

#else
	UpdateError Update();
#endif

	const CRootDirectory* GetCurrentRoot() const;
	LPCWSTR GetCurrentRootPath() const;
	LPWSTR GetCurrentRootPathStr() const;

	BOOL EnumDatabases(int iDatabase,LPWSTR& szName,LPWSTR& szFile,CDatabase::ArchiveType& nArchiveType,CRootDirectory*& pFirstRoot);
	
	const LPCWSTR GetCurrentDatabaseName() const;
	const LPCWSTR GetCurrentDatabaseFile() const;
	LPWSTR GetCurrentDatabaseNameStr() const;
	
	DWORD GetNumberOfDatabases() const;
	DWORD GetCurrentDatabase() const;
	WORD GetProgressStatus() const; // Estimated progress status between 0-1000

	BOOL IsIncrementUpdate() const;

#ifdef WIN32
	static BYTE GetAttribFlag(DWORD dwAttribs);
#endif


private:

	static CFile* OpenDatabaseFileForIncrementalUpdate(LPCWSTR szArchive,DWORD dwFiles,DWORD dwDirectories,BOOL bUnicode);

	CArrayFP<DBArchive*> m_aDatabases;
	DWORD m_dwCurrentDatabase;

	DWORD m_dwFiles;
	DWORD m_dwDirectories;
	

	

private:
	CFile* dbFile;
	UPDATEPROC m_pProc;
	DWORD_PTR m_dwData;
#ifdef WIN32
	HANDLE m_hThread;
	volatile LONG m_lForceQuit;
#endif

	
	CRootDirectory* m_pCurrentRoot;

	friend CRootDirectory;

public:
	// Status flags (statusNotChanged is not by CDatabaseUpdater)
	enum UpdateStatus {
		statusNotChanged,
		statusInitializing,
		statusScanning,
		statusWritingDB,
		statusFinishing
	};	
	UpdateStatus GetStatus() const { return sStatus; }
private:
	UpdateStatus sStatus;
};

typedef CDatabaseUpdater* PDATABASEUPDATER;





inline HFIND CDatabaseUpdater::CRootDirectory::_FindFirstFile(LPCSTR szFolder,FIND_DATA* fd)
{
	HFIND hFind=FindFirstFileA(szFolder,fd);
	if (hFind!=INVALID_HANDLE_VALUE)
	{
		m_aOpenHandles.AddHead(hFind);
		DebugOpenHandle(dhtFileFind,hFind,szFolder);
	}
	return hFind;
}

inline HFIND CDatabaseUpdater::CRootDirectory::_FindFirstFile(LPCWSTR szFolder,FIND_DATAW * fd)
{
	HFIND hFind=FindFirstFileW(szFolder,fd);
	if (hFind!=INVALID_HANDLE_VALUE)
	{
		m_aOpenHandles.AddHead(hFind);
		DebugOpenHandle(dhtFileFind,hFind,szFolder);
	}
	return hFind;
}
inline BOOL CDatabaseUpdater::CRootDirectory::_FindNextFile(HFIND hFind,FIND_DATA* fd)
{
	return FindNextFileA(hFind,fd);
}
inline BOOL CDatabaseUpdater::CRootDirectory::_FindNextFile(HFIND hFind,FIND_DATAW* fd)
{
	return FindNextFileW(hFind,fd);
}
inline void CDatabaseUpdater::CRootDirectory::_FindClose(HFIND hFind)
{
	ASSERT(hFind==m_aOpenHandles.GetHead());
	m_aOpenHandles.RemoveHead();
	FindClose(hFind);
	DebugCloseHandle(dhtFileFind,hFind,STRNULL);
}
inline BOOL CDatabaseUpdater::CRootDirectory::_FindIsFolder(FIND_DATA* fd)
{
	return (fd->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY;
}
inline BOOL CDatabaseUpdater::CRootDirectory::_FindIsFolder(FIND_DATAW* fd)
{
	return (fd->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY;
}
inline LPCSTR CDatabaseUpdater::CRootDirectory::_FindGetName(FIND_DATA* fd)
{
	return fd->cFileName;
}
inline LPCWSTR CDatabaseUpdater::CRootDirectory::_FindGetName(FIND_DATAW* fd)
{
	return fd->cFileName;
}

inline BYTE CDatabaseUpdater::CRootDirectory::_FindGetAttribFlag(FIND_DATA* fd)
{
	return CDatabaseUpdater::GetAttribFlag(fd->dwFileAttributes);
}
inline BYTE CDatabaseUpdater::CRootDirectory::_FindGetAttribFlag(FIND_DATAW* fd)
{
	return CDatabaseUpdater::GetAttribFlag(fd->dwFileAttributes);
}


inline void CDatabaseUpdater::CRootDirectory::_FindGetLastWriteDosDateTime(FIND_DATA* fd,WORD* pwDate,WORD* pwTime)
{
	FILETIME ft;
	FileTimeToLocalFileTime(&fd->ftLastWriteTime,&ft);
	FileTimeToDosDateTime(&ft,pwDate,pwTime);
}
inline void CDatabaseUpdater::CRootDirectory::_FindGetCreationDosDate(FIND_DATA* fd,WORD* pwDate)
{
	FILETIME ft;
	WORD wTemp;
	FileTimeToLocalFileTime(&fd->ftCreationTime,&ft);
	FileTimeToDosDateTime(&ft,pwDate,&wTemp);
}
inline void CDatabaseUpdater::CRootDirectory::_FindGetLastAccessDosDate(FIND_DATA* fd,WORD* pwDate)
{
	FILETIME ft;
	WORD wTemp;
	FileTimeToLocalFileTime(&fd->ftLastAccessTime,&ft);
	FileTimeToDosDateTime(&ft,pwDate,&wTemp);
}


inline void CDatabaseUpdater::CRootDirectory::_FindGetLastWriteDosDateTime(FIND_DATAW* fd,WORD* pwDate,WORD* pwTime)
{
	FILETIME ft;
	FileTimeToLocalFileTime(&fd->ftLastWriteTime,&ft);
	FileTimeToDosDateTime(&ft,pwDate,pwTime);
}
inline void CDatabaseUpdater::CRootDirectory::_FindGetCreationDosDate(FIND_DATAW* fd,WORD* pwDate,WORD* pwTime)
{
	FILETIME ft;
	FileTimeToLocalFileTime(&fd->ftCreationTime,&ft);
	FileTimeToDosDateTime(&ft,pwDate,pwTime);
}
inline void CDatabaseUpdater::CRootDirectory::_FindGetLastAccessDosDate(FIND_DATAW* fd,WORD* pwDate,WORD* pwTime)
{
	FILETIME ft;
	FileTimeToLocalFileTime(&fd->ftLastAccessTime,&ft);
	FileTimeToDosDateTime(&ft,pwDate,pwTime);
}

inline DWORD CDatabaseUpdater::CRootDirectory::_FindGetFileSizeLo(FIND_DATA* fd)
{
	return fd->nFileSizeLow;
}
inline DWORD CDatabaseUpdater::CRootDirectory::_FindGetFileSizeLo(FIND_DATAW* fd)
{
	return fd->nFileSizeLow;
}

inline DWORD CDatabaseUpdater::CRootDirectory::_FindGetFileSizeHi(FIND_DATA* fd)
{
	return fd->nFileSizeHigh;
}
inline DWORD CDatabaseUpdater::CRootDirectory::_FindGetFileSizeHi(FIND_DATAW* fd)
{
	return fd->nFileSizeHigh;
}

inline CDatabaseUpdater::CRootDirectory::CRootDirectory(LPCWSTR szPath,
														LPCWSTR szPathInDb,int iPathInDbLen)
:	m_Path(szPath),m_dwFiles(0),m_dwDirectories(0),
	m_pFirstBuffer(NULL),m_aExcludeFilesPatternsA(NULL)
{
	if (szPathInDb!=NULL)
		m_PathInDatabase.Copy(szPathInDb,iPathInDbLen);
	else
		m_PathInDatabase.Copy(m_Path);
}

inline CDatabaseUpdater::CRootDirectory::CRootDirectory(LPCWSTR szPath,
														int iLength,LPCWSTR szPathInDb,int iPathInDbLen)
:	m_Path(szPath,iLength),m_dwFiles(0),m_dwDirectories(0),
	m_pFirstBuffer(NULL),m_aExcludeFilesPatternsA(NULL)
{
	if (szPathInDb!=NULL)
		m_PathInDatabase.Copy(szPathInDb,iPathInDbLen);
	else
		m_PathInDatabase.Copy(m_Path);
}

inline CDatabaseUpdater::DBArchive::DBArchive()
:	m_szArchive(NULL),m_nArchiveType(CDatabase::archiveFile),m_pFirstRoot(NULL),m_nFlags(0),
	m_dwExpectedDirectories(DWORD(-1)),m_dwExpectedFiles(DWORD(-1)),
	m_szExtra1(NULL),m_szExtra2(NULL),m_aExcludeFilesPatternsA(NULL)
{
}

inline BOOL CDatabaseUpdater::DBArchive::IsFlagged(DBFlags flag)
{
	return m_nFlags&flag;
}

inline void CDatabaseUpdater::DBArchive::SetFlag(DBFlags flag,BOOL bSet)
{
	if (bSet)
		m_nFlags|=flag;
	else
		m_nFlags&=~BYTE(flag);
}


#ifdef WIN32
inline void CDatabaseUpdater::CouldStop()
{
	InterlockedExchange(&m_lForceQuit,TRUE);
}

inline BYTE CDatabaseUpdater::GetAttribFlag(DWORD dwAttribs)
{
	BYTE bRet=0;
	if (dwAttribs&FILE_ATTRIBUTE_ARCHIVE)
		bRet|=UDBATTRIB_ARCHIVE;
	if (dwAttribs&FILE_ATTRIBUTE_HIDDEN)
		bRet|=UDBATTRIB_HIDDEN;
	if (dwAttribs&FILE_ATTRIBUTE_READONLY)
		bRet|=UDBATTRIB_READONLY;
	if (dwAttribs&FILE_ATTRIBUTE_SYSTEM)
		bRet|=UDBATTRIB_SYSTEM;
	return bRet;
}

#endif

inline DWORD CDatabaseUpdater::GetNumberOfDatabases() const
{
	return m_aDatabases.GetSize();
}

inline DWORD CDatabaseUpdater::GetCurrentDatabase() const
{
	return m_dwCurrentDatabase;
}

inline const CDatabaseUpdater::CRootDirectory* CDatabaseUpdater::GetCurrentRoot() const
{
	return m_pCurrentRoot;
}


inline WORD CDatabaseUpdater::GetProgressStatus() const
{
	if (m_dwCurrentDatabase==DWORD(-1))
		return WORD(-1);
	
	
	if (m_aDatabases[m_dwCurrentDatabase]->m_dwExpectedDirectories==DWORD(-1))
		return WORD(-1);

	DWORD dwDirectoriesCurrently=m_dwDirectories;
	if (m_pCurrentRoot!=NULL)
		dwDirectoriesCurrently+=m_pCurrentRoot->m_dwDirectories;

	if (dwDirectoriesCurrently>m_aDatabases[m_dwCurrentDatabase]->m_dwExpectedDirectories)
		return 990;

	if (m_aDatabases[m_dwCurrentDatabase]->m_dwExpectedDirectories>0)
		return WORD((dwDirectoriesCurrently*1000)/m_aDatabases[m_dwCurrentDatabase]->m_dwExpectedDirectories);

	return WORD(-1);
}

inline LPCWSTR CDatabaseUpdater::GetCurrentRootPath() const
{
	if (m_pCurrentRoot==NULL)
		return NULL;
	return m_pCurrentRoot->m_Path;
}

inline LPWSTR CDatabaseUpdater::GetCurrentRootPathStr() const
{
	if (m_pCurrentRoot==NULL)
		return NULL;
	return alloccopy(m_pCurrentRoot->m_Path,m_pCurrentRoot->m_Path.GetLength());
}

inline const LPCWSTR CDatabaseUpdater::GetCurrentDatabaseName() const
{
	if (m_dwCurrentDatabase==DWORD(-1))
		return NULL;
	return m_aDatabases[m_dwCurrentDatabase]->m_szName;
}

inline const LPCWSTR CDatabaseUpdater::GetCurrentDatabaseFile() const
{
	if (m_dwCurrentDatabase==DWORD(-1))
		return NULL;
	return m_aDatabases[m_dwCurrentDatabase]->m_szArchive;
}

inline LPWSTR CDatabaseUpdater::GetCurrentDatabaseNameStr() const
{
	if (m_dwCurrentDatabase==DWORD(-1))
		return 0;
	
	LPWSTR szRet=new WCHAR[m_aDatabases[m_dwCurrentDatabase]->m_dwNameLength+1];
	ASSERT(istrlenw(m_aDatabases[m_dwCurrentDatabase]->m_szName)==m_aDatabases[m_dwCurrentDatabase]->m_dwNameLength);
	
	MemCopyW(szRet,m_aDatabases[m_dwCurrentDatabase]->m_szName,m_aDatabases[m_dwCurrentDatabase]->m_dwNameLength+1);
	return szRet;
}

inline BOOL CDatabaseUpdater::IsIncrementUpdate() const
{
	if (m_dwCurrentDatabase==DWORD(-1))
		return FALSE;
	
	return m_aDatabases[m_dwCurrentDatabase]->IsFlagged(DBArchive::IncrementalUpdate);
}

inline BOOL CDatabaseUpdater::EnumDatabases(int iDatabase,LPWSTR& szName,LPWSTR& szFile,CDatabase::ArchiveType& nArchiveType,CRootDirectory*& pFirstRoot)
{
	if (iDatabase<0 || iDatabase>=m_aDatabases.GetSize())
		return FALSE;

	szName=m_aDatabases[iDatabase]->m_szName;
	szFile=m_aDatabases[iDatabase]->m_szArchive;
	nArchiveType=m_aDatabases[iDatabase]->m_nArchiveType;
	pFirstRoot=m_aDatabases[iDatabase]->m_pFirstRoot;
	return TRUE;
}

#if defined(WIN32) && !defined(DBUPD_NOFORCELIBS)
	#pragma comment(lib, "Mpr.lib")
#endif

#endif