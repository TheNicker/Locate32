////////////////////////////////////////////////////////////////////
// HFC Library - Copyright (C) 1999-2008 Janne Huttunen
////////////////////////////////////////////////////////////////////

#ifndef HFCMEMORY_H
#define HFCMEMORY_H




/*
	 XXXXXXXX is normal version
	iXXXXXXXX is inline version
	dXXXXXXXX is define version
*/
// Memory copiers
#define MemCopy(dst,src,len) CopyMemory(dst,src,len)
#define dMemCopy(dst,src,len) \
{for (register UINT __i_=0;__i_<(len);__i_++) \
((BYTE*)(dst))[__i_]=((BYTE*)(src))[__i_];}

// Memory setters
#define MemSet(dst,value,count) FillMemory(dst,count,value)
#define dMemSet(dst,byte,count) \
{for (register UINT __i_=0;__i_<(count);__i_++) \
((BYTE*)(dst))[__i_]=(BYTE)(byte);}



//////////////////////////////////////
// CLASSES

class CAlloc : public CExceptionObject
{
public:
	CAlloc(SIZE_T nSize) { m_pData=new BYTE[nSize]; if (m_bThrow && m_pData==NULL) throw CException(CException::cannotAllocate); }
	CAlloc(BYTE* pData) { m_pData=pData; }
	~CAlloc() { if (m_pData!=NULL) delete[] m_pData; }
	
	void Free() { if (m_pData!=NULL) delete[] m_pData; m_pData=NULL; }
	BOOL IsAllocated() const { return m_pData!=NULL; }
	void ReAlloc(SIZE_T nSize) { Free(); m_pData=new BYTE[nSize];  if (m_bThrow && m_pData==NULL) throw CException(CException::cannotAllocate); }
	
	operator BYTE*() { return m_pData; }
	operator LPCSTR() const { return (LPCSTR)m_pData; }
	operator LPSTR() { return (LPSTR)m_pData; }
	operator INT*() { return (INT*)m_pData; }
	operator UINT*() { return (UINT*)m_pData; }
	operator DWORD*() { return (DWORD*)m_pData; }
	operator LONG*() { return (LONG*)m_pData; }
	operator HANDLE*() { return (HANDLE*)m_pData; }
	operator void*() { return (void*)m_pData; }

	
public:
	BYTE* m_pData;
};

template<class TYPE>
class CAllocTmpl : public CExceptionObject
{
public:
	CAllocTmpl() { m_pData=new TYPE;  }
	CAllocTmpl(BOOL bThrow) { m_pData=new TYPE; m_bThrow=bThrow; if (m_bThrow && m_pData==NULL) throw CException(CException::cannotAllocate); }
	CAllocTmpl(TYPE* pData) { m_pData=pData; }
	~CAllocTmpl() { if (m_pData!=NULL) delete m_pData; }
	
	void Free() { if (m_pData!=NULL) delete m_pData; m_pData=NULL; }
	BOOL IsAllocated() const { return m_pData!=NULL; }
	
	operator void*() { return (void*)m_pData; }
	operator TYPE*() { return m_pData; }
	TYPE* operator ->() {return m_pData; }

	
public:
	TYPE* m_pData;
};

template<class TYPE>
class CAllocArrayTmpl : public CExceptionObject
{
public:
	CAllocArrayTmpl(SIZE_T dwSize) { m_pData=new TYPE[dwSize];  }
	CAllocArrayTmpl(SIZE_T dwSize,BOOL bThrow) { m_pData=new TYPE[dwSize]; m_bThrow=bThrow; if (m_bThrow && m_pData==NULL) throw CException(CException::cannotAllocate); }
	CAllocArrayTmpl(TYPE* pData) { m_pData=pData; }
	~CAllocArrayTmpl() { if (m_pData!=NULL) delete[] m_pData; }
	
	void Free() { if (m_pData!=NULL) delete[] m_pData; m_pData=NULL; }
	BOOL IsAllocated() const { return m_pData!=NULL; }
	
	operator void*() { return (void*)m_pData; }
	operator TYPE*() { return m_pData; }

	TYPE* operator ->() {return m_pData; }
	TYPE operator[](int nIndex) const { return m_pData[nIndex]; }
	TYPE& operator[](int nIndex) { return m_pData[nIndex]; }
	TYPE operator[](UINT nIndex) const { return m_pData[nIndex]; }
	TYPE& operator[](UINT nIndex) { return m_pData[nIndex]; }
	TYPE operator[](DWORD nIndex) const { return m_pData[nIndex]; }
	TYPE& operator[](DWORD nIndex) { return m_pData[nIndex]; }
	TYPE operator[](LONG nIndex) const { return m_pData[nIndex]; }
	TYPE& operator[](LONG nIndex) { return m_pData[nIndex]; }
#ifdef _WIN64
	TYPE operator[](LONG_PTR nIndex) const { return m_pData[nIndex]; }
	TYPE& operator[](LONG_PTR nIndex) { return m_pData[nIndex]; }
	TYPE operator[](ULONG_PTR nIndex) const { return m_pData[nIndex]; }
	TYPE& operator[](ULONG_PTR nIndex) { return m_pData[nIndex]; }
#endif	
	
public:
	TYPE* m_pData;
};




#ifdef WIN32
class CGlobalAlloc : public CExceptionObject
{
public:
	enum allocFlags { 
		fixed=GMEM_FIXED,
		moveable=GMEM_MOVEABLE,
		zeroinit=GMEM_ZEROINIT,
		ptr=GPTR,
		hnd=GHND,
		modify=GMEM_MODIFY ,
		discardable=GMEM_DISCARDABLE,
		noCompact=GMEM_NOCOMPACT,
		noDiscard=GMEM_NODISCARD,
		notBanked=GMEM_NOT_BANKED,
		share=GMEM_SHARE,
		ddeshare=GMEM_DDESHARE,
		notify=GMEM_NOTIFY
	};


	CGlobalAlloc(SIZE_T nSize,allocFlags nFlags=moveable);
	CGlobalAlloc(HGLOBAL hGlobal,allocFlags nFlags=ptr);
	~CGlobalAlloc();
	
	BOOL Alloc(SIZE_T nSize,allocFlags nFlags=moveable);
	BOOL ReAlloc(SIZE_T nSize,allocFlags nFlags=moveable);
	void Free();
	BOOL IsAllocated() const;
	SIZE_T GetSize() const;
	
	void Lock();
	void Unlock();
	BOOL IsLocked() const;

	allocFlags GetFlags() const;
	BOOL Discard();
	
	operator BYTE*() { Lock(); return m_pData; }
	operator LPCSTR() { Lock(); return (LPCSTR)m_pData; }
	operator LPSTR() { Lock(); return (LPSTR)m_pData; }
	operator INT*() { Lock(); return (INT*)m_pData; }
	operator UINT*() { Lock(); return (UINT*)m_pData; }
	operator DWORD*() { Lock(); return (DWORD*)m_pData; }
	operator LONG*() { Lock(); return (LONG*)m_pData; }
	operator HANDLE*() { Lock(); return (HANDLE*)m_pData; }
	operator void*() { Lock(); return (void*)m_pData; }
	HGLOBAL GetGlobal() const { return m_hGlobal;};

	
public:
	BYTE* m_pData;
	HGLOBAL m_hGlobal;
	allocFlags m_nFlags;
};

class CHeap : CExceptionObject
{
public:
	enum attributes {
		none=0,
		noSerialize=HEAP_NO_SERIALIZE,
		generateExceptions=HEAP_GENERATE_EXCEPTIONS,
		growable=HEAP_GROWABLE,
		zeroMemory=HEAP_ZERO_MEMORY,
		reallocInPlaceOnly=HEAP_REALLOC_IN_PLACE_ONLY,
		tailCheckingEnabled=HEAP_TAIL_CHECKING_ENABLED,
		freeCheckingEnabled=HEAP_FREE_CHECKING_ENABLED,
		disableCoalesceOnFree=HEAP_DISABLE_COALESCE_ON_FREE,
		createAlign16=HEAP_CREATE_ALIGN_16,
		createEnableTracing=HEAP_CREATE_ENABLE_TRACING 
	};

	class CHeapBlock
	{
	public:
		enum attributes {
			none=0,
			noSerialize=HEAP_NO_SERIALIZE,
			generateExceptions=HEAP_GENERATE_EXCEPTIONS,
			growable=HEAP_GROWABLE,
			zeroMemory=HEAP_ZERO_MEMORY,
			reallocInPlaceOnly=HEAP_REALLOC_IN_PLACE_ONLY,
			tailCheckingEnabled=HEAP_TAIL_CHECKING_ENABLED,
			freeCheckingEnabled=HEAP_FREE_CHECKING_ENABLED,
			disableCoalesceOnFree=HEAP_DISABLE_COALESCE_ON_FREE,
			createAlign16=HEAP_CREATE_ALIGN_16,
			createEnableTracing=HEAP_CREATE_ENABLE_TRACING 
		};
		
		CHeapBlock(void* pBlock);
		CHeapBlock(HANDLE hHeap,attributes nAttributes);
		~CHeapBlock();
		
		SIZE_T GetSize();
		BOOL Free();
		BOOL Release() { Free(); delete this; }

		operator BYTE*() { return (BYTE*)m_pBlock; }
		operator LPCSTR() { return (LPCSTR)m_pBlock; }
		operator LPSTR() { return (LPSTR)m_pBlock; }
		operator INT*() { return (INT*)m_pBlock; }
		operator UINT*() { return (UINT*)m_pBlock; }
		operator DWORD*() { return (DWORD*)m_pBlock; }
		operator LONG*() { return (LONG*)m_pBlock; }
		operator HANDLE*() { return (HANDLE*)m_pBlock; }
		operator void*() { return (void*)m_pBlock; }

	public:
		void* m_pBlock;
	};


	CHeap();
	CHeap(SIZE_T dwInitialSize,SIZE_T dwMaximumSize,attributes nAttributes=none);
	~CHeap();
	
	BOOL Create(SIZE_T dwInitialSize,SIZE_T dwMaximumSize,attributes nAttributes=none);
	BOOL Destroy();
	BOOL IsAllocated();

	SIZE_T Compact(attributes nAttributes=none);
	BOOL Lock();
	BOOL Unlock();
	BOOL Validate(LPCVOID lpBlock=NULL,attributes nAttributes=none);
	BOOL Walk(LPPROCESS_HEAP_ENTRY lpEntry);

	CHeapBlock* Alloc(SIZE_T nSize,attributes nAttributes=none);
	
	static CHeap GetProcessHeap();

	
private:
	CHeap(HANDLE hHeap) { m_hHeap=hHeap; m_bDestroy=FALSE; }

public:
	BOOL m_bDestroy;
	HANDLE m_hHeap;
};

#endif

// CAutoPtr, works like auto_ptr in STL
template<class TYPE> 
class CAutoPtr
{
public:
	CAutoPtr(TYPE* data=NULL) { m_data=data;  }
	CAutoPtr(CAutoPtr<TYPE>& another) { m_data=another.Release(); }
	~CAutoPtr();

	TYPE* Release();
	
	operator TYPE*() const { return m_data; }
	operator TYPE*&() { return m_data; }
	
	TYPE* operator ->() { return m_data; }

	void Assign(TYPE* data);
	void Assign(CAutoPtr<TYPE>& another);

	CAutoPtr<TYPE>& operator=(TYPE* data);
	CAutoPtr<TYPE>& operator=(CAutoPtr<TYPE>& another);


private:
	TYPE* m_data;

};


// CAutoPtrA, use detele[] when freeing memory
template<class TYPE> 
class CAutoPtrA
{
public:
	CAutoPtrA(TYPE* data=NULL) { m_data=data;  }
	CAutoPtrA(CAutoPtrA<TYPE>& another) { m_data=another.Release(); }
	~CAutoPtrA();

	TYPE* Release();
	
	operator TYPE*() const { return m_data; }
	operator TYPE*&() { return m_data; }
	
	TYPE* operator ->() { return m_data; }

	void Assign(TYPE* data);
	void Assign(CAutoPtr<TYPE>& another);

	CAutoPtrA<TYPE>& operator=(TYPE* data);
	CAutoPtrA<TYPE>& operator=(CAutoPtrA<TYPE>& another);

	// No need for these, compiler use TYPE*& automatically
	//TYPE operator[](LONG_PTR nIndex) const { return m_data[nIndex]; }
	//TYPE& operator[](LONG_PTR nIndex) { return m_data[nIndex]; };
	


private:
	TYPE* m_data;

};

// CDataContainer, keeps memory until m_dwCount==0
template<class TYPE> 
class CDataContainer
{
public:
	CDataContainer(TYPE* data=NULL);
	~CDataContainer();

	void AddRef() { m_dwCount++; }
	CDataContainer* Release();
	
	operator TYPE*() const { return m_data; }
	operator TYPE*&() { return m_data; }
	
	TYPE* operator ->() { return m_data; }

private:
	TYPE* m_data;
	DWORD m_dwCount;

};

// CDataContainer, keeps memory until m_dwCount==0
template<class TYPE> 
class CDataContainerA
{
public:
	CDataContainerA(TYPE* data=NULL);
	~CDataContainerA();

	void AddRef() { m_dwCount++; }
	CDataContainerA* Release();
	
	operator TYPE*() const { return m_data; }
	operator TYPE*&() { return m_data; }
	
	TYPE* operator ->() { return m_data; }

private:
	TYPE* m_data;
	DWORD m_dwCount;

};


#include "Memory.inl"

#endif
