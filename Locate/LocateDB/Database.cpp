/* Copyright (c) 1997-2004 Janne Huttunen
   database updater v2.98.4.9200                  */

#include <HFCLib.h>
#include "Locatedb.h"

CDatabase::CDatabase(CDatabase& src)
{
	sMemCopy(this,&src,sizeof(CDatabase));
	// This cleares m_aExcludedDirectories to null to prevent invalid memory handling
    m_aExcludedDirectories.GiveBuffer();
	
	if (m_szName!=NULL)
		m_szName=alloccopy(m_szName);
	if (m_szCreator!=NULL)
		m_szCreator=alloccopy(m_szCreator);
	if (m_szDescription!=NULL)
		m_szDescription=alloccopy(m_szDescription);
	if (m_szArchiveName!=NULL)
		m_szArchiveName=alloccopy(m_szArchiveName);

	
	if (m_szRoots!=NULL)
	{
		for (DWORD iLength=0;m_szRoots[iLength]!='\0' || m_szRoots[iLength+1]!='\0';iLength++);
		iLength+=2;
        m_szRoots=new char[iLength];
		sMemCopy(m_szRoots,src.m_szRoots,iLength);
	}

	for (int i=0;i<src.m_aExcludedDirectories.GetSize();i++)
		m_aExcludedDirectories.Add(alloccopy(src.m_aExcludedDirectories[i]));
}

BOOL CDatabase::LoadFromRegistry(HKEY hKeyRoot,LPCSTR szPath,CArray<CDatabase*>& aDatabases)
{
	CRegKey RegKey;
	CString Path(szPath);
	Path<<'\\';
	
	
	if (RegKey.OpenKey(hKeyRoot,Path,CRegKey::defRead|CRegKey::samEnumerateSubkeys)!=ERROR_SUCCESS)
		return FALSE;

    CString key;
	for (int i=0;RegKey.EnumKey(i,key);i++)
	{
		CDatabase* pDatabase=CDatabase::FromKey(RegKey,"",key);
		if (pDatabase->m_wID==0)
			pDatabase->m_wID=GetUniqueIndex(aDatabases);

		if (pDatabase!=NULL)
			aDatabases.Add(pDatabase);
	}

	return TRUE;
}

CDatabase* CDatabase::FromName(HKEY hKeyRoot,LPCSTR szPath,LPCSTR szName,int iNameLength)
{
	CRegKey RegKey,RegKey2;
	CString Path(szPath);
	Path<<'\\';
	
	
	if (RegKey.OpenKey(hKeyRoot,Path,CRegKey::defRead|CRegKey::samEnumerateSubkeys)!=ERROR_SUCCESS)
		return NULL;

	if (iNameLength==-1)
		dstrlen(szName,iNameLength);

    CString key;
	for (int i=0;RegKey.EnumKey(i,key);i++)
	{
		if (RegKey2.OpenKey(RegKey,key,CRegKey::defRead)==ERROR_SUCCESS)
		{
			DWORD dwLength=RegKey2.QueryValueLength(NULL);
			if (dwLength>1)
			{
				CAllocArrayTmpl<CHAR> pName(dwLength);
				RegKey2.QueryValue(NULL,(LPSTR)pName,dwLength);
                
				if (strcasencmp(pName,szName,iNameLength)==0)
					return CDatabase::FromKey(RegKey,"",key);		
			}
			RegKey2.CloseKey();
		}
	}
	return NULL;
}

BOOL CDatabase::SaveToRegistry(HKEY hKeyRoot,LPCSTR szPath,const PDATABASE* ppDatabases,int nDatabases)
{
	CRegKey RegKey;
	CString Path(szPath);
	Path<<'\\';
	
	if (RegKey.OpenKey(hKeyRoot,Path,CRegKey::samAll|CRegKey::createNew)!=ERROR_SUCCESS)
		return FALSE;
	
	CString name;
	// First, delete all existing keys
	for (;RegKey.EnumKey(0,name);)
		RegKey.DeleteKey(name);

	
	// Retrieving digits;
	int nDigits=1;
	for (int i=10;i<=nDatabases;i*=10,nDigits++);
	char* pNum=new char[nDigits+1];
		

    for (int i=0;i<nDatabases;i++)
	{
		DWORD dwLength;
		char* pKeyName=ppDatabases[i]->GetValidKey(i+1);
		dstrlen(pKeyName,dwLength);
		
		// Copying name
		char* pKey=new char[nDigits+dwLength+2];

        sMemCopy(pKey+nDigits+1,pKeyName,dwLength+1);
		pKey[nDigits]='_';
        
        // Formating number
		itoa(i+1,pNum,10);
		ASSERT(strlen(pNum)<=size_t(nDigits));

		dstrlen(pNum,dwLength);
        sMemCopy(pKey+nDigits-dwLength,pNum,dwLength);
		sMemSet(pKey,'0',nDigits-dwLength);
        
		ppDatabases[i]->SaveToRegistry(hKeyRoot,szPath,pKey);
		delete[] pKey;
	}

	delete[] pNum;
	return TRUE;
}


BOOL CDatabase::SaveToRegistry(HKEY hKeyRoot,LPCSTR szPath,LPCSTR szKey)
{
	CRegKey RegKey;
	CString Path(szPath);
	Path<<'\\'<<szKey;
	
	if (RegKey.OpenKey(hKeyRoot,Path,CRegKey::defWrite)!=ERROR_SUCCESS)
		return FALSE;

	RegKey.SetValue(NULL,m_szName);
	RegKey.SetValue("Flags",DWORD(m_wFlags));
	RegKey.SetValue("Thread",DWORD(m_wThread));
	RegKey.SetValue("ID",DWORD(m_wID));

	RegKey.SetValue("ArchiveType",DWORD(m_ArchiveType));
	if (m_szArchiveName!=NULL)
		RegKey.SetValue("ArchiveName",m_szArchiveName);
	else
		RegKey.SetValue("ArchiveName",szEmpty);

	if (m_szCreator!=NULL)
		RegKey.SetValue("Creator",m_szCreator);
	else
		RegKey.SetValue("Creator",szEmpty);

	if (m_szDescription!=NULL)
		RegKey.SetValue("Description",m_szDescription);
	else
		RegKey.SetValue("Description",szEmpty);

	if (m_szRoots!=NULL)
	{
		for (int i=0;m_szRoots[i]!='\0' || m_szRoots[i+1]!='\0';i++);
		RegKey.SetValue("Roots",m_szRoots,i+2,REG_MULTI_SZ);
	}
	else
		RegKey.SetValue("Roots",szEmpty);

	// Excluded directories
	DWORD dwLength=1;
	for (int i=0;i<m_aExcludedDirectories.GetSize();i++)
		dwLength+=istrlen(m_aExcludedDirectories[i])+1;

	char* pString=new char[dwLength+1];
	char* pPtr=pString;
	for (int i=0;i<m_aExcludedDirectories.GetSize();i++)
	{
		if (i>0)
			*(pPtr++)=';';

		int iLength=istrlen(m_aExcludedDirectories[i]);
		CopyMemory(pPtr,m_aExcludedDirectories[i],iLength);
		pPtr+=iLength;
	}
	*pPtr='\0';
		
	RegKey.SetValue("Excluded Directories",pString);
    delete[] pString;
	return TRUE;
}

CDatabase* CDatabase::FromKey(HKEY hKeyRoot,LPCSTR szPath,LPCSTR szKey)
{
	CRegKey RegKey;
	CString Path(szPath);
	if (!Path.IsEmpty() && Path.LastChar()!='\\')
		Path<<'\\';
	Path << szKey;
	
	if (RegKey.OpenKey(hKeyRoot,Path,CRegKey::defRead)!=ERROR_SUCCESS)
		return NULL;

	DWORD dwLength=RegKey.QueryValueLength(NULL);
	if (int(dwLength)<=1)
		return NULL;

	CDatabase* pDatabase=new CDatabase;
	
	// Copying name
	pDatabase->m_szName=new char[dwLength];
	RegKey.QueryValue("",pDatabase->m_szName,dwLength);
		
	// Retrieving flags and archive type
	DWORD dwTemp=0;
	RegKey.QueryValue("ID",dwTemp);
	pDatabase->m_wID=LOWORD(dwTemp);
	RegKey.QueryValue("Flags",dwTemp);
	pDatabase->m_wFlags=LOWORD(dwTemp);
	RegKey.QueryValue("Thread",dwTemp);
	pDatabase->m_wThread=LOWORD(dwTemp);
	
	RegKey.QueryValue("ArchiveType",*((DWORD*)&pDatabase->m_ArchiveType));
	
	
	// Copying archive name info
	dwLength=RegKey.QueryValueLength("ArchiveName");
	if (dwLength>1)
	{
		pDatabase->m_szArchiveName=new char[dwLength];
		RegKey.QueryValue("ArchiveName",pDatabase->m_szArchiveName,dwLength);
	}

    // Copying creator info
	dwLength=RegKey.QueryValueLength("Creator");
	if (dwLength>1)
	{
		pDatabase->m_szCreator=new char[dwLength];
		RegKey.QueryValue("Creator",pDatabase->m_szCreator,dwLength);
	}

	// Copying description
	dwLength=RegKey.QueryValueLength("Description");
	if (dwLength>1)
	{
		pDatabase->m_szDescription=new char[dwLength];
		RegKey.QueryValue("Description",pDatabase->m_szDescription,dwLength);
	}

	// Copying roots
	dwLength=RegKey.QueryValueLength("Roots");
	if (dwLength>1)
	{
		pDatabase->m_szRoots=new char[dwLength];
		RegKey.QueryValue("Roots",pDatabase->m_szRoots,dwLength);
	}

	// Excluded directories
	dwLength=RegKey.QueryValueLength("Excluded Directories");
	if (dwLength>1)
	{
		LPSTR pString=new char[dwLength];
		RegKey.QueryValue("Excluded Directories",pString,dwLength);

		LPCSTR pPtr=pString;
		while (*pPtr!='\0')
		{
			int nIndex=FirstCharIndex(pPtr,';');
			if (nIndex!=-1)
			{
				pDatabase->m_aExcludedDirectories.Add(alloccopy(pPtr,nIndex));
                pPtr+=nIndex+1;
			}
			else
			{
				pDatabase->m_aExcludedDirectories.Add(alloccopy(pPtr));
				break;
			}
		}

		delete[] pString;
	}

	return pDatabase;
}

CDatabase* CDatabase::FromOldStyleDatabase(HKEY hKeyRoot,LPCSTR szPath)
{
	CRegKey RegKey;
	
	// Try to open key
	if (RegKey.OpenKey(hKeyRoot,szPath,CRegKey::defRead)!=ERROR_SUCCESS)
		return NULL;
	
	// Try to retrieve database file
	DWORD dwLength=RegKey.QueryValueLength("DatabaseFile");
	if (dwLength==0)
		return NULL;

    // Old style database info found, loading it
	CDatabase* pDatabase=new CDatabase;
	
	pDatabase->m_ArchiveType=CDatabase::archiveFile;
	pDatabase->m_szArchiveName=new char[dwLength];
	RegKey.QueryValue("DatabaseFile",pDatabase->m_szArchiveName,dwLength);

	
	// Setting name to "default"
	pDatabase->m_szName=new char[8];
	sMemCopy(pDatabase->m_szName,"default",8);
	
	// Copying creator info
	dwLength=RegKey.QueryValueLength("Default Creator");
	if (dwLength>1)
	{
		pDatabase->m_szCreator=new char[dwLength];
		RegKey.QueryValue("Default Creator",pDatabase->m_szCreator,dwLength);
	}
	
	// Copying description
	dwLength=RegKey.QueryValueLength("Default Description");
	if (dwLength>1)
	{
		pDatabase->m_szDescription=new char[dwLength];
		RegKey.QueryValue("Default Description",pDatabase->m_szDescription,dwLength);
	}

	// Copying roots
	dwLength=RegKey.QueryValueLength("Drives");
	if (dwLength>1)
	{
		pDatabase->m_szRoots=new char[dwLength];
		RegKey.QueryValue("Drives",pDatabase->m_szRoots,dwLength);
	}
	
	return pDatabase;
}

CDatabase* CDatabase::FromFile(LPCSTR szFileName,int dwNameLength)
{
	LPSTR szFile=GetCorrertFileName(szFileName,dwNameLength);
	if (szFile==NULL)
		return NULL;

    CDatabase* pDatabase=new CDatabase;

	
	
	pDatabase->m_szName=new char[6];
	sMemCopy(pDatabase->m_szName,"param",6);

	pDatabase->m_ArchiveType=archiveFile;
	pDatabase->m_szArchiveName=szFile;

	return pDatabase;
}

CDatabase* CDatabase::FromDefaults(BOOL bDefaultFileName,LPCSTR szAppDir,int iAppDirLength)
{
	CDatabase* pDatabase=new CDatabase; // This default dwFlags and description and drives to NULL

	pDatabase->m_szName=new char[8];
	sMemCopy(pDatabase->m_szName,"default",8);

	pDatabase->m_ArchiveType=CDatabase::archiveFile;
	
	if (bDefaultFileName)
	{
		if (iAppDirLength==-1)
			dstrlen(szAppDir,iAppDirLength);

		pDatabase->m_szArchiveName=new char[iAppDirLength+10];
		CopyMemory(pDatabase->m_szArchiveName,szAppDir,iAppDirLength);
		CopyMemory(pDatabase->m_szArchiveName+iAppDirLength,"files.dbs",10);

	}
	else
		pDatabase->m_szArchiveName=NULL;
	return pDatabase;
}

LPSTR CDatabase::GetCorrertFileName(LPCSTR szFileName,int dwNameLength)
{
	if (dwNameLength==-1)
		dstrlen(szFileName,dwNameLength);
	
	LPSTR szFile;
	if (szFileName[0]!='\\' && szFileName[1]!=':')
	{
		DWORD dwLength=GetCurrentDirectory(0,NULL);
        if (dwLength==0)
			return NULL;
		szFile=new char[dwLength+dwNameLength+2];
		GetCurrentDirectory(dwLength,szFile);
		dwLength--;
		if (szFile[dwLength-1]!='\\')
			szFile[dwLength++]='\\';
		sMemCopy(szFile+dwLength,szFileName,DWORD(dwNameLength));
		szFile[dwLength+dwNameLength]='\0';
	}
	else
	{
		szFile=new char[dwNameLength+1];
		sMemCopy(szFile,szFileName,DWORD(dwNameLength));
		szFile[dwNameLength]='\0';
	}

	// Checking whether file exists
	if (!CFile::IsValidFileName(szFile))
	{
		delete[] szFile;
		return NULL;
	}
	return szFile;
}

void CDatabase::CheckDoubleNames(PDATABASE* ppDatabases,int nDatabases)
{
	for (int i=1;i<nDatabases;i++)
	{
		DWORD dwLength;
		dstrlen(ppDatabases[i]->m_szName,dwLength);
		
		for (int j=0;j<i;j++)
		{
			if (strcasencmp(ppDatabases[i]->m_szName,ppDatabases[j]->m_szName,dwLength+1)==0)
			{
				if (ppDatabases[i]->m_szName[dwLength-1]>='0' && ppDatabases[i]->m_szName[dwLength-1]<'9')
					ppDatabases[i]->m_szName[dwLength-1]++;
				else if (ppDatabases[i]->m_szName[dwLength-1]=='9')
				{
					if (ppDatabases[i]->m_szName[dwLength-2]>='1' && ppDatabases[i]->m_szName[dwLength-2]<'9')
						ppDatabases[i]->m_szName[dwLength-2]++;
					else
					{
						char* tmp=new char[dwLength+2];
						sMemCopy(tmp,ppDatabases[i]->m_szName,dwLength-1);
						delete[] ppDatabases[i]->m_szName;
						tmp[dwLength-1]='1';
						tmp[dwLength]='0';
						tmp[dwLength+1]='\0';
						ppDatabases[i]->m_szName=tmp;
					}
				}
				else
				{
					char* tmp=new char[dwLength+2];
					sMemCopy(tmp,ppDatabases[i]->m_szName,dwLength);
					tmp[dwLength++]='1';
                    tmp[dwLength]='\0';
					delete[] ppDatabases[i]->m_szName;
                    ppDatabases[i]->m_szName=tmp;
				}
				
				dstrlen(ppDatabases[i]->m_szName,dwLength);
				j=-1;
			}			
		}
	}
}



void CDatabase::CheckValidNames(PDATABASE* ppDatabases,int nDatabases)
{
	for (int i=0;i<nDatabases;i++)
	{
		if (!IsNameValid(ppDatabases[i]->m_szName))
		{
			MakeNameValid(ppDatabases[i]->m_szName);
			
			// Unallocaling unnecessary memory
			DWORD dwLength;
			sstrlen(ppDatabases[i]->m_szName,dwLength);
			char* pNew=new char[++dwLength];
			sMemCopy(pNew,ppDatabases[i]->m_szName,dwLength);
			delete[] ppDatabases[i]->m_szName;
			ppDatabases[i]->m_szName=pNew;
		}
	}
}


#define ISVALIDFORNAME(a) \
	( (a)!='\\' && (a)!='\"' && (a)!='\'' )

#define ISVALIDFORKEY(a) \
	( ((a)>='0' && (a)<='9') || \
	  ((a)>='a' && (a)<='z') || \
	  ((a)>='A' && (a)<='A') || \
	  (a)==' ' || \
	  (a)=='#' || \
	  (a)=='-' || \
	  (a)=='_' )
	
BOOL CDatabase::IsNameValid(LPCSTR szName)
{
	for (int i=0;szName[i]!='\0';i++)
	{
		if (!ISVALIDFORNAME(szName[i]))
			return FALSE;
	}
	return i>0;
}

void CDatabase::MakeNameValid(LPSTR szName)
{
	int j=0;
	for (int i=0;szName[i]!='\0';i++)
	{
		if (ISVALIDFORNAME(szName[i]))
			szName[j++]=szName[i];
	}
	szName[j]='\0';
}

LPSTR CDatabase::GetValidKey(DWORD dwUniqueNum) const
{
	DWORD dwValid=0;
	for (int i=0;m_szName[i]!='\0';i++)
	{
		if (ISVALIDFORKEY(m_szName[i]))
			dwValid++;
	}

	if (dwValid==0)
	{
		if (dwUniqueNum>0)
		{
			char* key=new char[25];
			wsprintf(key,"db%d",dwUniqueNum);
			return key;            
		}
		else
			return alloccopy("db");
	}

	char* key=new char[dwValid+1];
	for (int i=0,j=0;m_szName[i]!='\0';i++)
	{
		if (ISVALIDFORKEY(m_szName[i]))
			key[j++]=m_szName[i];
	}
	key[j]='\0';
	return key;
}

WORD CDatabase::CheckIDs(PDATABASE* ppDatabases,int nDatabases)
{
	int nCurrentlyMustBe=0;
	
	for (int index=0;index<nDatabases;index++)
	{
		if (ppDatabases[index]->m_wID==0)
			ppDatabases[index]->m_wID=GetUniqueIndex(ppDatabases,nDatabases);
	}

	index=0;
	while (index<nDatabases)
	{
		int nCurrentThreadID=ppDatabases[index]->m_wThread;
		if (nCurrentThreadID!=nCurrentlyMustBe)
		{
			// Make databases with current ID to nCurrentlyMustBe
			ppDatabases[index++]->m_wThread=nCurrentlyMustBe;

            while (index<nDatabases && ppDatabases[index]->m_wThread==nCurrentThreadID)
				ppDatabases[index++]->m_wThread=nCurrentlyMustBe;
		}
		else
		{
			for (;index<nDatabases && ppDatabases[index]->m_wThread==nCurrentThreadID;index++);
		}
		nCurrentlyMustBe++;
	}
	return nCurrentlyMustBe;
}

void CDatabase::GetRoots(CArray<LPSTR>& aRoots) const
{
	if (m_szRoots!=NULL)
	{
		LPCSTR pPtr=m_szRoots;
		while (*pPtr!='\0')
		{
			DWORD dwLength;
			dstrlen(pPtr,dwLength);
			LPSTR pStr=new char[++dwLength];
			sMemCopy(pStr,pPtr,dwLength);
			aRoots.Add(pStr);
			pPtr+=dwLength;
		}
	}
}

void CDatabase::SetRoots(LPSTR* pRoots,int nCount)
{
	if (m_szRoots!=NULL)
		delete[] m_szRoots;

	if (nCount==0)
	{
		m_szRoots=NULL;
		return;
	}

	// Counting required buffer size
	DWORD dwBufferSize=0;
	DWORD* dwLengths=new DWORD[nCount];

	for (int i=0;i<nCount;i++)
	{
		dstrlen(pRoots[i],dwLengths[i]);
		dwBufferSize+=++dwLengths[i];
	}
	
	m_szRoots=new char[dwBufferSize+1];
	LPSTR pPtr=m_szRoots;
	for (i=0;i<nCount;i++)
	{
		sMemCopy(pPtr,pRoots[i],dwLengths[i]);
		pPtr+=dwLengths[i];
	}
	*pPtr='\0';

	delete[] dwLengths;
}

CDatabase* CDatabase::FindByName(PDATABASE* ppDatabases,int nDatabases,LPCSTR szName,int iLength)
{
	CString sName(szName,iLength);
	sName.MakeLower();

	for (int i=0;i<nDatabases;i++)
	{
		CString str(ppDatabases[i]->GetName());
		str.MakeLower();        

		if (sName.Compare(str)==0)
			return ppDatabases[i];
	}
	return NULL;
}

CDatabase* CDatabase::FindByFile(PDATABASE* ppDatabases,int nDatabases,LPCSTR szFile,int iLength)
{
	char* pPath1=NULL;
	char szPath1[MAX_PATH]; 

	if (iLength==0)
		return NULL;

	DWORD dwRet;

	if (szFile[0]!='\\' && szFile[1]!=':')
	{
		if (iLength==-1)
			dstrlen(szFile,iLength);
			
		DWORD dwLength=GetCurrentDirectory(0,NULL);
		if (dwLength==0)
			return NULL;
       
		pPath1=new char[dwLength+iLength+2];
		GetCurrentDirectory(dwLength,pPath1);
		dwLength--;
		if (pPath1[dwLength-1]!='\\')
			pPath1[dwLength++]='\\';

		sMemCopy(pPath1+dwLength,szFile,DWORD(iLength));
		pPath1[dwLength+iLength]='\0';

		dwRet=GetShortPathName(pPath1,szPath1,MAX_PATH);
	}
	else if (iLength!=-1)
	{
		pPath1=new char[iLength+1];
		sMemCopy(pPath1,szFile,DWORD(iLength));
		pPath1[iLength]='\0';
		dwRet=GetShortPathName(pPath1,szPath1,MAX_PATH);
	}
	else
		dwRet=GetShortPathName(szFile,szPath1,MAX_PATH);
	
	// File does not exists
	if (dwRet==0)
	{
		if (pPath1==NULL)
			pPath1=const_cast<LPSTR>(szFile);
		
		for (int i=0;i<nDatabases;i++)
		{
			if (!CFile::IsFile(ppDatabases[i]->m_szArchiveName))
			{
				if (strcasecmp(pPath1,ppDatabases[i]->m_szArchiveName)==0)
				{
					if (pPath1!=szFile)
						delete[] pPath1;
					return ppDatabases[i];
				}
			}
		}
		if (pPath1!=szFile)
			delete[] pPath1;
		return NULL;
	}

	if (pPath1!=NULL)
		delete[] pPath1;
	for (int i=0;i<nDatabases;i++)
	{
		char szPath2[MAX_PATH];
		dwRet=GetShortPathName(ppDatabases[i]->m_szArchiveName,szPath2,MAX_PATH);

		if (dwRet!=NULL)
		{
			if (strcasecmp(szPath1,szPath2)==0)
				return ppDatabases[i];
		}
	}
	return NULL;
}

WORD CDatabase::GetUniqueIndex(PDATABASE* ppDatabases,int nDatabases)
{
	WORD wNewIndex,i;

	do
	{
		wNewIndex=LOWORD(GetTickCount());

		if (wNewIndex==0)
			continue;

        for (i=0;i<WORD(nDatabases);i++)
		{
			if (ppDatabases[i]->m_wID==wNewIndex)
				break;
		}
	}
	while (i<WORD(nDatabases));
	return wNewIndex;
}

BOOL CDatabase::IsFileNamesOEM() const
{
#ifdef WIN32
	BYTE* szBuffer=new BYTE[11];
	BOOL bOEM=-1;
	CFile* dbFile=NULL;

	try 
	{
		switch (m_ArchiveType)
		{
		case archiveFile:
			dbFile=new CFile(GetArchiveName(),CFile::defRead,TRUE);
			break;
		default:
			return -1;
		}
		
		// Reading and verifing header
		dbFile->Read(szBuffer,11);
		if (szBuffer[0]=='L' && szBuffer[1]=='O' &&
			szBuffer[2]=='C' && szBuffer[3]=='A' && 
			szBuffer[4]=='T' && szBuffer[5]=='E' && 
			szBuffer[6]=='D' && szBuffer[7]=='B' &&
			szBuffer[8]=='2' && szBuffer[9]=='0' )
		{		
			bOEM=!(szBuffer[10]&0x10);
		}
		dbFile->Close();
	}
	catch (...)
	{
	}
	
	delete[] szBuffer;
	
	if (dbFile!=NULL)
		delete dbFile;
	return bOEM;
#else
	return TRUE;
#endif
}

void CDatabase::AddLocalRoots()
{
	CArrayFAP<LPSTR> aLocalRoots;
	GetLogicalDrives(&aLocalRoots);

	if (m_szRoots==NULL)
	{
		CIntArray aLengths;
		DWORD dwDataLength=0;
		for (int i=0;i<aLocalRoots.GetSize();i++)
		{
			int iLength;
			dstrlen(aLocalRoots[i],iLength);
			aLengths.Add(++iLength);
			dwDataLength+=iLength;
		}
		m_szRoots=new char[++dwDataLength];
		LPSTR ptr=m_szRoots;
		for (i=0;i<aLocalRoots.GetSize();i++)
		{
			CopyMemory(ptr,aLocalRoots[i],aLengths[i]);
			ptr+=aLengths[i];
		}
		*ptr='\0';
		return;
	}

	for (int i=0;i<aLocalRoots.GetSize();i++)
		AddRoot(aLocalRoots[i]);
}

void CDatabase::AddRoot(LPCSTR pRoot)
{
	DWORD dwLength;
	dstrlen(pRoot,dwLength);
	
	if (m_szRoots==NULL)
	{
		m_szRoots=new char[dwLength+2];
		CopyMemory(m_szRoots,pRoot,dwLength);
		m_szRoots[dwLength++]='\0';
		m_szRoots[dwLength++]='\0';
		return;
	}
	
	// Counting previous data length;
	DWORD dwDataLength;
	for (dwDataLength=0;m_szRoots[dwDataLength]!='\0' || m_szRoots[dwDataLength+1]!='\0';dwDataLength++)
	{
		if (strcasecmp(m_szRoots+dwDataLength,pRoot)==0)
			return; // Already exist
	}
	dwDataLength++;

	LPSTR pNew=new char[dwDataLength+dwLength+2];
    CopyMemory(pNew,m_szRoots,dwDataLength);
	CopyMemory(pNew+dwDataLength,pRoot,dwLength);
	dwDataLength+=dwLength;
	pNew[dwDataLength++]='\0';
	pNew[dwDataLength++]='\0';

	delete[] m_szRoots;
	m_szRoots=pNew;
}

#ifdef WIN32

void CDatabase::GetLogicalDrives(CArrayFAP<LPSTR>* paRoots)
{
	paRoots->RemoveAll();
	DWORD dwBufferLen=GetLogicalDriveStrings(0,NULL)+1;
	char* szDrives=new char[dwBufferLen];
	GetLogicalDriveStrings(dwBufferLen,szDrives);
	for (int i=0;szDrives[i*4]!='\0';i++)
	{
		if (GetDriveType(szDrives+i*4)==DRIVE_FIXED)
		{
			char* tmp=new char[3];
			tmp[0]=szDrives[i*4];
			tmp[1]=':';
			tmp[2]='\0';
			paRoots->Add(tmp);
		}
	}
	delete[] szDrives;
}

#else

void CDatabase::GetLogicalDrives(CArrayFAP<LPSTR>* paRoots)
{            
	char szDrives[27];
	getlocaldrives(szDrives);
	for (int i=0;i<27;i++)
	{
		if (szDrives[i])
		{
			char* pRoot=new char[3];
			pRoot[0]=i+'A';
			pRoot[1]=':';
			pRoot[2]='\0';
			paRoots->Add(pRoot);
		}
	}
}


/*
void CDatabase::ReadIniFile(BYTE* pData,CArrayFAP<LPSTR>* paRoots,CString* pCreator,CString* pDescription,CString* pDatabaseFile)
{
    int i;
    while (1)
    {
        LPCSTR szData=LPCSTR(pData);
    	while (szData[0]==' ' || szData[0]=='\n' || szData[0]=='\r') szData++;
        for (i=0;szData[i]!='=' && szData[i]!='\0';i++);
        if (szData[i]=='\0')
            return;    
        pData[i]='\0';
        if (strcasecmp(szData,"DatabaseFile")==0)
        {
            szData+=i+1;
            if (pDatabaseFile==NULL)
            {
				while (szData[0]!='\n' && szData[0]!='\0') szData++;
			}
            else
            {
				pDatabaseFile->Empty();
				for (i=0;szData[i]!='\0' && szData[i]!='\n'&& szData[i]!='\r';i++)
                    *pDatabaseFile<<szData[i];
            }
            if (szData[0]=='\0')
                return;
            szData++;
        }
        else if (strcasecmp(szData,"Creator")==0)
        {
            szData+=i+1;
            if (pCreator==NULL)
            {
				while (szData[0]!='\n' && szData[0]!='\0') szData++;
			}
            else
            {
				pCreator->Empty();
				for (i=0;szData[i]!='\0' && szData[i]!='\n'&& szData[i]!='\r';i++)
                    *pCreator<<szData[i];
            }
            if (szData[0]=='\0')
                return;
            szData++;
        }
        else if (strcasecmp(szData,"Description")==0)
        {
            szData+=i+1;
            if (pDescription==NULL)
            {
				while (szData[0]!='\n' && szData[0]!='\0') szData++;
			}
            else
            {
				pDescription->Empty();
				for (i=0;szData[i]!='\0' && szData[i]!='\n'&& szData[i]!='\r';i++)
                    *pDescription<<szData[i];
            }
            if (szData[0]=='\0')
                return;
            szData++;
        }
        else if (strcasecmp(szData,"Drives")==0)
        {
            szData+=i+1;
            if (paRoots==NULL)
            {
                while (szData[0]!='\n' && szData[0]!='\0') szData++;
            }
            else
            {
                paRoots->RemoveAll();

				while (szData[i]!='\n' && szData[i]!='\r' && szData[i]!='\0')
				{
					DWORD dwLength;
					for (dwLength=0;szData[dwLength]!='\r' && szData[dwLength]!='\n' && szData[dwLength]!='\0' && szData[dwLength]!=';';dwLength++);
					LPSTR pRoot=new char[dwLength+1];
					sMemCopy(pRoot,szData,dwLength);
					pRoot[dwLength]='\0';
					paRoots->Add(pRoot);
					szData+=dwLength;
					if (szData[i]==';')
						szData++;
				}
            }
            if (szData[0]=='\0')
                return;
            szData++;
        }
        while (szData[0]=='\n' || szData[0]=='\r')
        {
            if (szData[0]=='\0')
                return;
            szData++;   
        }     
    }
}
*/

#endif

LPSTR CDatabase::ConstructExtraBlock() const
{
	CString str;
	str.Format("$$LDBSET$T:%04X$",m_wThread);
	
	// Flags
	if (m_wFlags!=0)
	{
		str << "F:";
		
		if (m_wFlags&flagEnabled)
			str << "E1";
		else
			str << "E0";
		if (m_wFlags&flagGlobalUpdate)
			str << "G1";
		else
			str << "G0";
		if (m_wFlags&flagStopIfRootUnavailable)
			str << "S1";
		else
			str << "S0";
		if (m_wFlags&flagIncrementalUpdate)
			str << "I1";
		else
			str << "I0";
		
		str << '$';
	}

	if (m_szName!=NULL)
	{
		// Name
		str << "N:";
		for (int i=0;m_szName[i]!='\0';i++)
		{
			if (m_szName[i]=='$')
				str << '\\';
			str << m_szName[i];
		}
		str << '$';
	}

	if (m_ArchiveType==archiveFile && m_szArchiveName!=NULL)
	{
		// Archive file
		str << "AF:";
		for (int i=0;m_szArchiveName[i]!='\0';i++)
		{
			if (m_szArchiveName[i]=='$')
				str << '\\';
			str << m_szArchiveName[i];
		}
		str << '$';
	}

	if (m_szCreator!=NULL)
	{
		// Creator
		str << "C:";
		for (int i=0;m_szCreator[i]!='\0';i++)
		{
			if (m_szCreator[i]=='$')
				str << '\\';
			str << m_szCreator[i];
		}
		str << '$';
	}
	if (m_szDescription!=NULL)
	{
		// Description
		str << "D:";
		for (int i=0;m_szDescription[i]!='\0';i++)
		{
			if (m_szDescription[i]=='$')
				str << '\\';
			str << m_szDescription[i];
		}
		str << '$';
	}

	// Roots
	if (m_szRoots==NULL)
		str << "R:1$";
	else
	{
		LPSTR pStr=m_szRoots;
		while (*pStr!='\0')
		{
			str << "R:";
			while (*pStr!='\0')
			{
				if (*pStr=='$')
					str << '$';
				str << *pStr;
				pStr++;
			}
			str << '$';
			pStr++;
		}
	}

	// Excluded directories
	for (int i=0;i<m_aExcludedDirectories.GetSize();i++)
	{
		str << "E:";
		for (int j=0;m_aExcludedDirectories[i][j]!='\0';j++)
		{
			if (m_aExcludedDirectories[i][j]=='$')
				str << '\\';
			str << m_aExcludedDirectories[i][j];
		}
		str << '$';
	
	}

	str << '$';
	str.FreeExtra();
	return str.GiveBuffer();
}
