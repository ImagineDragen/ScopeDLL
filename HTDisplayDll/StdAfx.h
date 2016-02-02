// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__CA35C6FA_9A4A_467F_A549_8C88AAAB1935__INCLUDED_)
#define AFX_STDAFX_H__CA35C6FA_9A4A_467F_A549_8C88AAAB1935__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000




// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include "..\\Scope\\DefMacro.h"//add by zhang

// TODO: reference additional headers your program requires here
#include <math.h>
#define DLL_API extern "C" __declspec(dllexport)
/* commit by zhang 20160202
#define MIN_DATA						0				//
#ifdef VERTICAL_8BIT
	#define MAX_DATA					255				//
	#define MID_DATA					128				//
#elif defined VERTICAL_10BIT
	#define MAX_DATA					1023			//
	#define MID_DATA					512				//
#elif defined VERTICAL_12BIT
	#define MAX_DATA					4095			//
	#define MID_DATA					2048			//
#endif
*/
#define VECTORS						0
#define DOTS						1
#define V_GRID_NUM					8//垂直8个大格
#define H_GRID_NUM					10//水平10个大格
#define	PI							3.14159265358979323846
#define DC							0
#define AC							1
#define GND							2
#define RISE						0
#define FALL						1
#define YT_NORMAL					0
#define YT_SCAN						1
#define YT_ROLL						2
/*  commit by zhang 20160202
#ifndef MINISCOPE
	#define DEF_READ_DATA_LEN			BUF_10K_LEN		//默认读取的数据长度
	#define DEF_DRAW_DATA_LEN			10000			
#else
	#define DEF_READ_DATA_LEN			BUF_4K_LEN		
	#define DEF_DRAW_DATA_LEN			BUF_4K_LEN
#endif

#define BUF_4K_LEN					4000
#define BUF_10K_LEN					10240
#define BUF_1M_LEN					1048576
#define BUF_2M_LEN					2097152
#define BUF_4M_LEN					4194304
#define BUF_8M_LEN					8388608
#define BUF_12M_LEN					12582912			
#define BUF_16M_LEN					16252928
#define BUF_40K_LEN					40960//32768
  */
#define CUR_NONE					0
#define CUR_CROSS					1
#define CUR_TRACE					2
#define CUR_VERTICAL				3
#define CUR_HORIZONTAL				4
#define SAMPLE_NORMAL			0
#define SAMPLE_PEAK				1
#define SAMPLE_ETS				2
#define SAMPLE_AVG				3


#include "HTDisplayDll.h"

#ifdef GDIPLUS
	#ifndef ULONG_PTR
		#define ULONG_PTR ULONG
	#endif
	#include "ObjSafe.h"
	#pragma comment(lib, "gdiplus.lib")
	
	#include "GdiPlus.h"
	using namespace Gdiplus;

#endif



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__CA35C6FA_9A4A_467F_A549_8C88AAAB1935__INCLUDED_)
