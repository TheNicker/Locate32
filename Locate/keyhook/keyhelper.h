/* Keyhook handler for Locate
Copyright (C) 2004 Janne Huttunen				*/

#if !defined(KEYHOOK_H)
#define KEYHOOK_H

#if _MSC_VER >= 1000
#pragma once
#endif 

#ifdef KEYHOOK_EXPORTS
#define KEYHOOK_API __declspec(dllexport)
#else
#define KEYHOOK_API DECLSPEC_IMPORT
#endif

extern "C" {
	KEYHOOK_API HHOOK SetHook(HWND hTargetWnd);
	KEYHOOK_API BOOL UnsetHook(HHOOK hHook);

	KEYHOOK_API LRESULT CALLBACK HookKeyboardProc(int code,WPARAM wParam,LPARAM lParam);
}


// Messages
#define WM_ANOTHERINSTANCE			WM_APP+101

#endif