/* Locate32 - Copyright (c) 1997-2009 Janne Huttunen */

#if !defined(RESULTDIALOGS_H)
#define RESULTDIALOGS_H

#if _MSC_VER >= 1000
#pragma once
#endif 

class CResults : public CExceptionObject
{
public:
	CResults(BOOL bThrowExceptions=FALSE);
	CResults(DWORD dwFlags,LPCWSTR szDescription,BOOL bThrowExceptions);
	~CResults();

	BOOL Initialize(DWORD dwFlags,LPCWSTR szDescription);
	void Close();


	// Initialize structures. bDataToTmpFile has to be FALSE if SaveToHtmlFile
	// with template file going to be used, otherwise bDataTmpFile has to be TRUE
	BOOL Create(CListCtrl* pList,int* pSelectedDetails,int nSelectedDetails,BOOL bDataToTmpFile);


	BOOL SaveToFile(LPCWSTR szFile) const;
	BOOL SaveToHtmlFile(LPCWSTR szFile) const;
	BOOL SaveToHtmlFile(LPCWSTR szFile,LPCWSTR szTemplateFile);
	
private:
	DWORD m_dwFlags;
	CStringW m_strDescription;

	int m_nSelectedDetails;
	int* m_pSelectedDetails;
	DWORD* m_pLengths;
	int m_nResults;
	int m_nFiles;
	int m_nDirectories;
	
	CWordArray m_aFromDatabases;

	CStringW m_sTempFile;
	
	
	
	CAutoPtrA<CLocateDlg::ViewDetails> m_AllDetails;
	CArray<CLocatedItem*> m_Items;	

private:
	class Value {
	public:
		enum OperatorType {
			None=0,
			Not, // !
			Equal, // = or ==
			NotEqual, // !=
			Less,  // <
			Greater,  // >
			LessOrEqual, // <=
			GreaterOrEqual, // >=
			Add, // +
			Subtract, // -
			Multiply, // *
			Divide, // /
			DivideAndRoundToInfinity, // // 
			Remainder, // %
			And, // // 
			Or // %
		};

	private:
		enum Type {
			Null,
			Integer,
			String,
			Operator
		} nType;
		
		union {
			int nInteger;
			LPCWSTR pString;
			OperatorType nOperator;
		};

	private:
		Value(Type nType,LPCWSTR pString);

	public:
		Value();
		Value(int nValue);
		Value(LPCWSTR pValue);
		Value(LPCWSTR pValue,DWORD nLen);
		Value(OperatorType nValue);

		Value(const Value& val);
		Value(Value& val,BOOL bTakePtr);
		
		
		~Value();

		BOOL GetType() const {return nType; };
		BOOL IsInteger() const;
		BOOL IsActualInteger() const { return nType==Integer; }
		BOOL IsNull() const { return nType==Null; }
		BOOL IsString() const { return nType==String; }
		BOOL IsOperator() const { return nType==Operator; }
		BOOL GetOperator() const;
		void ToString();
		BOOL ToInteger(BOOL bForce);
		BOOL IsEmptyString() const { return nType==String && (pString==NULL || pString[0]==L'\0'); }
		//BOOL OnlySpaces() const;
		
		operator int() const;
		operator LPCWSTR() const;

		void Set(int nNewValue);
		void Set(OperatorType nNewValue);
		void Set(LPCWSTR pNewValue);
		void Set(LPCWSTR pNewValue,DWORD nLen);
		void Set(Value& value,BOOL bTakePtr);
		void SetPtr(LPCWSTR pNewValue);
		
		void GetString(CStringW& str) const;
		void Write(CStream& stream);

		BOOL AddString(LPCWSTR pAdd);
		BOOL AddString(LPCWSTR pAdd,DWORD nLen);
		
		Value MakeStatic();
		Value* MakeDynamic();

	};

	

	CStringMapFP<CHAR,Value*> m_Variables;

	
	Value* GetVariable(LPCSTR szName) const;
	BOOL SetVariable(LPCSTR szName,Value& value,BOOL bTakePtr=FALSE);
	BOOL SetVariable(LPCSTR szName,int newInteger);
	BOOL SetVariable(LPCSTR szName,LPCWSTR pString);
	BOOL SetVariable(LPCSTR szName,LPCWSTR pString,DWORD nLen);
	
	BOOL ParseBuffer(CStream& stream,LPCWSTR pBuffer,int iBufferLen);
	BOOL ParseBlockLength(LPCWSTR& pBuffer,int& iBufferLen,int& riBlockLen) const;
	Value* ParseFunctionsAndVariables(LPCWSTR& pPtr,int& iBufferLen,BOOL& bFreeReturnedValue);
	
	Value EvaluateCondition(LPCWSTR pBuffer,int iConditionLength);
	
	
	
	
	
	void InitializeSystemImageList(int cx);
	union { 
		IImageList* m_pSystemImageList; // Large and extra large icons (SHGetImageList needed)
		HIMAGELIST m_hSystemImageList; // Medium size icons (got using SHGetFileInfo)
	};
	DWORD m_nSystemImageListSize; // if MSB is 1, list is interface
};

class CSaveResultsDlg : public CFileDialog  
{
public:
	CSaveResultsDlg();
	virtual ~CSaveResultsDlg();
	virtual BOOL OnInitDialog(HWND hwndFocus);
	virtual BOOL OnFileNameOK();
	virtual BOOL OnNotify(int idCtrl,LPNMHDR pnmh);
	virtual BOOL OnCommand(WORD wID,WORD wNotifyCode,HWND hControl);
	
	BOOL ListNotifyHandler(NMLISTVIEW *pNm);
	BOOL ItemUpOrDown(BOOL bUp);
	void AddTemplates();
	
public:
	DWORD m_nFlags;
	CIntArray m_aDetails;
	CStringW m_strDescription;
	CStringW m_strTemplate;

private:
	CListCtrl* m_pList;
	CImageList m_ToolbarIL,m_ToolbarILHover,m_ToolbarILDisabled;

	CArrayFAP<LPCWSTR> m_TemplateFiles;
};






/////////////////////////////////////////////////////
// Inline function for CResults::Value

inline CResults::Value::Value(Type nTyp,LPCWSTR pStr)
: nType(nTyp),pString(pStr)
{
}

inline CResults::Value::Value()
: nType(Null),pString(NULL)
{
}


inline CResults::Value::Value(int nValue) 
: nType(Integer),nInteger(nValue)
{
}

inline CResults::Value::Value(OperatorType nValue) 
: nType(Operator),nOperator(nValue)
{
}

inline CResults::Value::Value(LPCWSTR pValue) 
:	nType(String)
{
	pString=alloccopy(pValue); 
}

inline CResults::Value::Value(LPCWSTR pValue,DWORD nLen)
: nType(String)
{
	pString=alloccopy(pValue,nLen); 
}

inline CResults::Value::~Value()
{ 
	if (nType==String && pString!=NULL) 
		delete[] pString; 
}

inline CResults::Value::Value(const Value& val)
:	nType(val.nType)
{ 
	if (nType==String && val.pString!=NULL)
		pString=alloccopy(val.pString);
	else
		nInteger=val.nInteger;
}

inline CResults::Value::Value(Value& val,BOOL bTakePtr)
:	nType(val.nType)
{ 
	if (nType==String && val.pString!=NULL)
	{
		if (bTakePtr)
		{
			pString=val.pString;
			val.pString=NULL;
		}
		else
			pString=alloccopy(val.pString);
	}
	else
		nInteger=val.nInteger;
}

inline CResults::Value::operator int() const 
{ 
	ASSERT(nType!=Operator) // Do not use with operators

	if (nType==String && pString!=NULL)
	{
		int nSpaces;
		for (nSpaces=0;pString[nSpaces]==' ' || pString[nSpaces]=='\t';nSpaces++);
		return _wtoi(pString+nSpaces);
	}
	return nInteger; 
}

inline void CResults::Value::Set(int nNewValue)
{
	if (nType==String && pString!=NULL) 
		delete[] pString; 
	nType=Integer;
	nInteger=nNewValue;
}

inline void CResults::Value::Set(CResults::Value::OperatorType nNewValue)
{
	if (nType==String && pString!=NULL) 
		delete[] pString; 
	nType=Operator;
	nOperator=nNewValue;
}

inline void CResults::Value::Set(CResults::Value& val, BOOL bTakePtr)
{
	if (nType==String && pString!=NULL) 
		delete[] pString; 
	nType=val.nType;
	if (nType==String && val.pString!=NULL)
	{
		if (bTakePtr)
		{
			pString=val.pString;
			val.pString=NULL;
		}
		else
			pString=alloccopy(val.pString);
	}
	else
		nInteger=val.nInteger;
}

inline void CResults::Value::Set(LPCWSTR pNewValue)
{
	if (nType==String && pString!=NULL) 
		delete[] pString; 
	nType=String;
	pString=alloccopy(pNewValue);
}

inline void CResults::Value::Set(LPCWSTR pNewValue,DWORD nLen)
{
	if (nType==String && pString!=NULL) 
		delete[] pString; 
	nType=String;
	pString=alloccopy(pNewValue,nLen);
}

inline void CResults::Value::SetPtr(LPCWSTR pNewValue)
{
	if (nType==String && pString!=NULL) 
		delete[] pString; 
	nType=String;
	pString=pNewValue;
}


inline BOOL CResults::Value::GetOperator() const
{
	if (nType!=Operator)
		return None;
	return nOperator;
}

		
inline CResults::Value* CResults::Value::MakeDynamic()
{
	Value* pRet=new Value(nType,pString);
	pString=NULL;
	return pRet;
}

inline CResults::Value CResults::Value::MakeStatic()
{
	LPCWSTR pTemp=pString;
	pString=NULL;	
	return Value(nType,pTemp);
}



/////////////////////////////////////////////////////
// Inline function for CResults

inline CResults::CResults(BOOL bThrowExceptions)
:	m_nSelectedDetails(0),m_pSelectedDetails(NULL),m_pLengths(NULL),
	m_dwFlags(0),m_hSystemImageList(NULL),CExceptionObject(bThrowExceptions)
{
}

inline CResults::CResults(DWORD dwFlags,LPCWSTR szDescription,BOOL bThrowExceptions)
:	m_nSelectedDetails(0),m_pSelectedDetails(NULL),m_pLengths(NULL),
	m_hSystemImageList(NULL),CExceptionObject(bThrowExceptions)
{
	Initialize(dwFlags,szDescription);
}

inline CResults::~CResults()
{
	Close();
}

inline CResults::Value* CResults::GetVariable(LPCSTR szName) const
{
	POSITION pPos=m_Variables.Find(szName);
	if (pPos==NULL)
		return NULL;
	return m_Variables.GetAt(pPos);
}

inline BOOL CResults::SetVariable(LPCSTR szName,int newInteger)
{
	POSITION pPos=m_Variables.Find(szName);
	if (pPos!=NULL)
		m_Variables.GetAt(pPos)->Set(newInteger);
	else
		m_Variables.AddTail(szName,new Value(newInteger));
	return TRUE;
}

inline BOOL CResults::SetVariable(LPCSTR szName,Value& value,BOOL bTakePtr)
{
	POSITION pPos=m_Variables.Find(szName);
	if (pPos!=NULL)
		m_Variables.GetAt(pPos)->Set(value,bTakePtr);
	else 
		m_Variables.AddTail(szName,new Value(value,bTakePtr));
	return TRUE;
}

inline BOOL CResults::SetVariable(LPCSTR szName,LPCWSTR pString)
{
	POSITION pPos=m_Variables.Find(szName);
	if (pPos!=NULL)
		m_Variables.GetAt(pPos)->Set(pString);
	else
		m_Variables.AddTail(szName,new Value(pString));
	return TRUE;
}

inline BOOL CResults::SetVariable(LPCSTR szName,LPCWSTR pString,DWORD nLen)
{
	POSITION pPos=m_Variables.Find(szName);
	if (pPos!=NULL)
		m_Variables.GetAt(pPos)->Set(pString);
	else
		m_Variables.AddTail(szName,new Value(pString,nLen));
	return TRUE;
}


#endif
