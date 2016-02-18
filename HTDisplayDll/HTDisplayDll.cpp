// HTDisplayDll.cpp : Defines the entry point for the DLL application.
//

/*
20100105
修改了 HTDrawWaveInYT

20100209
增加函数 HTDrawAcquireMode









*/

#include "stdafx.h"
#include <string.h>

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

DLL_API long WINAPI UserRound(double dbVal)//四舍五入
{
	if(dbVal < 0)
		return (long)(dbVal - 0.5);
	else
		return (long)(dbVal + 0.5);
}
//
void DrawGridScale(HDC hDC,CONST RECT* lpRect,COLORREF PenColor,USHORT nHoriGridNum,USHORT nVertGridNum,USHORT IsGrid,BOOL bLimit)
{
	HPEN CurrentPen,oldPen;
	int i,j,nMaxHScale,nMaxVScale,nShortScale,nLongScale;
	double x,y,xStep,yStep;
	POINT ptCross;
	CurrentPen = CreatePen(PS_SOLID,1,PenColor);
	oldPen = (HPEN)SelectObject(hDC,CurrentPen);
	ptCross.x = 0;
	ptCross.y  =0;
	nMaxHScale = nHoriGridNum * 5;//水平平均分成60份。0~50共61个刻度
	nMaxVScale = nVertGridNum * 5;//垂直平均分成40份。0~40共41个刻度
	if(1)
	{
		xStep = float((lpRect->right - lpRect->left)*1.0/nMaxHScale);//每小格的长度，每5小格为1大格
		yStep = float((lpRect->bottom - lpRect->top)*1.0/nMaxVScale);
	}
	else//网格是1:1的正方形
	{
		xStep = yStep = float((lpRect->right - lpRect->left)*1.0/nMaxHScale);//每小格的长度，每5小格为1大格
	}

	//四周刻度线的长度
	nShortScale = int(xStep/3);
	nLongScale = int(xStep/2);
	if(bLimit)
	{
		if(nShortScale > 5)
		{
			nShortScale = 5;
		}
		if(nLongScale > 8)
		{
			nLongScale = 8;
		}
	}
//垂直方向(线的方向)
	x = lpRect->left;
	y = lpRect->top;
	for(i=0;i<(nMaxVScale/2);i++)
	{
		y = y + yStep;
	}
	ptCross.y = int(y);
	y = lpRect->top;
	for(i=1;i<nMaxHScale;i++)//i=0或者nMaxHScale时不画，即不画两边
	{
		x = x + xStep;
		if(i == (nMaxHScale/2))//中间十字线垂直线
		{
			MoveToEx(hDC,int(x),lpRect->top,NULL);
			LineTo(hDC,int(x),lpRect->bottom);
			ptCross.x = int(x);
		}
		else if(i%5==0 && i>=5)//5个小格为1大格
		{
			//顶部长短线
			MoveToEx(hDC,int(x),int(y),NULL);
			LineTo(hDC,int(x),int(y+nLongScale));
			//底部长短线
			MoveToEx(hDC,int(x),lpRect->bottom,NULL);
			LineTo(hDC,int(x),lpRect->bottom-nLongScale);
			//垂直虚线
			if(IsGrid == 1)//网格是否可用
			{
				for(j=1;j<nMaxVScale;j++)
				{
					y = y + yStep;
					SetPixel(hDC,int(x),int(y),PenColor);
				}
				y = lpRect->top;
			}
		}
		else
		{
			//顶部短短线
			MoveToEx(hDC,int(x),int(y),NULL);
			LineTo(hDC,int(x),int(y+nShortScale));
			//底部短短线
			MoveToEx(hDC,int(x),lpRect->bottom,NULL);
			LineTo(hDC,int(x),lpRect->bottom-nShortScale);
		}
		//画中间十字线->水平线上的刻度
		MoveToEx(hDC,int(x),ptCross.y,NULL);
		LineTo(hDC,int(x),ptCross.y+nShortScale);
		MoveToEx(hDC,int(x),ptCross.y,NULL);
		LineTo(hDC,int(x),ptCross.y-nShortScale);
	}
//水平方向(线的方向)
	x = lpRect->left;
	y = lpRect->top;
	for(i=1;i<nMaxVScale;i++)
	{
		y = y + yStep;
		if(i == (nMaxVScale/2))//中间十字线水平线
		{
			MoveToEx(hDC,lpRect->left,int(y),NULL);
			LineTo(hDC,lpRect->right,int(y));
		}
		else if(i%5==0 && i>=5)
		{
			//左边长短线
			MoveToEx(hDC,int(x),int(y),NULL);
			LineTo(hDC,int(x+nLongScale),int(y));
			//右边长短线
			MoveToEx(hDC,lpRect->right,int(y),NULL);
			LineTo(hDC,int(lpRect->right-nLongScale),int(y));
			//水平虚线
			if(IsGrid == 1)//网格是否可用
			{
				for(j=1;j<nMaxHScale;j++)
				{
					x = x + xStep;
					SetPixel(hDC,int(x),int(y),PenColor);
				}
				x = lpRect->left;
			}
		}
		else
		{
			//左边短短线
			MoveToEx(hDC,int(x),int(y),NULL);
			LineTo(hDC,int(x+nShortScale),int(y));
			//右边短短线
			MoveToEx(hDC,lpRect->right,int(y),NULL);
			LineTo(hDC,int(lpRect->right-nShortScale),int(y));
		}
		//画中间十字线->垂直线上的刻度
		MoveToEx(hDC,ptCross.x,int(y),NULL);
		LineTo(hDC,ptCross.x+nShortScale,int(y));
		MoveToEx(hDC,ptCross.x,int(y),NULL);
		LineTo(hDC,ptCross.x-nShortScale,int(y));
	}
	SelectObject(hDC,oldPen);
	DeleteObject(CurrentPen);
}

void DrawGridScaleNew(HDC hDC,CONST RECT* lpRect,COLORREF PenColor,USHORT nHoriGridNum,USHORT nVertGridNum,USHORT IsGrid,BOOL bLimit)
{
	HPEN CurrentPen,oldPen;
	int i,j,nMaxHScale,nMaxVScale,nShortScale,nLongScale;
	double x,y,deltaX,deltaY;
	short HORI_SCALE = 100;
	short VERT_SCALE = 255;
	double dbTemp = 0.0;
	COLORREF clrScale = RGB(200,200,200);

	CurrentPen = CreatePen(PS_SOLID,1,clrScale);
	oldPen = (HPEN)SelectObject(hDC,CurrentPen);

	nMaxHScale = nHoriGridNum * 5;//水平共50个刻度
	nMaxVScale = nVertGridNum * 5;//垂直共40个刻度
	
	deltaX = float((lpRect->right - lpRect->left)*1.0/HORI_SCALE);//水平步进的长度(按照水平触发位置0 ~ 100计算)
	deltaY = float((lpRect->bottom - lpRect->top)*1.0/VERT_SCALE);//垂直每个步进的长度(分255个步进)

//边框上刻度线的长度
	nShortScale = UserRound(deltaX/3);//短刻度线
	nLongScale = UserRound(deltaX/2);//长刻度线
	if(bLimit)//限制一下刻度线的长度
	{
		if(nShortScale > 5)
			nShortScale = 5;
		if(nLongScale > 8)
			nLongScale = 8;
	}
//行
	x = lpRect->left;
	y = lpRect->top;
	for(i = 0; i <= VERT_SCALE; i++)
	{
		if(i == 0)//Top 刻度线
		{
			for(j = 0; j <= HORI_SCALE; j++)
			{
				if(j == HORI_SCALE/2)//中间垂直实线
				{
					MoveToEx(hDC,UserRound(x),lpRect->top,NULL);
					LineTo(hDC,UserRound(x),lpRect->bottom);
				}
				else if((j % 10) == 0)
				{
					MoveToEx(hDC,UserRound(x),UserRound(y),NULL);
					LineTo(hDC,UserRound(x),UserRound(y+nLongScale));
				}
				else if((j % 2) == 0)
				{
					MoveToEx(hDC,UserRound(x),UserRound(y),NULL);
					LineTo(hDC,UserRound(x),UserRound(y+nShortScale));
				}
				x = x + deltaX;
			}
		}
		else if( i == VERT_SCALE)//Bottom 刻度线
		{
			for(j = 0; j <= HORI_SCALE; j++)
			{
				if((j % 10) == 0)
				{
					MoveToEx(hDC,UserRound(x),UserRound(y),NULL);
					LineTo(hDC,UserRound(x),UserRound(y-nLongScale));
				}
				else if((j % 2) == 0)
				{
					MoveToEx(hDC,UserRound(x),UserRound(y),NULL);
					LineTo(hDC,UserRound(x),UserRound(y-nShortScale));
				}
				x = x + deltaX;
			}
		}
		else if(i == UserRound(VERT_SCALE / 2.0))//中间水平实线
		{
			MoveToEx(hDC,lpRect->left,UserRound(y),NULL);
			LineTo(hDC,lpRect->right,UserRound(y));
			for(j = 0; j <= HORI_SCALE; j++)//实线上的刻度线
			{
				if((j % 2) == 0)
				{
					MoveToEx(hDC,UserRound(x),UserRound(y+nShortScale),NULL);
					LineTo(hDC,UserRound(x),UserRound(y-nShortScale-1));
				}
				x = x + deltaX;
			}
		}
		else if((i % UserRound(VERT_SCALE * 1.0 / nVertGridNum)) == 0)//水平网点
		{
			if(IsGrid == 1)
			{
				for(j = 0; j <= HORI_SCALE; j++)
				{
					if(j % 2 == 0)
					{
						SetPixel(hDC,UserRound(x),UserRound(y),PenColor);
					}
					x = x + deltaX;
				}
			}
		}
		else
		{
		}
		x = lpRect->left;
		y = y + deltaY;
	}
//列
	x = lpRect->left;
	y = lpRect->top;
	dbTemp = UserRound(VERT_SCALE * 1.0 / nVertGridNum) / 5.0;// = 6.4
	for(j = 0; j <= HORI_SCALE; j++)
	{
		if(j == 0)
		{
			for(i = 0; i < nMaxVScale; i++)
			{
				if(i % 5 == 0)//长刻度线
				{
					MoveToEx(hDC,UserRound(x),UserRound(y),NULL);
					LineTo(hDC,UserRound(x+nLongScale),UserRound(y));
				}
				else
				{
					MoveToEx(hDC,UserRound(x),UserRound(y),NULL);
					LineTo(hDC,UserRound(x+nShortScale),UserRound(y));
				}
				y = y + dbTemp * deltaY;
			}
		}
		if(j == HORI_SCALE/2)//中间垂直实线上的刻度线
		{
			for(i = 0; i < nMaxVScale; i++)
			{
				MoveToEx(hDC,UserRound(x+nShortScale),UserRound(y),NULL);
				LineTo(hDC,UserRound(x-nShortScale-1),UserRound(y));
				y = y + dbTemp * deltaY;
			}
		}
		else if(j == HORI_SCALE)
		{
			for(i = 0; i < nMaxVScale; i++)
			{
				if(i % 5 == 0)//长刻度线
				{
					MoveToEx(hDC,UserRound(x),UserRound(y),NULL);
					LineTo(hDC,UserRound(x-nLongScale),UserRound(y));
				}
				else
				{
					MoveToEx(hDC,UserRound(x),UserRound(y),NULL);
					LineTo(hDC,UserRound(x-nShortScale),UserRound(y));
				}
				y = y + dbTemp * deltaY;
			}
		}
		else if(j % 10 == 0 && j > 0)//垂直网点
		{
			if(IsGrid == 1)
			{
				for(i = 0; i < nMaxVScale; i++)
				{
					SetPixel(hDC,UserRound(x),UserRound(y),PenColor);
					y = y + dbTemp * deltaY;
				}
			}
		}
		x = x + deltaX;
		y = lpRect->top;
	}

	SelectObject(hDC,oldPen);
	DeleteObject(CurrentPen);
}

void DrawWaveXY(HDC hDC,CONST RECT* lpRect,POINT* ptData,ULONG nDataLen,COLORREF clrRGB)
{
	ULONG i=0,j=0;
	POINT* ptSource = ptData;

	for(i=0;i<nDataLen;i++)
	{
		j = i + 1;
		if(j < nDataLen)
		{
			for(;j < nDataLen;j++)
			{
				if(ptSource[i].x == ptSource[j].x)//横坐标相等(即在同一列上)
				{
					if(ptSource[i].y == ptSource[j].y)
					{
						break;//如果纵坐标相同，则直接跳到下一个点(i++)
					}
					else
					{
						continue;//如果纵坐标不相同，则跳到下一个横坐标相同的点(j++)
					}
				}
				else
				{
					SetPixel(hDC,ptSource[i].x,ptSource[i].y,clrRGB);
					break;
				}
			}
			SetPixel(hDC,ptSource[i].x,ptSource[i].y,clrRGB);//add by yt 20120321-
		}
		else
		{
			SetPixel(hDC,ptSource[i].x,ptSource[i].y,clrRGB);
		}
	}
}
/*
void DrawWaveYTNew(HDC hdDC,			//
				   CONST RECT* lpRect,
				   WORD* pSrcData,		//源数据
				   ULONG nSrcDataLen,	//源数据长度
				   ULONG nDisDataLen,	//要显示的数据长度
				   COLORREF clrRGB		//绘图颜色
				   )
{
	long nSrnWidth = lpRect->right - lpRect->left + 1;//获取Rect的像素列数
	long nSrnHeight = lpRect->bottom - lpRect->top + 1;//获取Rect的像素行数
	double dbDeltaX = nSrcWidth * 1.0 / nDisDataLen;//要显示数据的水平之间的像素间隔
	double dbDeltaY = nSrcHeight * 1.0 / MAX_DATA;//要显示数据的垂直之间的像素间隔
}
*/

void DrawWaveYT(HDC hDC,CONST RECT* lpRect,WORD* pData,ULONG nDataLen,/*double fHTriggerPos,*/COLORREF clrRGB,USHORT nDisMode,ULONG nScanLen)
{
	ULONG i;
	ULONG j = 0;
	POINT* ptSource;
	double x,y,fxStep,fyStep;
	long nWidth = lpRect->right - lpRect->left + 1;//获取Rect的像素列数
	long nHeight = lpRect->bottom - lpRect->top + 1;//获取Rect的像素行数
	long nScanIndex = lpRect->right+1;

	if(nWidth < 0 || nHeight < 0)
	{
		return;
	}

	if(nDataLen < 1)
	{
		fxStep = 0.0f;
	}
	else
	{
		fxStep = nWidth*1.0/(nDataLen-1);//水平每2个点之间的距离
	}
	fyStep = nHeight*1.0/MAX_DATA;//垂直刻度 
	ptSource = new POINT[nDataLen];
//计算数据点的坐标
	x = lpRect->left;
	for(i=0;i<nDataLen;i++)
	{
		y = lpRect->top + (MAX_DATA - pData[i]) * fyStep;
		if(x < lpRect->left)
		{
			x = lpRect->left;
		}
		else if(x > lpRect->right)
		{
			x = lpRect->right;
		}
		if(y < lpRect->top)
		{
			y = lpRect->top;
		}
		else if(y > lpRect->bottom)
		{
			y = lpRect->bottom;
		}
		ptSource[i].x = long(x);
		ptSource[i].y = long(y);
		x = x + fxStep;
	}
//绘图
	if(nDisMode == VECTORS)//线
	{
		POINT* ptDraw;
		ULONG nDrawLen = ULONG(nWidth*2);
		HPEN hPen,hOldPen;
		hPen = CreatePen(PS_SOLID,1,clrRGB);
		hOldPen = (HPEN)SelectObject(hDC,hPen);

		if(fxStep < 1)//当水平两点之间的间距 < 1个像素列: --1个像素列上有多个点
		{
			long nMax=0,nMin=0;
			ULONG nMaxIndex=0,nMinIndex=0,nIndex=0;
			ptDraw = new POINT[nDrawLen];
			j=0;
			for(i=ULONG(lpRect->left);i<=ULONG(lpRect->right);i++)
			{
				nMaxIndex=j;
				nMinIndex=j;
				nMax = ptSource[j].y;
				nMin = ptSource[j].y;
				for(;j<nDataLen;j++)
				{
					if(j == nScanLen)
					{
						nScanIndex = i;
					}
					if(i == ULONG(ptSource[j].x))
					{
						if(ptSource[j].y > nMax)
						{
							nMax = ptSource[j].y;
							nMaxIndex = j;
						}
						else if(ptSource[j].y < nMin)
						{
							nMin = ptSource[j].y;
							nMinIndex = j;
						}
						continue;
					}
					else
					{
						break;
					}
				}
				if(nMinIndex < nMaxIndex)
				{
					ptDraw[nIndex].x = i;
					ptDraw[nIndex].y = nMin;
					ptDraw[nIndex+1].x = i;
					ptDraw[nIndex+1].y = nMax;
				}
				else
				{
					ptDraw[nIndex].x = i;
					ptDraw[nIndex].y = nMax;
					ptDraw[nIndex+1].x = i;
					ptDraw[nIndex+1].y = nMin;
				}
				nIndex+=2;
				if(j >= nDataLen)
				{
					j--;
				}
			}
		}
		else//当水平两点之间的间距 > 1个像素列 :
		{
			nDrawLen = nDataLen;
			ptDraw = ptSource;
		}
		//
		MoveToEx(hDC,ptDraw[0].x,ptDraw[0].y,NULL);
		for(i=1;i<nDrawLen;i++)
		{
			/*(暂时不采用)
			//同1列上的2个点如果相同，则不绘,直接MoveToEx
			if(ptDraw[i-1].x == ptDraw[i].x)
			{
				if(ptDraw[i-1].y == ptDraw[i].y)
				{
					MoveToEx(hDC,ptDraw[i].x,ptDraw[i].y,NULL);
				}
				else
				{
					LineTo(hDC,ptDraw[i].x,ptDraw[i].y);
				}
			}
			else
			{
				LineTo(hDC,ptDraw[i].x,ptDraw[i].y);
			}
			*/
			if(ptDraw[i].x >= nScanIndex && ptDraw[i].x <= nScanIndex+10)
			{
				MoveToEx(hDC,ptDraw[i].x,ptDraw[i].y,NULL);
			}
			else
			{
				LineTo(hDC,ptDraw[i].x,ptDraw[i].y);
			}
		}
		if(fxStep < 1)
		{
			delete ptDraw;
		}
		SelectObject(hDC,hOldPen);
		DeleteObject(hPen);
	}
	else//点
	{
//		SetPixel(hDC,ptSource[0].x,ptSource[0].y,clrRGB);
		for(i=0;i<nDataLen;i++)
		{
			j = i + 1;
			if(j < nDataLen)
			{
				for(;j < nDataLen;j++)
				{
					if(ptSource[i].x == ptSource[j].x)//横坐标相等(即在同一列上)
					{
						if(ptSource[i].y == ptSource[j].y)
						{
							break;//如果纵坐标相同，则直接跳到下一个点(i++)
						}
						else
						{
							continue;//如果纵坐标不相同，则跳到下一个横坐标相同的点(j++)
						}
					}
					else
					{
						if(i < nScanLen || i > nScanLen+200)
						{
							SetPixel(hDC,ptSource[i].x,ptSource[i].y,clrRGB);
						}
						break;
					}
				}
			}
			else
			{
				if(i < nScanLen || i > nScanLen+200)
				{
					SetPixel(hDC,ptSource[i].x,ptSource[i].y,clrRGB);
				}
			}
		}
	}
	delete ptSource;
}

void DrawWaveRoll(HDC hDC,CONST RECT* lpRect,WORD* pData,ULONG nDataLen,double fHTriggerPos,COLORREF clrRGB,USHORT nDisMode)
{
	ULONG i;
	ULONG j = 0;
	POINT* ptSource;
	double x,y,fxStep,fyStep;
	long nWidth = lpRect->right - lpRect->left + 1;//获取Rect的像素列数
	long nHeight = lpRect->bottom - lpRect->top + 1;//获取Rect的像素行数

	fxStep = nWidth*1.0/(nDataLen-1);//水平每2个点之间的距离
	fyStep = nHeight*1.0/MAX_DATA;//垂直刻度 
	ptSource = new POINT[nDataLen];
	ULONG nTriggerPoint = ULONG(fHTriggerPos * nDataLen);
	if(nTriggerPoint >= nDataLen)
	{
		nTriggerPoint = nDataLen - 1;
	}
//计算数据点的坐标
	//触发点左边数据
	x = lpRect->left + nWidth * fHTriggerPos;
	for(i=nTriggerPoint;i>=0 && i<nDataLen;i--)
	{
		y = lpRect->top + (MAX_DATA - pData[i]) * fyStep;
		if(x < lpRect->left)
		{
			x = lpRect->left;
		}
		else if(x > lpRect->right)
		{
			x = lpRect->right;
		}
		if(y < lpRect->top)
		{
			y = lpRect->top;
		}
		else if(y > lpRect->bottom)
		{
			y = lpRect->bottom;
		}
		ptSource[i].x = long(x);
		ptSource[i].y = long(y);
		x = x - fxStep;
	}
	//触发点右边数据
	x = lpRect->left + nWidth * fHTriggerPos;
	for(i=nTriggerPoint;i<nDataLen;i++)
	{
		y = lpRect->top + (MAX_DATA - pData[i]) * fyStep;
		if(x < lpRect->left)
		{
			x = lpRect->left;
		}
		else if(x > lpRect->right)
		{
			x = lpRect->right;
		}
		if(y < lpRect->top)
		{
			y = lpRect->top;
		}
		else if(y > lpRect->bottom)
		{
			y = lpRect->bottom;
		}
		ptSource[i].x = long(x);
		ptSource[i].y = long(y);
		x = x + fxStep;
	}
	if(nDisMode == VECTORS)//线
	{
		POINT* ptDraw;
		long nMax=0,nMin=0;
		ULONG nMaxIndex=0,nMinIndex=0,nIndex=0;
		ptDraw = new POINT[nWidth*2];
		for(i=ULONG(lpRect->left);i<=ULONG(lpRect->right);i++)
		{
			nMaxIndex=j;
			nMinIndex=j;
			nMax = ptSource[j].y;
			nMin = ptSource[j].y;
			for(;j<nDataLen;j++)
			{
				if(i == ULONG(ptSource[j].x))
				{
					if(ptSource[j].y > nMax)
					{
						nMax = ptSource[j].y;
						nMaxIndex = j;
					}
					else if(ptSource[j].y < nMin)
					{
						nMin = ptSource[j].y;
						nMinIndex = j;
					}
					continue;
				}
				else
				{
					break;
				}
			}
			if(nMinIndex < nMaxIndex)
			{
				ptDraw[nIndex].x = i;
				ptDraw[nIndex].y = nMin;
				ptDraw[nIndex+1].x = i;
				ptDraw[nIndex+1].y = nMax;
			}
			else
			{
				ptDraw[nIndex].x = i;
				ptDraw[nIndex].y = nMax;
				ptDraw[nIndex+1].x = i;
				ptDraw[nIndex+1].y = nMin;
			}
			nIndex+=2;
		}
		//绘图
		HPEN hPen,hOldPen;
		hPen = CreatePen(PS_SOLID,1,clrRGB);
		hOldPen = (HPEN)SelectObject(hDC,hPen);
		MoveToEx(hDC,ptDraw[0].x,ptDraw[0].y,NULL);
		for(i=1;i<ULONG(nWidth*2);i++)
		{
			/*(暂时不采用)
			//同1列上的2个点如果相同，则不绘,直接MoveToEx
			if(ptDraw[i-1].x == ptDraw[i].x)
			{
				if(ptDraw[i-1].y == ptDraw[i].y)
				{
					MoveToEx(hDC,ptDraw[i].x,ptDraw[i].y,NULL);
				}
				else
				{
					LineTo(hDC,ptDraw[i].x,ptDraw[i].y);
				}
			}
			else
			{
				LineTo(hDC,ptDraw[i].x,ptDraw[i].y);
			}
			*/
			LineTo(hDC,ptDraw[i].x,ptDraw[i].y);
		}
		delete ptDraw;
		SelectObject(hDC,hOldPen);
		DeleteObject(hPen);
	}
	else//点
	{
//		SetPixel(hDC,ptSource[0].x,ptSource[0].y,clrRGB);
		for(i=0;i<nDataLen;i++)
		{
			j = i + 1;
			if(j < nDataLen)
			{
				for(;j < nDataLen;j++)
				{
					if(ptSource[i].x == ptSource[j].x)//横坐标相等(即在同一列上)
					{
						if(ptSource[i].y == ptSource[j].y)
						{
							break;//如果纵坐标相同，则直接跳到下一个点(i++)
						}
						else
						{
							continue;//如果纵坐标不相同，则跳到下一个横坐标相同的点(j++)
						}
					}
					else
					{
						SetPixel(hDC,ptSource[i].x,ptSource[i].y,clrRGB);
						break;
					}
				}
			}
			else
			{
				SetPixel(hDC,ptSource[i].x,ptSource[i].y,clrRGB);
			}
		}
	}
	delete ptSource;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Export..........
DLL_API void WINAPI HTDrawGrid(HDC hDC,int nLeft,int nTop,int nRight,int nBottom,USHORT nHoriGridNum,USHORT nVertGridNum,USHORT nBright,USHORT IsGrid)
{
	RECT Rect;
	HBRUSH hBrush;

	Rect.left = nLeft;
	Rect.top = nTop;
	Rect.right = nRight;
	Rect.bottom = nBottom;

	if(nBright > 255)
	{
		nBright = 255;
	}
	//Draw Grid
	hBrush = CreateSolidBrush(RGB(0,0,0));
	FillRect(hDC,&Rect,hBrush);//Grid BkGround
	DeleteObject(hBrush);
	DrawGridScale(hDC,&Rect,RGB(nBright,nBright,nBright),nHoriGridNum,nVertGridNum,IsGrid,TRUE);//Draw grid Scale
	//Draw Grid Border
	RECT rcBorder;
	rcBorder.left = Rect.left - 1;
	rcBorder.top = Rect.top - 1;
	rcBorder.right = Rect.right + 2;
	rcBorder.bottom = Rect.bottom + 2;
	DrawEdge(hDC,&rcBorder,EDGE_BUMP,BF_TOPLEFT);
	DrawEdge(hDC,&rcBorder,EDGE_ETCHED,BF_BOTTOMRIGHT);/**/
}

DLL_API void WINAPI HTDrawGridNew(HDC hDC,int nLeft,int nTop,int nRight,int nBottom,USHORT nHoriGridNum,USHORT nVertGridNum,USHORT nBright,USHORT IsGrid)
{
	RECT Rect;
	HBRUSH hBrush;
	BYTE clr;

	Rect.left = nLeft;
	Rect.top = nTop;
	Rect.right = nRight;
	Rect.bottom = nBottom;

	if(nBright > 100)
	{
		nBright = 100;
	}
	clr = nBright * 255 / 100;
	//Draw Grid
	hBrush = CreateSolidBrush(RGB(0,0,0));
	FillRect(hDC,&Rect,hBrush);//Grid BkGround
	DeleteObject(hBrush);
	DrawGridScaleNew(hDC,&Rect,RGB(clr,clr,clr),nHoriGridNum,nVertGridNum,IsGrid,TRUE);//Draw grid Scale
	//Draw Grid Border
	RECT rcBorder;
	rcBorder.left = Rect.left - 1;
	rcBorder.top = Rect.top - 1;
	rcBorder.right = Rect.right + 2;
	rcBorder.bottom = Rect.bottom + 2;
	DrawEdge(hDC,&rcBorder,EDGE_BUMP,BF_TOPLEFT);
	DrawEdge(hDC,&rcBorder,EDGE_ETCHED,BF_BOTTOMRIGHT);/**/
}

DLL_API void WINAPI HTDrawGridARB(HDC hDC,int nLeft,int nTop,int nRight,int nBottom,USHORT nHoriGridNum,USHORT nVertGridNum,USHORT nBright,USHORT IsGrid)
{
	RECT Rect;
//	HBRUSH hBrush;

	Rect.left = nLeft;
	Rect.top = nTop;
	Rect.right = nRight;
	Rect.bottom = nBottom;

	if(nBright > 255)
	{
		nBright = 255;
	}
	//Draw Grid
//	hBrush = CreateSolidBrush(RGB(0,0,0));
//	FillRect(hDC,&Rect,hBrush);//Grid BkGround
//	DeleteObject(hBrush);
	DrawGridScale(hDC,&Rect,RGB(nBright,nBright,nBright),nHoriGridNum,nVertGridNum,IsGrid,TRUE);//Draw grid Scale
	//Draw Grid Border
	RECT rcBorder;
	rcBorder.left = Rect.left - 1;
	rcBorder.top = Rect.top - 1;
	rcBorder.right = Rect.right + 2;
	rcBorder.bottom = Rect.bottom + 2;
	DrawEdge(hDC,&rcBorder,EDGE_BUMP,BF_TOPLEFT);
	DrawEdge(hDC,&rcBorder,EDGE_ETCHED,BF_BOTTOMRIGHT);/**/
}


DLL_API void WINAPI HTDrawGridBorder(HDC hDC,int nLeft,int nTop,int nRight,int nBottom)
{
	RECT Rect;

	Rect.left = nLeft;
	Rect.top = nTop;
	Rect.right = nRight+1;
	Rect.bottom = nBottom+1;

	DrawEdge(hDC,&Rect,EDGE_BUMP,BF_TOPLEFT);
	DrawEdge(hDC,&Rect,EDGE_ETCHED,BF_BOTTOMRIGHT);
}

DLL_API void WINAPI HTDrawPrintGrid(HDC hDC,int nLeft,int nTop,int nRight,int nBottom,USHORT nHoriGridNum,USHORT nVertGridNum,USHORT nBright,USHORT IsGrid)
{
	RECT Rect;
	HBRUSH hBrush;
	HPEN hPen;

	Rect.left = nLeft;
	Rect.top = nTop;
	Rect.right = nRight;
	Rect.bottom = nBottom;

	if(nBright > 255)
	{
		nBright = 255;
	}
	//Draw Grid
	hBrush = CreateSolidBrush(RGB(255,255,255));
	FillRect(hDC,&Rect,hBrush);//Grid BkGround
	DeleteObject(hBrush);
	DrawGridScale(hDC,&Rect,RGB(nBright,nBright,nBright),nHoriGridNum,nVertGridNum,IsGrid,FALSE);//Draw grid Scale
	//Draw Grid Border
	RECT rcBorder;
	rcBorder.left = Rect.left - 1;
	rcBorder.top = Rect.top - 1;
	rcBorder.right = Rect.right + 2;
	rcBorder.bottom = Rect.bottom + 2;
	hPen = CreatePen(PS_SOLID,1,RGB(nBright,nBright,nBright));
	HPEN hOldPen = (HPEN)SelectObject(hDC,hPen);
	MoveToEx(hDC,rcBorder.left,rcBorder.top,NULL);
	LineTo(hDC,rcBorder.right,rcBorder.top);
	LineTo(hDC,rcBorder.right,rcBorder.bottom);
	LineTo(hDC,rcBorder.left,rcBorder.bottom);
	LineTo(hDC,rcBorder.left,rcBorder.top);
	SelectObject(hDC,hOldPen);
	DeleteObject(hPen);
}

DLL_API void WINAPI HTDrawPrintGridBorder(HDC hDC,RECT Rect,USHORT nBright)
{
	HPEN hPen;
	RECT rcBorder;
	rcBorder.left = Rect.left - 1;
	rcBorder.top = Rect.top - 1;
	rcBorder.right = Rect.right + 2;
	rcBorder.bottom = Rect.bottom + 2;
	hPen = CreatePen(PS_SOLID,1,RGB(nBright,nBright,nBright));
	HPEN hOldPen = (HPEN)SelectObject(hDC,hPen);
	MoveToEx(hDC,rcBorder.left,rcBorder.top,NULL);
	LineTo(hDC,rcBorder.right,rcBorder.top);
	LineTo(hDC,rcBorder.right,rcBorder.bottom);
	LineTo(hDC,rcBorder.left,rcBorder.bottom);
	LineTo(hDC,rcBorder.left,rcBorder.top);
	SelectObject(hDC,hOldPen);
	DeleteObject(hPen);
}

#ifdef GDIPLUS
DLL_API void WINAPI HTDrawKnob(HDC hDC,WORD nTotalScale,COLORREF color,WORD nScaleValue)
{
	if(hDC == NULL)
	{
		return;
	}
	short Size,deep;
	double p;
	int left,right,width,height,bottom,top;
	Size = 87;//100;
	left = 9;	
	top = 9;
	right = 0;
	bottom = 0;
	width = Size;
	height = Size;	
	Color CHColor(GetRValue(color),GetGValue(color),GetBValue(color));
	Color CHColorDarker(UCHAR(GetRValue(color)*0.6),UCHAR(GetGValue(color)*0.6),UCHAR(GetBValue(color)*0.6));
	SolidBrush CHColorBrush(CHColor),CHColorBrushDarker(CHColorDarker);
	Graphics     graphics(hDC);	
	graphics.SetSmoothingMode(SmoothingModeAntiAlias);
	Pen          pen(Color(255,0,0,0));//黑色画笔
	//定义四个矩形
	deep=3;
	Rect OutRect(left-deep,top-deep,width-2*(left-deep),height-2*(top-deep));
	Rect MidRect(left,top,width-2*left,height-2*top);
	deep=6;
	Rect InnerRect(left+deep,top+deep,width-2*(left+deep),height-2*(top+deep));
	deep=9;
	Rect InnerCoverRect(left+deep,top+deep,width-2*(left+deep),height-2*(top+deep));
	deep=28;
	Rect CoverRect(left+deep,top+deep,width-2*(left+deep),height-2*(top+deep));
	//定义四个矩形
	LinearGradientBrush LineB(
		MidRect,
		Color(255,220,220,220),
		Color(255,110,110,110),
		LinearGradientModeForwardDiagonal

		);
	
	deep=10;
	//左边脚
	Pie(hDC,left-deep,top-deep,width-left+deep,height-top+deep,0,height,1,height);
	//右边脚
	Pie(hDC,left-deep,top-deep,width-left+deep,height-top+deep,width,height+1,width,height);
	//画四个同心圆弧
	graphics.DrawArc(&pen,OutRect,135,270);

	graphics.DrawEllipse(&pen, MidRect);
	graphics.FillEllipse(&LineB,MidRect);

	graphics.DrawEllipse(&pen, InnerRect);
	graphics.FillEllipse(&CHColorBrushDarker,InnerRect);

	pen.SetColor(CHColor);
	graphics.DrawEllipse(&pen, InnerCoverRect);

	graphics.FillEllipse(&CHColorBrush,InnerCoverRect);
	//画四个同心圆弧	

	
	//画指针
	p=(nScaleValue)*1.0/(nTotalScale-1);
	if(p<0) p=0; else if(p>1) p=1;
	double dAngel=(0.25+1.5*p)*PI;
	pen.SetColor(Color(255,233,233,200));
	pen.SetWidth(2);
	deep=(int)((float)width/3.5); 
	double xOffset=sin(dAngel)*deep;
	double yOffset=cos(dAngel)*deep;
	PointF p1(float(width/2.0),float(height/2.0)),p2(float(width/2.0+xOffset),float(height/2.0+yOffset));	
	graphics.DrawLine(&pen,p1,p2);
	pen.SetColor(CHColor);
	graphics.DrawEllipse(&pen, CoverRect);
	graphics.FillEllipse(&CHColorBrush,CoverRect);
}
#else
DLL_API void WINAPI HTDrawKnob(HDC hDC,WORD nTotalScale,COLORREF color,WORD nScaleValue)
{
	HBRUSH NewBrush,OldBrush;
	HPEN NewPen,OldPen;
	short deep,value,Size;
	double cal_val;
	short left,right,width,height,bottom,top;

	left = 9;
	right = 0;
	top = 9;
	bottom = 0;
	width = 87;
	height = 85;

	Size = 87;//100;


	if(hDC == NULL)
	{
		return;
	}

	deep=3;
	//最外圆弧
	Arc(hDC,left-deep,top-deep,width-left+deep , height-top+deep,width,height,0,height);
	deep=10;
	//左边脚
	Pie(hDC,left-deep,top-deep,width-left+deep,height-top+deep,0,height,1,height);
	//右边脚
	Pie(hDC,left-deep,top-deep,width-left+deep,height-top+deep,width,height+1,width,height);

	//左上边半圆弧
	NewBrush=CreateSolidBrush(RGB(210,210,210)); 
	OldBrush=(HBRUSH)SelectObject(hDC,NewBrush);
	Chord(hDC,left,top,width-left,height-top,width,0,0,height);
	SelectObject(hDC,OldBrush);
	DeleteObject (NewBrush);
	//右下边半圆弧
	NewBrush=CreateSolidBrush(RGB(180,180,180));
	OldBrush=(HBRUSH)SelectObject(hDC,NewBrush);
	Chord(hDC,left,top,width-left,height-top,0,height,width,0);
	SelectObject(hDC,OldBrush);
	DeleteObject (NewBrush);

//绘制圆圈

	value = (nTotalScale-1-nScaleValue)*Size/(nTotalScale-1);

	NewBrush=CreateSolidBrush(color);
	deep=6;
	OldBrush=(HBRUSH)SelectObject(hDC,NewBrush);
	Chord(hDC,left+deep,top+deep,width-left-deep,height-top-deep,0,0,0,0);

	//画指针
	deep=6;
	if (value<0 ) value=0;
	if (value>Size) value=Size;

	cal_val=(float)(-value)*0.75-12.5;
	cal_val=cal_val*2*PI/Size;

	deep=(int)((float)width/3.5);   
	NewPen=CreatePen(PS_SOLID,2,RGB(233,233,200));
	OldPen=(HPEN)SelectObject(hDC,NewPen);
	MoveToEx(hDC,width/2,height/2,NULL);
    LineTo(hDC ,(int)(sin(cal_val)*deep)+width/2 ,(int)(cos(cal_val)*deep)+height/2);
	SelectObject(hDC,OldPen);
	DeleteObject(NewPen);

	deep=28;
	NewPen=CreatePen(PS_SOLID,1,color);
	OldPen=(HPEN)SelectObject(hDC,NewPen);
	Chord(hDC,left+deep,top+deep,width-left-deep,height-top-deep,0,0,0,0);
	SelectObject(hDC,OldPen);
	DeleteObject(NewPen);
	SelectObject(hDC,OldBrush);
	DeleteObject (NewBrush);
}

#endif
#ifdef GDIPLUS
DLL_API void WINAPI HTDrawEllipse(HDC hDC,int left,int top,int width,int height,COLORREF color)
{	
	if(hDC==NULL) return;
	
	int deep=1;
	Color CHColor(GetRValue(color),GetGValue(color),GetBValue(color));
	Color CHColorDarker(UCHAR(GetRValue(color)*0.6),UCHAR(GetGValue(color)*0.6),UCHAR(GetBValue(color)*0.6));
	SolidBrush CHColorBrush(CHColor),CHColorBrushDarker(CHColorDarker);
	Pen          pen(CHColorDarker);
	Rect OutRect(left,top,width,height);
	Rect InRect(left+deep,top+deep,width-2*(deep),height-2*(deep));

	Graphics     graphics(hDC);
	graphics.SetSmoothingMode(SmoothingModeAntiAlias);
	graphics.DrawEllipse(&pen, OutRect);
	graphics.FillEllipse(&CHColorBrushDarker,OutRect);
	pen.SetColor(CHColor);
	graphics.DrawEllipse(&pen, InRect);
	graphics.FillEllipse(&CHColorBrush,InRect);

}
#else
DLL_API void WINAPI HTDrawEllipse(HDC hDC,int left,int top,int width,int height,COLORREF color)
{	
	HBRUSH fillBrush;
	fillBrush=CreateSolidBrush(color);
	SelectObject(hDC,fillBrush);
	Chord(hDC,left,top,width+left,height+top,0,0,0,0);
}

#endif
//箭头为0点，顺时针排列
DLL_API void WINAPI HTDrawRightPentagon(HDC hDC,int nCenterX,int nCenterY,COLORREF clr)
{
	POINT Points[5];
	POINT ptCenter;
	HBRUSH NewBrush;
	HRGN TRgn;
	ptCenter.x = nCenterX;
	ptCenter.y = nCenterY;
//
	Points[0].x = ptCenter.x+8;
	Points[0].y = ptCenter.y;

	Points[1].x = ptCenter.x+1;
	Points[1].y = ptCenter.y+6;

	Points[2].x = ptCenter.x-9;
	Points[2].y = ptCenter.y+6;

	Points[3].x = ptCenter.x-9;
	Points[3].y = ptCenter.y-5;

	Points[4].x = ptCenter.x+1;
	Points[4].y = ptCenter.y-6;
//
	TRgn = CreatePolygonRgn(Points,5,ALTERNATE);
	NewBrush = CreateSolidBrush(clr);
	//填充
	FillRgn(hDC,TRgn,NewBrush);
	//
	DeleteObject(NewBrush);
	DeleteObject(TRgn);
}

//画方向向左的五边形
DLL_API void WINAPI HTDrawLeftPentagon(HDC hDC,int nCenterX,int nCenterY,COLORREF clr)
{
	POINT Points[5];
	POINT ptCenter;
	HBRUSH NewBrush;
	HRGN TRgn;
	ptCenter.x = nCenterX;
	ptCenter.y = nCenterY;
//
	Points[0].x = ptCenter.x-8;
	Points[0].y = ptCenter.y;

	Points[1].x = ptCenter.x-3;
	Points[1].y = ptCenter.y-5;

	Points[2].x = ptCenter.x+9;
	Points[2].y = ptCenter.y-5;

	Points[3].x = ptCenter.x+9;
	Points[3].y = ptCenter.y+6;

	Points[4].x = ptCenter.x-3;
	Points[4].y = ptCenter.y+6;
//
	TRgn = CreatePolygonRgn(Points,5,ALTERNATE);
	NewBrush = CreateSolidBrush(clr);
	//填充
	FillRgn(hDC,TRgn,NewBrush);
	//
	DeleteObject(NewBrush);
	DeleteObject(TRgn);
}

//画方向向上的五边形
DLL_API void WINAPI HTDrawTopPentagon(HDC hDC,int nCenterX,int nCenterY,COLORREF clr)
{
	POINT Points[5];
	POINT ptCenter;
	HBRUSH NewBrush;
	HRGN TRgn;
	ptCenter.x = nCenterX;
	ptCenter.y = nCenterY;
//
	Points[0].x = ptCenter.x;
	Points[0].y = ptCenter.y-9;

	Points[1].x = ptCenter.x+6;
	Points[1].y = ptCenter.y-3;

	Points[2].x = ptCenter.x+6;
	Points[2].y = ptCenter.y+8;

	Points[3].x = ptCenter.x-5;
	Points[3].y = ptCenter.y+8;

	Points[4].x = ptCenter.x-5;
	Points[4].y = ptCenter.y-3;
//
	TRgn = CreatePolygonRgn(Points,5,ALTERNATE);
	NewBrush = CreateSolidBrush(clr);
	//填充
	FillRgn(hDC,TRgn,NewBrush);
	//
	DeleteObject(NewBrush);
	DeleteObject(TRgn);
}
//画方向向下的五边形
DLL_API void WINAPI HTDrawBottomPentagon(HDC hDC,int nCenterX,int nCenterY,COLORREF clr)
{
	POINT Points[5];
	POINT ptCenter;
	HBRUSH NewBrush;
	HRGN TRgn;
	ptCenter.x = nCenterX;
	ptCenter.y = nCenterY;
//
	Points[0].x = ptCenter.x;
	Points[0].y = ptCenter.y + 9;

	Points[1].x = ptCenter.x - 5;
	Points[1].y = ptCenter.y + 3;

	Points[2].x = ptCenter.x - 5;
	Points[2].y = ptCenter.y - 6;

	Points[3].x = ptCenter.x + 6;
	Points[3].y = ptCenter.y - 6;

	Points[4].x = ptCenter.x + 6;
	Points[4].y = ptCenter.y + 3;
//
	TRgn = CreatePolygonRgn(Points,5,ALTERNATE);
	NewBrush = CreateSolidBrush(clr);
	//填充
	FillRgn(hDC,TRgn,NewBrush);
	//
	DeleteObject(NewBrush);
	DeleteObject(TRgn);
}

//画藕荷
DLL_API void WINAPI HTDrawCouplingImage(HDC hDC,int nLeftTopX,int nLeftTopY,COLORREF clr,USHORT nCoupling)
{
	int i;
	POINT Point[16];
	HPEN hOldPen;
	HPEN hPen;
	
	hPen = CreatePen(PS_SOLID,1,clr);
	hOldPen = (HPEN)SelectObject(hDC,hPen);
	if(nCoupling == DC)//直流
	{
		Point[0].x = nLeftTopX;
		Point[0].y = nLeftTopY;
		for(i=1;i<4;i++)//上面4个点
		{
			Point[i].x = Point[i-1].x+3;
			Point[i].y = Point[i-1].y;
		}
		Point[4].x = Point[0].x;
		Point[4].y = Point[0].y+4;
		Point[5].x = Point[3].x+1;
		Point[5].y = Point[3].y+4;
		for(i=0;i<4;i++)
		{
			SetPixel(hDC,Point[i].x,Point[i].y,clr);
		}
		MoveToEx(hDC,Point[4].x,Point[4].y,(LPPOINT)NULL);
		LineTo(hDC,Point[5].x,Point[5].y);
	}
	else if(nCoupling == AC)//交流
	{
		Point[0].x = nLeftTopX;
		Point[0].y = nLeftTopY+2;
		Point[1].x = Point[0].x;
		Point[1].y = Point[0].y-2;
		Point[2].x = Point[1].x+2;
		Point[2].y = Point[1].y-2;
		Point[3].x = Point[2].x+2;
		Point[3].y = Point[1].y;
		Point[4].x = Point[3].x;
		Point[4].y = Point[0].y;
		Point[5].x = Point[4].x+2;
		Point[5].y = Point[4].y+2;
		Point[6].x = Point[5].x+2;
		Point[6].y = Point[4].y;
		Point[7].x = Point[6].x;
		Point[7].y = Point[3].y-1;
		for(i=1;i<8;i++)
		{
			MoveToEx(hDC,Point[i-1].x,Point[i-1].y,(LPPOINT)NULL);
			LineTo(hDC,Point[i].x,Point[i].y);
		}
	}
	else//接地
	{
		Point[0].x = nLeftTopX;
		Point[0].y = nLeftTopY+1;
		Point[1].x = Point[0].x+5;
		Point[1].y = Point[0].y;
		Point[2].x = Point[1].x+6;
		Point[2].y = Point[0].y;
		Point[3].x = Point[1].x;
		Point[3].y = Point[1].y-5;
		Point[4].x = Point[0].x+2;
		Point[4].y = Point[0].y+3;
		Point[5].x = Point[4].x+7;
		Point[5].y = Point[4].y;

		MoveToEx(hDC,Point[0].x,Point[0].y,(LPPOINT)NULL);
		LineTo(hDC,Point[2].x,Point[2].y);
		MoveToEx(hDC,Point[1].x,Point[1].y,(LPPOINT)NULL);
		LineTo(hDC,Point[3].x,Point[3].y);
		MoveToEx(hDC,Point[4].x,Point[4].y,(LPPOINT)NULL);
		LineTo(hDC,Point[5].x,Point[5].y);
	}
	SelectObject(hDC,hOldPen);
	DeleteObject(hPen);
}

//画边沿触发(上升沿，下降沿)
DLL_API void WINAPI HTDrawEdgeSlope(HDC hDC,int nCenterX,int nCenterY,COLORREF clr, USHORT nSlope)
{
	int i;
	POINT Triangle[3],EagePoint[4];

	HRGN hRgn;
	HPEN hOldPen;
	HPEN hPen;
	HBRUSH hBrush;

	hPen = CreatePen(PS_SOLID,1,clr);
	hBrush = CreateSolidBrush(clr);
	hOldPen = (HPEN)SelectObject(hDC,hPen);

	if(nSlope == RISE)
	{	//上升沿
		EagePoint[0].x = nCenterX - 5;
		EagePoint[0].y = nCenterY + 8;
		EagePoint[1].x = EagePoint[0].x + 5;
		EagePoint[1].y = EagePoint[0].y;
		EagePoint[2].x = EagePoint[1].x;
		EagePoint[2].y = EagePoint[1].y - 16;
		EagePoint[3].x = EagePoint[2].x + 5;
		EagePoint[3].y = EagePoint[2].y;
		
		Triangle[0].x = nCenterX;
		Triangle[0].y = nCenterY-3;
		Triangle[1].x = Triangle[0].x - 4;
		Triangle[1].y = Triangle[0].y + 5;
		Triangle[2].x = Triangle[0].x + 4;
		Triangle[2].y = Triangle[1].y;
	}
	else
	{	//下降沿
		EagePoint[0].x = nCenterX - 5;
		EagePoint[0].y = nCenterY - 8;
		EagePoint[1].x = EagePoint[0].x + 5;
		EagePoint[1].y = EagePoint[0].y;
		EagePoint[2].x = EagePoint[1].x;
		EagePoint[2].y = EagePoint[1].y + 16;
		EagePoint[3].x = EagePoint[2].x + 5;
		EagePoint[3].y = EagePoint[2].y;
		
		Triangle[0].x = nCenterX;
		Triangle[0].y = nCenterY + 3;
		Triangle[1].x = Triangle[0].x - 3;
		Triangle[1].y = Triangle[0].y - 4;
		Triangle[2].x = Triangle[0].x + 4;
		Triangle[2].y = Triangle[1].y;
	}

	for(i=1;i<4;i++)
	{
		MoveToEx(hDC,EagePoint[i-1].x,EagePoint[i-1].y,NULL);
		LineTo(hDC,EagePoint[i].x,EagePoint[i].y);
	}
	hRgn = CreatePolygonRgn(Triangle,3,ALTERNATE);
	FillRgn(hDC,hRgn,hBrush);

	SelectObject(hDC,hOldPen);
	DeleteObject(hPen);
	DeleteObject(hBrush);
	DeleteObject(hRgn);
}

//画脉冲触发(正脉宽，负脉宽)
DLL_API void WINAPI HTDrawPulseWidth(HDC hDC,int nCenterX,int nCenterY,COLORREF clr,USHORT nPW)
{
	int i;
	POINT PulsePoint[6];
	HPEN hPen;
	HPEN hOldPen;

	hPen = CreatePen(PS_SOLID,1,clr);
	hOldPen = (HPEN)SelectObject(hDC,hPen);

	if(nPW == 1)//正脉宽
	{
		PulsePoint[0].x = nCenterX - 7;
		PulsePoint[0].y = nCenterY + 8;
		PulsePoint[1].x = PulsePoint[0].x + 4;
		PulsePoint[1].y = PulsePoint[0].y;
		PulsePoint[2].x = PulsePoint[1].x;
		PulsePoint[2].y = PulsePoint[1].y - 14;
		PulsePoint[3].x = PulsePoint[2].x + 6;
		PulsePoint[3].y = PulsePoint[2].y;
		PulsePoint[4].x = PulsePoint[3].x;
		PulsePoint[4].y = PulsePoint[3].y + 14;
		PulsePoint[5].x = PulsePoint[4].x + 5;
		PulsePoint[5].y = PulsePoint[4].y;
	}
	else//负脉宽
	{
		PulsePoint[0].x = nCenterX - 7;
		PulsePoint[0].y = nCenterY - 6;
		PulsePoint[1].x = PulsePoint[0].x + 4;
		PulsePoint[1].y = PulsePoint[0].y;
		PulsePoint[2].x = PulsePoint[1].x;
		PulsePoint[2].y = PulsePoint[1].y + 14;
		PulsePoint[3].x = PulsePoint[2].x + 6;
		PulsePoint[3].y = PulsePoint[2].y;
		PulsePoint[4].x = PulsePoint[3].x;
		PulsePoint[4].y = PulsePoint[3].y - 14;
		PulsePoint[5].x = PulsePoint[4].x + 5;
		PulsePoint[5].y = PulsePoint[4].y;
	}

	for(i=1;i<6;i++)
	{
		MoveToEx(hDC,PulsePoint[i-1].x,PulsePoint[i-1].y,NULL);
		LineTo(hDC,PulsePoint[i].x,PulsePoint[i].y);
	}
	SelectObject(hDC,hOldPen);
	DeleteObject(hPen);
}

//输出自定义文本
DLL_API void WINAPI HTDrawDefineText(HDC hDC,short nFontHeight,LPCTSTR lpszFontType,int nBkMode,LPCTSTR lpszStr,int nCount,int x,int y)//nBkMode: TRANSPARENT: 1; OPAQUE: 2;
{
	HFONT hFont;
	HFONT hDefFont;
	int nOldBkMode;
	hFont = CreateFont(nFontHeight,               // nHeight
						   0,                         // nWidth
						   0,                         // nEscapement
						   0,                         // nOrientation
						   FW_NORMAL,                 // nWeight
						   FALSE,                     // bItalic
						   FALSE,                     // bUnderline
						   0,                         // cStrikeOut
						   ANSI_CHARSET,              // nCharSet
						   OUT_DEFAULT_PRECIS,        // nOutPrecision
						   CLIP_DEFAULT_PRECIS,       // nClipPrecision
						   DEFAULT_QUALITY,           // nQuality
						   DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
						   lpszFontType);            // lpszFacename
	
	// Do something with the font just created...
	hDefFont = (HFONT)SelectObject(hDC,hFont);
	nOldBkMode = SetBkMode(hDC,nBkMode);

	TextOut(hDC,x,y,lpszStr,nCount);

	SelectObject(hDC,hDefFont);
	SetBkMode(hDC,nOldBkMode);
	// Done with the font. Delete the font object.
	DeleteObject(hFont);
}
/*
DLL_API void WINAPI HTDrawWaveLineYT(HDC hDC,CONST RECT* lpRect,WORD* pData,ULONG nDataLen,COLORREF clrRGB)
{
	DrawWaveYT(hDC,lpRect,pData,nDataLen,clrRGB,1);
}

DLL_API void WINAPI HTDrawWavePointYT(HDC hDC,CONST RECT* lpRect,WORD* pData,ULONG nDataLen,COLORREF clrRGB)
{
	DrawWaveYT(hDC,lpRect,pData,nDataLen,clrRGB,0);
}
*/

DLL_API void WINAPI HTDrawWaveInYTNew(HDC hDC,			//绘图句柄
									  RECT Rect,			//绘图 Rect
									  COLORREF clrRGB,		//颜色
									  USHORT nDisType,		//点或线
									  short* pSrcData,		//源数据
									  ULONG nSrcDataLen,	//源数据长度
									  ULONG nDisDataLen,	//显示数据长度
									  __int64 nCenterData,	//显示在屏幕中间的索引值
									  USHORT nDisLeverPos,	//显示垂直零电平位置
									  double dbHorizontal,	//水平放大倍数
									  double dbVertical,	//垂直放大倍数
									  USHORT nYTFormat,		//Normal/Scan/Roll
									  ULONG nScanLen		//Scan
									  )
{
	__int64 i = 0;
	__int64 nRefData = nCenterData;
	ULONG nLeftDataNum = nDisDataLen / 2;//显示数据的左一半数据
	ULONG nRightDataNum = nDisDataLen - nLeftDataNum;//显示数据的右一半数据(触发点计算在右边数据个数中,所以右边数据会多一个点)

	if(nRefData >= (nSrcDataLen + nLeftDataNum))//整个波形都已经移动至屏幕左侧之外
	{
		//屏幕上无数据
	}
	else if((nRefData + nRightDataNum) <= 0)//整个波形都已经移动至屏幕右侧之外
	{
		//屏幕上无数据
	}
	else//屏幕上有数据
	{
		HPEN hPen,hOldPen;
		double xPre,yPre,xCur,yCur;
		double xRef;//参考点x坐标
		long nWidth = Rect.right - Rect.left + 1;//获取Rect的像素列数
		long nHeight = Rect.bottom - Rect.top + 1;//获取Rect的像素行数
		double dbX = nWidth * 1.0 / nDisDataLen / dbHorizontal;//数据之间的水平间隔象素数
		double dbY = nHeight * 1.0 / MAX_DATA / dbVertical;//垂直刻度间隔象素数
		
		if(nWidth < 0)
			nWidth = 0;

		if(nHeight < 0)
			nHeight = 0;

		hPen = CreatePen(PS_SOLID,1,clrRGB);
		hOldPen = (HPEN)SelectObject(hDC,hPen);
		//求出参考点
		xRef = Rect.left + nWidth / 2.0;
		
		if(nRefData < 0)//波形只存在于屏幕中间右半部分
		{
			xPre = xRef + (-nRefData) * dbX;
			yPre = Rect.top + (nDisLeverPos - pSrcData[0]) * dbY;
			MoveToEx(hDC,UserRound(xPre),UserRound(yPre),NULL);
			nRefData += nRightDataNum;
			for(i = 1; i < nRefData;i++)
			{
				xCur = xPre + dbX;
				yCur = Rect.top + (nDisLeverPos - pSrcData[i]) * dbY;
				LineTo(hDC,UserRound(xCur),UserRound(yCur));
				xPre = xCur;
				yPre = yCur;
			}
		}
		else if(nRefData >= nSrcDataLen)//波形只存在于屏幕中间左半部分
		{
			xPre = xRef - (nRefData - nSrcDataLen) * dbX;
			yPre = Rect.top + (nDisLeverPos - pSrcData[nSrcDataLen - 1]) * dbY;
			MoveToEx(hDC,UserRound(xPre),UserRound(yPre),NULL);
			nRefData = nLeftDataNum - (nRefData - nSrcDataLen);
			for(i = 1; i < nRefData; i++)
			{
				xCur = xPre - dbX;
				yCur = Rect.top + (nDisLeverPos - pSrcData[nSrcDataLen - 1 - i]) * dbY;
				MoveToEx(hDC,UserRound(xCur),UserRound(yCur),NULL);
				LineTo(hDC,UserRound(xPre),UserRound(yPre));
				xPre = xCur;
				yPre = yCur;
			}
		}
		else //if(nRefData >= 0 && nRefData < nSrcDataLen)//参考点存在于数据数组中
		{
			xPre = xRef;
			yPre = Rect.top + (nDisLeverPos - pSrcData[nRefData]) * dbY;
			//参考点左
			for(i = 1; i < nLeftDataNum; i++)
			{
				xCur = xPre - dbX;
				if(nRefData >= i)
				{
					yCur = Rect.top + (nDisLeverPos - pSrcData[nRefData - i]) * dbY;
				}
				else
				{
					break;
				}
				MoveToEx(hDC,UserRound(xCur),UserRound(yCur),NULL);
				LineTo(hDC,UserRound(xPre),UserRound(yPre));
				xPre = xCur;
				yPre = yCur;
			}
			//参考点右
			xPre = xRef;
			yPre = Rect.top + (nDisLeverPos - pSrcData[nRefData]) * dbY;
			MoveToEx(hDC,UserRound(xPre),UserRound(yPre),NULL);
			for(i = 1; i < nRightDataNum;i++)
			{
				xCur = xPre + dbX;
				if((nRefData + i) < nSrcDataLen)
				{
					yCur = Rect.top + (nDisLeverPos - pSrcData[nRefData + i]) * dbY;
					LineTo(hDC,UserRound(xCur),UserRound(yCur));
					xPre = xCur;
					yPre = yCur;
				}
				else
				{
					break;
				}
				
			}
		}
		SelectObject(hDC,hOldPen);
		DeleteObject(hPen);
	}
}

DLL_API void WINAPI HTDrawWaveInYTVB(HDC hDC,				//绘图句柄
									  int left,
									  int top,
									  int right,
									  int bottom,
									  USHORT R,
									  USHORT G,
									  USHORT B,
									  USHORT nDisType,		//点或线
									  short* pSrcData,		//源数据
									  ULONG nSrcDataLen,	//源数据长度
									  ULONG nDisDataLen,	//显示数据长度
									  ULONG nCenterData,	//显示在屏幕中间的数据的索引值(在源数据中的索引值)
									  USHORT nDisLeverPos,	//显示垂直零电平位置
									  double dbHorizontal,	//水平放大倍数
									  double dbVertical,	//垂直放大倍数
									  USHORT nYTFormat,		//Normal/Scan/Roll
									  ULONG nScanLen		//Scan
									  )

{
	RECT Rect;
	Rect.left = left;
	Rect.top = top;
	Rect.right = right;
	Rect.bottom = bottom;//(left,top,right,bottom);
	COLORREF clrRGB;
	clrRGB = RGB(R,G,B);

	HTDrawWaveInYT(hDC,				//绘图句柄
				  Rect,			//绘图 Rect
				  clrRGB,		//颜色
				  nDisType,		//点或线
				  pSrcData,		//源数据
				  nSrcDataLen,	//源数据长度
				  nDisDataLen,	//显示数据长度
				  nCenterData,	//显示在屏幕中间的数据的索引值(在源数据中的索引值)
				  nDisLeverPos,	//显示垂直零电平位置
				  dbHorizontal,	//水平放大倍数
				  dbVertical,	//垂直放大倍数
				  nYTFormat,		//Normal/Scan/Roll
				  nScanLen		//Scan
				  );
}

DLL_API void WINAPI HTDrawWaveInYT(HDC hDC,				//绘图句柄
								  RECT Rect,			//绘图 Rect
								  COLORREF clrRGB,		//颜色
								  USHORT nDisType,		//点或线
								  short* pSrcData,		//源数据
								  ULONG nSrcDataLen,	//源数据长度
								  ULONG nDisDataLen,	//显示数据长度
								  ULONG nCenterData,	//显示在屏幕中间的数据的索引值(在源数据中的索引值)
								  USHORT nDisLeverPos,	//显示垂直零电平位置
								  double dbHorizontal,	//水平放大倍数
								  double dbVertical,	//垂直放大倍数
								  USHORT nYTFormat,		//Normal/Scan/Roll
								  ULONG nScanLen		//Scan
								  )
{
	ULONG i = 0,j = 0,nLDataNum = 0,nRDataNum = 0;
	long nTemp = 0;
	USHORT* pDisData;//加上nDisLeverPos的数据
	RECT rcDraw = Rect;//
	ULONG nOffset = 0;

	nDisDataLen = ULONG(nDisDataLen * dbHorizontal);

	if(nYTFormat != YT_ROLL)
	{
		if(nDisDataLen < 1)//只有1个点
		{
			double fyStep = (Rect.bottom - Rect.top)*1.0/MAX_DATA;//垂直刻度 
			double x = Rect.left + (Rect.right - Rect.left)/2.0;
			double y = 0.0;
			nTemp = long( (*(pSrcData+nCenterData) / dbVertical) + (MAX_DATA - nDisLeverPos));
			if(nTemp < MIN_DATA)
			{
				nTemp = MIN_DATA;
			}
			else if(nTemp > MAX_DATA)
			{
				nTemp = MAX_DATA;
			}
			y = Rect.top + (MAX_DATA - nTemp) * fyStep;
			SetPixel(hDC,int(x),int(y),clrRGB);
			return ;
		}
	////.......
		nLDataNum = nDisDataLen / 2;	//左边的数据点数
		nRDataNum = nDisDataLen - nLDataNum;	//右边的数据点数

		if((nCenterData >= nLDataNum) && ((nCenterData+nRDataNum) <= nSrcDataLen))//第1种情况: 波形显示满Rect
		{
			rcDraw = Rect;
			nOffset = nCenterData-nLDataNum;
		}
		else //第2种情况: 波形显示不满Rect(分3种情况--左边不满右边满/左边满右边不满/左右都不满)
		{
			long nWidth = 0;
			double fStep = 0.0;
			nWidth = Rect.right - Rect.left+1;
			if(nWidth < 0)
				nWidth = 0;
			fStep = nWidth * 1.0 / nDisDataLen;
			if((nCenterData < nLDataNum) && ((nCenterData+nRDataNum) >= nSrcDataLen))//左右都不满
			{
				nOffset = 0;
				nDisDataLen = nSrcDataLen;
				//计算绘图Rect
				rcDraw.left = Rect.left + long((nLDataNum - nCenterData) * fStep);
				rcDraw.right = Rect.right - long(((nCenterData + nRDataNum) - nDisDataLen) * fStep);
			}
			else if((nCenterData < nLDataNum) && (nCenterData+nRDataNum) < nSrcDataLen)//左边不满右边满
			{
				nOffset = 0;
				nDisDataLen = nDisDataLen - (nLDataNum - nCenterData);
				//计算绘图Rect
				if(nCenterData == 0)
				{
					rcDraw.left = Rect.left + (Rect.right - Rect.left)/2;
				}
				else
				{
					rcDraw.left = Rect.left + long((nLDataNum - nCenterData) * fStep);
				}
				rcDraw.right = Rect.right;
			}
			else //左边满右边不满
			{
				rcDraw.left = Rect.left;
				nDisDataLen = nDisDataLen - ((nCenterData + nRDataNum) - nSrcDataLen);
				nOffset = nSrcDataLen - nDisDataLen;
				if(nCenterData == nSrcDataLen-1)
				{
					rcDraw.right = Rect.left + (Rect.right - Rect.left)/2;
				}
				else
				{
					rcDraw.right = Rect.right - long(((nCenterData + nRDataNum) - nSrcDataLen) * fStep);
				}
			}
		}
	}
	else//Roll
	{
		double fStep = 0.0;
		long nWidth = Rect.right - Rect.left+1;
		
		if(nWidth < 0)
			nWidth = 0;
		fStep = nWidth * 1.0 / nDisDataLen;
		if(nDisDataLen > nCenterData)
		{
			nOffset = 0;
			nDisDataLen = nCenterData;
			//计算绘图Rect
			rcDraw.left = Rect.right - long(nDisDataLen * fStep);
			rcDraw.right = Rect.right;
		}
		else
		{
			if(nCenterData >= nDisDataLen)
			{
				nOffset = nCenterData - nDisDataLen;
			}
			else
			{
				nOffset = 0;
			}
		}
	}
//获取数据
	pDisData = new USHORT[nDisDataLen];
	for(i = 0; i < nDisDataLen; i++)//
	{
		nTemp = long( (*(pSrcData+nOffset+i) / dbVertical) + (MAX_DATA - nDisLeverPos) );
		if(nTemp < MIN_DATA)
		{
			nTemp = MIN_DATA;
		}
		else if(nTemp > MAX_DATA)
		{
			nTemp = MAX_DATA;
		}
		*(pDisData+i) = USHORT(nTemp);
	}
//

#ifndef MINISCOPE
	if(nDisDataLen < 2 && nDisType == VECTORS)//如果屏幕上不足10个点，则直接画点
	{
		nDisType = DOTS;
	}
#endif
	if(nYTFormat != YT_SCAN || nScanLen == 0)
	{
		nScanLen = nDisDataLen;
	}
	else
	{
		nScanLen = nScanLen - nOffset;
	}
	DrawWaveYT(hDC,&rcDraw,pDisData,nDisDataLen,clrRGB,nDisType,nScanLen);
	delete pDisData;
}

DLL_API void WINAPI Backup1_HTDrawWaveInYT(HDC hDC,RECT Rect,short* pSrcData,ULONG nSrcDataLen,ULONG nDisDataLen,ULONG nCenterData,USHORT nDisLeverPos,USHORT nHTriggerPos,float fVertical,COLORREF clrRGB,USHORT nDisType)
{
	long nTemp = 0;
	ULONG nStartPoint,nSrcIndex = 0;
	USHORT* pDisData;
	RECT rcDraw;
	long nWidth = 0,nHeight = 0;
	double fHTriggerPos = nHTriggerPos / 100.0f;
	double fStep = 0.0f;
	ULONG nPoint = 0;
	ULONG nSrcTriggerPoint = ULONG(nSrcDataLen * fHTriggerPos);
	
	nStartPoint = nCenterData;
	if(nDisDataLen < 1)
	{
		nDisDataLen = 1;
	}
	if(nDisDataLen < 50 && nDisType == VECTORS)//如果屏幕上不足50个点，则直接画点
	{
		nDisType = DOTS;
	}
	if(nDisDataLen <= nSrcDataLen)//当显示的数据点数目 <= 采集的源数据点数 (显示波形的一部分)
	{
		rcDraw = Rect;
	//	nStartPoint = ULONG(fHTriggerPos * (nSrcDataLen - nDisDataLen));//求得最左边的数据
	}
	else//当显示的数据点数目 > 采集的源数据点数 (显示在屏幕Rect中的一小部分)
	{
		if(nDisDataLen/10000.0 > 100000)//当缩小100000倍，限制
		{
			nDisDataLen = 1024000000;
		}
		//
		nWidth = Rect.right - Rect.left;
		nHeight = Rect.bottom - Rect.top;
		fStep = nWidth * 1.0f / nDisDataLen;
		nPoint = nDisDataLen - nSrcDataLen;
		//计算绘图Rect
		rcDraw.left = Rect.left + long(nPoint * fHTriggerPos * fStep);
		rcDraw.top = Rect.top;
		rcDraw.right = Rect.right - long(nPoint * (1.0f - fHTriggerPos) * fStep);
		rcDraw.bottom = Rect.bottom;
		//
	//	nStartPoint = 0;
		nDisDataLen = nSrcDataLen;
	}
	pDisData = new USHORT[nDisDataLen];
	for(ULONG i=0;i<nDisDataLen;i++)
	{
		nSrcIndex = nStartPoint+i;
		if(nSrcIndex < nSrcDataLen)
		{
			nTemp = long(*(pSrcData+nSrcIndex) / fVertical) + (MAX_DATA - nDisLeverPos);
		}
		else
		{
			break;
		}
		if(nTemp < MIN_DATA)
		{
			nTemp = MIN_DATA;
		}
		else if(nTemp > MAX_DATA)
		{
			nTemp = MAX_DATA;
		}
		*(pDisData+i) = USHORT(nTemp);
	}
	if(nStartPoint > nSrcTriggerPoint)//触发点在屏幕左边
	{
		fHTriggerPos = 0.0;
	}
	else if(nStartPoint + nSrcDataLen < nSrcTriggerPoint)//触发点在屏幕右边
	{
		fHTriggerPos = 100.0;
	}
//	DrawWaveYT(hDC,&rcDraw,pDisData,nDisDataLen,fHTriggerPos,clrRGB,nDisType);
	delete pDisData;
}

DLL_API void WINAPI Backup_HTDrawWaveInRoll(HDC hDC,RECT Rect,short* pSrcData,ULONG nSrcDataLen,ULONG nDisDataLen,ULONG nCenterData,USHORT nDisLeverPos,USHORT nHTriggerPos,float fVertical,COLORREF clrRGB,USHORT nDisType)
{
	long nTemp = 0;
	ULONG nStartPoint,nSrcIndex = 0;
	USHORT* pDisData;
	RECT rcDraw;
	long nWidth = 0,nHeight = 0;
	double fHTriggerPos = 1.0;
	double fStep = 0.0f;
	
	nStartPoint = nCenterData;
	if(nDisDataLen < 1)
	{
		nDisDataLen = 1;
	}
	if(nDisDataLen < 50 && nDisType == VECTORS)//如果屏幕上不足100个点，则直接画点
	{
		nDisType = DOTS;
	}
	nWidth = Rect.right - Rect.left;
	nHeight = Rect.bottom - Rect.top;
	fStep = nWidth * 1.0f / DEF_DRAW_DATA_LEN;
	//计算绘图Rect
	rcDraw.left = Rect.right - long(nDisDataLen * fStep);
	rcDraw.top = Rect.top;
	rcDraw.right = Rect.right;
	rcDraw.bottom = Rect.bottom;

	pDisData = new USHORT[nDisDataLen];
	for(ULONG i=0;i<nDisDataLen;i++)
	{
		nSrcIndex = nStartPoint+i;
		if(nSrcIndex < nSrcDataLen)
		{
			nTemp = long(*(pSrcData+nSrcIndex) / fVertical) + (MAX_DATA - nDisLeverPos);
		}
		else
		{
			break;
		}
		if(nTemp < MIN_DATA)
		{
			nTemp = MIN_DATA;
		}
		else if(nTemp > MAX_DATA)
		{
			nTemp = MAX_DATA;
		}
		*(pDisData+i) = USHORT(nTemp);
	}
	DrawWaveRoll(hDC,&rcDraw,pDisData,nDisDataLen,100,clrRGB,nDisType);
	delete pDisData;
}

DLL_API void WINAPI HTDrawWaveInXY(HDC hDC,RECT Rect,short* pXSrcData,short* pYSrcData,USHORT nXDisLeverPos,USHORT nYDisLeverPos,ULONG nDataLen,COLORREF clrRGB)
{
	ULONG i=0;
	RECT rcDraw;
	POINT* ptDisData;
	float xOffset,yOffset;
	rcDraw.left = Rect.left + (Rect.right - Rect.left)/H_GRID_NUM;
	rcDraw.top = Rect.top;
	rcDraw.right = Rect.right - (Rect.right - Rect.left)/H_GRID_NUM;
	rcDraw.bottom = Rect.bottom;

	ptDisData = new POINT[nDataLen];
	xOffset = (rcDraw.right - rcDraw.left)*1.0f/MAX_DATA;
	yOffset = (rcDraw.bottom - rcDraw.top)*1.0f/MAX_DATA;
	for(i=0;i<nDataLen;i++)
	{
		ptDisData[i].x = rcDraw.left + (long((*(pXSrcData+i) + (MAX_DATA-nXDisLeverPos))*xOffset));
		ptDisData[i].y = rcDraw.top + (long((*(pYSrcData+i) + nYDisLeverPos)*yOffset));
		if(ptDisData[i].x < rcDraw.left)
		{
			ptDisData[i].x = rcDraw.left;
		}
		else if(ptDisData[i].x > rcDraw.right)
		{
			ptDisData[i].x = rcDraw.right;
		}
		if(ptDisData[i].y < rcDraw.top)
		{
			ptDisData[i].y = rcDraw.top;
		}
		else if(ptDisData[i].y > rcDraw.bottom)
		{
			ptDisData[i].y = rcDraw.bottom;
		}
	}
	//
	DrawWaveXY(hDC,&rcDraw,ptDisData,nDataLen,clrRGB);

	delete ptDisData;
}

DLL_API void WINAPI HTDrawWaveInXYNew(HDC hDC,RECT Rect,short* pXSrcData,short* pYSrcData,USHORT nXDisLeverPos,USHORT nYDisLeverPos,ULONG nDataLen,COLORREF clrRGB)
{
	ULONG i=0;
	RECT rcDraw;
	POINT* ptDisData;
	float xOffset,yOffset;
	rcDraw = Rect;

	ptDisData = new POINT[nDataLen];
	xOffset = (rcDraw.right - rcDraw.left)*1.0f/MAX_DATA;
	yOffset = (rcDraw.bottom - rcDraw.top)*1.0f/MAX_DATA;
	for(i=0;i<nDataLen;i++)
	{
		ptDisData[i].x = rcDraw.left + (long((*(pXSrcData+i) + (nXDisLeverPos))*xOffset));
		ptDisData[i].y = rcDraw.top + (long((*(pYSrcData+i) + nYDisLeverPos)*yOffset));
		if(ptDisData[i].x < rcDraw.left)
		{
			ptDisData[i].x = rcDraw.left;
		}
		else if(ptDisData[i].x > rcDraw.right)
		{
			ptDisData[i].x = rcDraw.right;
		}
		if(ptDisData[i].y < rcDraw.top)
		{
			ptDisData[i].y = rcDraw.top;
		}
		else if(ptDisData[i].y > rcDraw.bottom)
		{
			ptDisData[i].y = rcDraw.bottom;
		}
	}
	//
	DrawWaveXY(hDC,&rcDraw,ptDisData,nDataLen,clrRGB);

	delete ptDisData;
}

DLL_API void WINAPI HTDrawCursorLine(HDC hDC,RECT Rect,POINT ptFirst,POINT ptSecond,USHORT nType)
{
	if(nType == CUR_NONE || nType == CUR_TRACE)
	{
		return;
	}
//判断第1个点是否在Rect范围内
	if(ptFirst.x < Rect.left)
	{
		ptFirst.x = Rect.left;
	}
	else if(ptFirst.x > Rect.right)
	{
		ptFirst.x = Rect.right;
	}
	if(ptFirst.y < Rect.top)
	{
		ptFirst.y = Rect.top;
	}
	else if(ptFirst.y > Rect.bottom)
	{
		ptFirst.y = Rect.bottom;
	}
//判断第2个点是否在Rect范围内
	if(nType != CUR_TRACE)
	{
		if(ptSecond.x < Rect.left)
		{
			ptSecond.x = Rect.left;
		}
		else if(ptSecond.x > Rect.right)
		{
			ptSecond.x = Rect.right;
		}
		if(ptSecond.y < Rect.top)
		{
			ptSecond.y = Rect.top;
		}
		else if(ptSecond.y > Rect.bottom)
		{
			ptSecond.y = Rect.bottom;
		}
	}
//开始绘图
	HPEN hFirstPen,hSecondPen;
	HPEN hOldPen;
	hFirstPen = CreatePen(PS_SOLID,1,RGB(0,200,255));
	hSecondPen = CreatePen(PS_DOT,1,RGB(0,200,255));
	if(nType == CUR_CROSS)
	{
		//第2个十字线(先)
		hOldPen = (HPEN)SelectObject(hDC,hSecondPen);
		MoveToEx(hDC,Rect.left + 1,ptSecond.y,NULL);
		LineTo(hDC,Rect.right,ptSecond.y);
		MoveToEx(hDC,ptSecond.x,Rect.top+1,NULL);
		LineTo(hDC,ptSecond.x,Rect.bottom);
		//第1个十字线(后)
		SelectObject(hDC,hFirstPen);
		MoveToEx(hDC,Rect.left + 1,ptFirst.y,NULL);
		LineTo(hDC,Rect.right,ptFirst.y);
		MoveToEx(hDC,ptFirst.x,Rect.top+1,NULL);
		LineTo(hDC,ptFirst.x,Rect.bottom);
		SelectObject(hDC,hOldPen);
	}
	else if(nType == CUR_VERTICAL)
	{
		//虚线
		hOldPen = (HPEN)SelectObject(hDC,hSecondPen);
		MoveToEx(hDC,ptSecond.x,Rect.top+1,NULL);
		LineTo(hDC,ptSecond.x,Rect.bottom);
		//实线
		SelectObject(hDC,hFirstPen);
		MoveToEx(hDC,ptFirst.x,Rect.top+1,NULL);
		LineTo(hDC,ptFirst.x,Rect.bottom);
		SelectObject(hDC,hOldPen);
	}
	else if(nType == CUR_HORIZONTAL)
	{
		//虚线
		hOldPen = (HPEN)SelectObject(hDC,hSecondPen);
		MoveToEx(hDC,Rect.left+1,ptSecond.y,NULL);
		LineTo(hDC,Rect.right,ptSecond.y);
		//实线
		SelectObject(hDC,hFirstPen);
		MoveToEx(hDC,Rect.left+1,ptFirst.y,NULL);
		LineTo(hDC,Rect.right,ptFirst.y);
		SelectObject(hDC,hOldPen);
	}
	else
	{
		;
	}
	DeleteObject(hFirstPen);
	DeleteObject(hSecondPen);
}

DLL_API void WINAPI HTDrawCursorTraceLine(HDC hDC,RECT Rect,POINT pt)
{
	HPEN hPen,hOldPen;
	int a = 5,i = 0;
	long x[4],y[2];

	//判断点是否在Rect范围内
	if(pt.x < Rect.left)
	{
		pt.x = Rect.left;
	}
	else if(pt.x > Rect.right)
	{
		pt.x = Rect.right;
	}
	if(pt.y < Rect.top)
	{
		pt.y = Rect.top;
	}
	else if(pt.y > Rect.bottom)
	{
		pt.y = Rect.bottom;
	}

	x[0] = pt.x - a;
	x[1] = pt.x + a;
	x[2] = pt.x - 2*a;
	x[3] = pt.x + 2*a;
	y[0] = pt.y - a;
	y[1] = pt.y + a;
	for(i=0;i<4;i++)
	{
		if(x[i] < Rect.left)
		{
			x[i] = Rect.left;
		}
		else if(x[i] >Rect.right)
		{
			x[i] = Rect.right;
		}
	}
	for(i=0;i<2;i++)
	{
		if(y[i] < Rect.top)
		{
			y[i] = Rect.top;
		}
		else if(y[i] > Rect.bottom)
		{
			y[i] = Rect.bottom;
		}
	}
	hPen = CreatePen(PS_SOLID,1,RGB(0,200,255));
	hOldPen = (HPEN)SelectObject(hDC,hPen);
//上、下直线
	MoveToEx(hDC,pt.x,y[0],NULL);
	LineTo(hDC,pt.x,Rect.top);
	MoveToEx(hDC,pt.x,y[1],NULL);
	LineTo(hDC,pt.x,Rect.bottom);
//方框
	MoveToEx(hDC,x[0],y[0],NULL);
	LineTo(hDC,x[1],y[0]);
	LineTo(hDC,x[1],y[1]);
	LineTo(hDC,x[0],y[1]);
	LineTo(hDC,x[0],y[0]);
//左、右短线
	MoveToEx(hDC,x[0],pt.y,NULL);
	LineTo(hDC,x[2],pt.y);
	MoveToEx(hDC,x[1],pt.y,NULL);
	LineTo(hDC,x[3],pt.y);
//
	SelectObject(hDC,hOldPen);
	DeleteObject(hPen);
}

DLL_API void WINAPI HTDrawAcquireMode(HDC hDC,int x,int y,COLORREF clr,USHORT nMode)
{
	HPEN hPen,hOldPen;
	hPen = CreatePen(PS_SOLID,1,clr);
	hOldPen = (HPEN)SelectObject(hDC,hPen);
	if(nMode == SAMPLE_NORMAL || nMode == SAMPLE_PEAK)
	{
		MoveToEx(hDC,x,y,NULL);
		int a = 0;
		for(int i=1;i<30;i++)
		{
			a = a^1;
			if(i > 8 && i < 17)
			{
				LineTo(hDC,x + i,y - 15 - a);
				if( i == 16)
				{
					LineTo(hDC,x + i,y);
					a = 0;
				}
			}
			else
			{
				LineTo(hDC,x + i,y - a);
				if( i == 8)
				{
					LineTo(hDC,x + i,y - 16);
					a = 1;
				}
			}
		}
		if(nMode == SAMPLE_PEAK)
		{
			MoveToEx(hDC,x + 23,y-1,NULL);
			LineTo(hDC,x + 23,y - 10);
		}
	}
	else if(nMode == SAMPLE_AVG)
	{
		MoveToEx(hDC,x,y,NULL);
		LineTo(hDC,x + 8 ,y);
		LineTo(hDC,x + 8 ,y - 15);
		LineTo(hDC,x + 16 ,y - 15);
		LineTo(hDC,x + 16 ,y);
		LineTo(hDC,x + 30 ,y);
	}
	else
	{
	}
	SelectObject(hDC,hOldPen);
	DeleteObject(hPen);
}

void DrawSquareWave_BACKOK(HDC hDC,CONST RECT* lpRect,WORD* pData,ULONG nDataLen,COLORREF clrRGB,USHORT nDisMode,ULONG nScanLen)
{
	ULONG i;
	ULONG j = 0;
	POINT* ptSource;
	double x,y,fxStep,fyStep;
	long nWidth = lpRect->right - lpRect->left + 1;//获取Rect的像素列数
	long nHeight = lpRect->bottom - lpRect->top + 1;//获取Rect的像素行数
	long nScanIndex = lpRect->right+1;
	if(nDataLen < 1)
	{
		fxStep = 0.0f;
	}
	else
	{
		fxStep = nWidth*1.0/(nDataLen-1);//水平每2个点之间的距离
	}
	fyStep = nHeight*1.0/MAX_DATA;//垂直刻度 
	ptSource = new POINT[nDataLen];
//计算数据点的坐标
	x = lpRect->left;
	for(i=0;i<nDataLen;i++)
	{
		y = lpRect->top + (MAX_DATA - pData[i]) * fyStep;
		if(y < lpRect->top)
		{
			y = lpRect->top;
		}
		else if(y > lpRect->bottom)
		{
			y = lpRect->bottom;
		}
		ptSource[i].x = long(x);
		ptSource[i].y = long(y);
		x = x + fxStep;
		if(x < lpRect->left)
		{
			x = lpRect->left;
		}
		else if(x > lpRect->right)
		{
			x = lpRect->right;
		}
	}
//绘图
	if(nDisMode == VECTORS)//线
	{
		HPEN hPen,hOldPen;
		hPen = CreatePen(PS_SOLID,1,clrRGB);
		hOldPen = (HPEN)SelectObject(hDC,hPen);

		j=0;
		MoveToEx(hDC,ptSource[j].x,ptSource[j].y,NULL);
		j++;
		for(long m=ptSource[j].x;m<=lpRect->right;m++)
		{
			if(j < nDataLen)//还没画完
			{
				while(m == ptSource[j].x)
				{
					if(j+1 >= nDataLen)
					{
						if(ptSource[j].y != ptSource[j-1].y)
						{
							LineTo(hDC,m,ptSource[j-1].y);
						}
						LineTo(hDC,m,ptSource[j].y);
						m=lpRect->right+1;
						break;
					}
					else
					{
						if(ptSource[j].y != ptSource[j-1].y)
						{
							LineTo(hDC,ptSource[j].x,ptSource[j-1].y);
							LineTo(hDC,ptSource[j].x,ptSource[j].y);
							if(ptSource[j].y != ptSource[j+1].y)
							{
								MoveToEx(hDC,ptSource[j].x,ptSource[j-1].y,NULL);
								j++;
								LineTo(hDC,ptSource[j].x,ptSource[j].y);
								m = ptSource[j].x;
							}
						}
						else
						{
							LineTo(hDC,ptSource[j].x,ptSource[j].y);
						}
						j++;
						if(j>= nDataLen)
						{
							m=lpRect->right+1;
							break;
						}
					}
				}
			}
			else
			{
				break;
			}
		}

		SelectObject(hDC,hOldPen);
		DeleteObject(hPen);
	}
	else//点
	{
		for(i=0;i<nDataLen;i++)
		{
			j = i + 1;
			if(j < nDataLen)
			{
				for(;j < nDataLen;j++)
				{
					if(ptSource[i].x == ptSource[j].x)//横坐标相等(即在同一列上)
					{
						if(ptSource[i].y == ptSource[j].y)
						{
							break;//如果纵坐标相同，则直接跳到下一个点(i++)
						}
						else
						{
							continue;//如果纵坐标不相同，则跳到下一个横坐标相同的点(j++)
						}
					}
					else
					{
						if(i < nScanLen || i > nScanLen+200)
						{
							SetPixel(hDC,ptSource[i].x,ptSource[i].y,clrRGB);
						}
						break;
					}
				}
			}
			else
			{
				if(i < nScanLen || i > nScanLen+200)
				{
					SetPixel(hDC,ptSource[i].x,ptSource[i].y,clrRGB);
				}
			}
		}
	}
	delete ptSource;
}

void DrawSquareWave(HDC hDC,CONST RECT* lpRect,WORD* pData,ULONG nDataLen,COLORREF clrRGB,USHORT nDisMode)
{
	ULONG i;
	ULONG j = 0;
	POINT* ptSource;
	double x,y,fxStep,fyStep;
	long nWidth = lpRect->right - lpRect->left + 1;//获取Rect的像素列数
	long nHeight = lpRect->bottom - lpRect->top + 1;//获取Rect的像素行数
	long nScanIndex = lpRect->right+1;
	if(nDataLen < 1)
	{
		fxStep = 0.0f;
	}
	else
	{
		fxStep = nWidth*1.0/nDataLen;//水平每2个点之间的距离
	}
	fyStep = nHeight*1.0/MAX_DATA;//垂直刻度 
	ptSource = new POINT[nDataLen];
//计算数据点的坐标
	x = lpRect->left;
	for(i=0;i<nDataLen;i++)
	{
		y = lpRect->top + (MAX_DATA - pData[i]) * fyStep;
		if(y < lpRect->top)
		{
			y = lpRect->top;
		}
		else if(y > lpRect->bottom)
		{
			y = lpRect->bottom;
		}
		ptSource[i].x = long(x);
		ptSource[i].y = long(y);
		x = x + fxStep;
		if(x < lpRect->left)
		{
			x = lpRect->left;
		}
		else if(x > lpRect->right)
		{
			x = lpRect->right;
		}
	}
//绘图
	if(nDisMode == VECTORS)//线
	{
		HPEN hPen,hOldPen;
		hPen = CreatePen(PS_SOLID,1,clrRGB);
		hOldPen = (HPEN)SelectObject(hDC,hPen);

		j=0;
		MoveToEx(hDC,ptSource[j].x,ptSource[j].y,NULL);
		j++;
		if(j < nDataLen)
		{
			for(long m=ptSource[j].x;m<=lpRect->right;m++)
			{
				if(j < nDataLen)//还没画完
				{
					while(m == ptSource[j].x)
					{
						if(ptSource[j].y != ptSource[j-1].y)
						{
							LineTo(hDC,m,ptSource[j-1].y);
						}
						LineTo(hDC,m,ptSource[j].y);
						j++;
						if(j>= nDataLen)
						{
							if(m < lpRect->right)
							{
								LineTo(hDC,lpRect->right,ptSource[nDataLen-1].y);
							}
							m=lpRect->right+1;
							break;
						}
					}
				}
				else
				{
					break;
				}
			}
		}
		else//只有1个点
		{
			LineTo(hDC,lpRect->right,ptSource[nDataLen-1].y);
		}

		SelectObject(hDC,hOldPen);
		DeleteObject(hPen);
	}
	else//点
	{
		for(i=0;i<nDataLen;i++)
		{
			j = i + 1;
			if(j < nDataLen)
			{
				for(;j < nDataLen;j++)
				{
					if(ptSource[i].x == ptSource[j].x)//横坐标相等(即在同一列上)
					{
						if(ptSource[i].y == ptSource[j].y)
						{
							break;//如果纵坐标相同，则直接跳到下一个点(i++)
						}
						else
						{
							continue;//如果纵坐标不相同，则跳到下一个横坐标相同的点(j++)
						}
					}
					else
					{
						SetPixel(hDC,ptSource[i].x,ptSource[i].y,clrRGB);
						break;
					}
				}
			}
			else
			{
				SetPixel(hDC,ptSource[i].x,ptSource[i].y,clrRGB);
			}
		}
	}
	delete ptSource;
}

DLL_API void WINAPI HTDrawSquareWaveInYT(HDC hDC,			//绘图句柄
										  RECT Rect,			//绘图 Rect
										  COLORREF clrRGB,		//颜色
										  USHORT nDisType,		//点或线
										  short* pSrcData,		//源数据
										  ULONG nSrcDataLen,	//源数据长度
										  ULONG nDisDataLen,	//显示数据长度
										  ULONG nCenterData,	//显示在屏幕中间的数据的索引值(在源数据中的索引值)
										  USHORT nDisLeverPos,	//显示垂直零电平位置
										  double dbHorizontal,	//水平放大倍数
										  double dbVertical,	//垂直放大倍数
										  USHORT nYTFormat,		//Normal/Scan/Roll
										  ULONG nScanLen		//Scan
										  )
{
	ULONG i = 0,j = 0,nLDataNum = 0,nRDataNum = 0;
	long nTemp = 0;
	USHORT* pDisData;//加上nDisLeverPos的数据

	nDisDataLen = ULONG(nDisDataLen * dbHorizontal);
//获取数据
	pDisData = new USHORT[nDisDataLen];
	for(i = 0; i < nDisDataLen; i++)//
	{
		nTemp = long( (*(pSrcData+i) / dbVertical) + (MAX_DATA - nDisLeverPos) );
		if(nTemp < MIN_DATA)
		{
			nTemp = MIN_DATA;
		}
		else if(nTemp > MAX_DATA)
		{
			nTemp = MAX_DATA;
		}
		*(pDisData+i) = USHORT(nTemp);
	}
	DrawSquareWave(hDC,&Rect,pDisData,nDisDataLen,clrRGB,nDisType);
	delete pDisData;
}

DLL_API void WINAPI HTDrawGeneratorRect(HDC hDC,int nLeft,int nTop,int nRight,int nBottom)
{
	RECT Rect;
	HBRUSH hBrush;

	Rect.left = nLeft;
	Rect.top = nTop;
	Rect.right = nRight;
	Rect.bottom = nBottom;
	//Draw Grid
	hBrush = CreateSolidBrush(RGB(0,0,0));
	FillRect(hDC,&Rect,hBrush);//Grid BkGround
	DeleteObject(hBrush);
//	DrawGridScale(hDC,&Rect,RGB(nBright,nBright,nBright),nHoriGridNum,nVertGridNum,IsGrid,TRUE);//Draw grid Scale
	//Draw Grid Border
	RECT rcBorder;
	rcBorder.left = Rect.left - 1;
	rcBorder.top = Rect.top - 1;
	rcBorder.right = Rect.right + 2;
	rcBorder.bottom = Rect.bottom + 2;
	DrawEdge(hDC,&rcBorder,EDGE_BUMP,BF_TOPLEFT);
	DrawEdge(hDC,&rcBorder,EDGE_ETCHED,BF_BOTTOMRIGHT);
}

//绘制高低电平
DLL_API void WINAPI HTDrawLevel(HDC hDC,int nCenterX,int nCenterY,COLORREF clr, USHORT nLevel)
{
	HPEN hOldPen;
	HPEN hPen;

	hPen = CreatePen(PS_SOLID,1,clr);
	hOldPen = (HPEN)SelectObject(hDC,hPen);
	if(nLevel == 1)//高电平
	{
		MoveToEx(hDC,nCenterX - 10,nCenterY + 5,NULL);
		LineTo(hDC,nCenterX - 5,nCenterY + 5);
		LineTo(hDC,nCenterX - 5,nCenterY - 5);
		LineTo(hDC,nCenterX + 5,nCenterY - 5);
		LineTo(hDC,nCenterX + 5,nCenterY + 5);
		LineTo(hDC,nCenterX + 11,nCenterY + 5);
	}
	else
	{
		MoveToEx(hDC,nCenterX - 10,nCenterY - 5,NULL);
		LineTo(hDC,nCenterX - 5,nCenterY - 5);
		LineTo(hDC,nCenterX - 5,nCenterY + 5);
		LineTo(hDC,nCenterX + 5,nCenterY + 5);
		LineTo(hDC,nCenterX + 5,nCenterY - 5);
		LineTo(hDC,nCenterX + 11,nCenterY - 5);
	}

	SelectObject(hDC,hOldPen);
	DeleteObject(hPen);
}

DLL_API void WINAPI HTDrawMeasLine(HDC hDC,RECT Rect,POINT ptFirst,USHORT nType)
{
	HPEN hFirstPen;
	HPEN hOldPen;
	POINT Points[3];
	HBRUSH NewBrush;
	HRGN TRgn;
	COLORREF clr(RGB(255,0,0));//RGB(0,200,255)

	if(nType == 1)
	{
		clr = RGB(255,255,255);
	}
	NewBrush = CreateSolidBrush(clr);
///////////
	if(ptFirst.x < Rect.left)
	{
		ptFirst.x = Rect.left;
	}
	else if(ptFirst.x > Rect.right)
	{
		ptFirst.x = Rect.right;
	}
	if(ptFirst.y < Rect.top)
	{
		ptFirst.y = Rect.top;
	}
	else if(ptFirst.y > Rect.bottom)
	{
		ptFirst.y = Rect.bottom;
	}
//开始绘图
	hFirstPen = CreatePen(PS_DASHDOTDOT,1,clr);
	hOldPen = (HPEN)SelectObject(hDC,hFirstPen);
	MoveToEx(hDC,ptFirst.x,ptFirst.y,NULL);
	LineTo(hDC,ptFirst.x,Rect.bottom);
	SelectObject(hDC,hOldPen);
	DeleteObject(hFirstPen);
//////////////////////////////////////
	Points[0].x = ptFirst.x - 7;
	Points[0].y = Rect.top - 8;

	Points[1].x = ptFirst.x + 8;
	Points[1].y = Rect.top - 8;

	Points[2].x = ptFirst.x;
	Points[2].y = Rect.top;

	TRgn = CreatePolygonRgn(Points,3,ALTERNATE);
	FillRgn(hDC,TRgn,NewBrush);
	DeleteObject(TRgn);
//////////////////////////////////////

	Points[0].x = ptFirst.x - 4;
	if(Points[0].x < Rect.left)
	{
		Points[0].x = Rect.left;
	}
	Points[0].y = Rect.bottom;

	Points[1].x = ptFirst.x + 5;
	if(Points[1].x > Rect.right)
	{
		Points[1].x = Rect.right;
	}
	Points[1].y = Rect.bottom;

	Points[2].x = ptFirst.x;
	Points[2].y = Rect.bottom - 5;

	TRgn = CreatePolygonRgn(Points,3,ALTERNATE);
	FillRgn(hDC,TRgn,NewBrush);
	DeleteObject(TRgn);

	DeleteObject(NewBrush);

}



/*	For Logic Analyzer													*/

void DrawLASquareSignal(HDC hDC,CONST RECT* lpRect,COLORREF SignalColor,ULONG* pData,ULONG nDataLen,ULONG nCenterData,double fPT/*两个点之间的水平间距*/,ULONG nBit)//画LA方波
{
	ULONG i = 0;
	HPEN OldPen,CurPen;
	long top,bottom,CenterX;
	POINT ptCur,ptPre;
	double dbTmp = 0.0;

	CenterX = lpRect->left + (lpRect->right - lpRect->left)/2;
	top = lpRect->top + 2;
	bottom = lpRect->bottom - 3;
	
	CurPen = CreatePen(PS_SOLID,1,SignalColor);
	OldPen = (HPEN)SelectObject(hDC,CurPen);

//从中间点向右画
	if(nCenterData < nDataLen)
	{
		dbTmp = CenterX;
		ptPre.x = (long)dbTmp;
		if((pData[nCenterData] & nBit) >= 1)//高电平
			ptPre.y = top;
		else// = 0 :低电平
			ptPre.y = bottom;
		MoveToEx(hDC,ptPre.x,ptPre.y,NULL);
		for(i = nCenterData + 1;i < nDataLen; i++)
		{
			dbTmp += fPT;
			ptCur.x = (long)dbTmp;
			if((pData[i] & nBit) >= 1)//高电平
				ptCur.y = top;
			else// = 0 :低电平
				ptCur.y = bottom;
			if(ptCur.x > lpRect->right)//已到屏幕右边沿
			{
				LineTo(hDC,lpRect->right,ptPre.y);
				break;
			}
			else
			{
				LineTo(hDC,ptCur.x,ptPre.y);
				LineTo(hDC,ptCur.x,ptCur.y);
				ptPre = ptCur;
			}
		}
	}
//从中间点向左画
	if(nCenterData > 0)
	{
		dbTmp = CenterX;
		ptPre.x = (long)dbTmp;
		if((pData[nCenterData] & nBit) >= 1)//高电平
			ptPre.y = top;
		else// = 0 :低电平
			ptPre.y = bottom;
		MoveToEx(hDC,ptPre.x,ptPre.y,NULL);
		for(i = nCenterData - 1;i < nCenterData && i >= 0; i--)
		{
			dbTmp -= fPT;
			ptCur.x = (long)dbTmp;
			if((pData[i] & nBit) >= 1)//高电平
				ptCur.y = top;
			else// = 0 :低电平
				ptCur.y = bottom;
			if(ptCur.x < lpRect->left)//已到屏幕左边沿
			{
				LineTo(hDC,ptPre.x,ptCur.y);
				LineTo(hDC,lpRect->left,ptCur.y);
				break;
			}
			else
			{
				LineTo(hDC,ptPre.x,ptCur.y);
				LineTo(hDC,ptCur.x,ptCur.y);
				ptPre = ptCur;
			}
		}
	}


	SelectObject(hDC,OldPen);
	DeleteObject(CurPen);
}

void DrawGridScaleLA(HDC hDC,CONST RECT* lpRect,COLORREF PenColor,USHORT nHoriGridNum,USHORT nVertGridNum,USHORT IsGrid,BOOL bLimit)
{
	HPEN CurrentPen,oldPen,Pen;
	int i,j = 0;
	POINT ptCenter;
	long top = 0;
	int  preMode = 0;

	ptCenter.x = lpRect->left + (lpRect->right - lpRect->left)/2;
	ptCenter.y = lpRect->top + (lpRect->bottom - lpRect->top)/2;
//画大刻度虚线
	Pen = CreatePen(PS_SOLID,1,PenColor);
	CurrentPen = CreatePen(PS_DOT,1,PenColor);
	oldPen = (HPEN)SelectObject(hDC,CurrentPen);
	preMode = SetBkMode(hDC,TRANSPARENT);
	top = lpRect->top + 15;
	for(i = ptCenter.x/* + 100*/; i <= lpRect->right;i += 100)
	{
		MoveToEx(hDC,i,top,NULL);
		LineTo(hDC,i,lpRect->bottom);
	}
	for(i = ptCenter.x - 100; i >= lpRect->left;i -= 100)
	{
		MoveToEx(hDC,i,top,NULL);
		LineTo(hDC,i,lpRect->bottom);
	}
//画实线表示中间刻度
//	MoveToEx(hDC,ptCenter.x,top,NULL);
//	LineTo(hDC,ptCenter.x,lpRect->bottom);

	SelectObject(hDC,Pen);
//画顶部实线小刻度	
	top = lpRect->top + 15;
	MoveToEx(hDC,ptCenter.x,top - 5,NULL);
	LineTo(hDC,ptCenter.x,top - 15);
	j = 0;
	for(i = ptCenter.x + 20; i <= lpRect->right;i += 20)
	{
		j++;
		MoveToEx(hDC,i,top - 5,NULL);
		if(j % 5 == 0)
			LineTo(hDC,i,top - 15);
		else
			LineTo(hDC,i,top - 8);
	}
	j = 0;
	for(i = ptCenter.x - 20; i >= lpRect->left;i -= 20)
	{
		j++;
		MoveToEx(hDC,i,top - 5,NULL);
		if(j % 5 == 0)
			LineTo(hDC,i,top - 15);
		else
			LineTo(hDC,i,top - 8);
	}
	SelectObject(hDC,oldPen);
	SetBkMode(hDC,preMode);
	DeleteObject(CurrentPen);
	DeleteObject(Pen);
}
/*
void DrawTopScaleLA(HDC hDC,CONST RECT* lpRect,COLORREF PenColor,USHORT nHoriGridNum,USHORT nVertGridNum,USHORT IsGrid,BOOL bLimit)
{
	HPEN oldPen,Pen;
	int i,j = 0;
	POINT ptCenter;
	long top = 0;

	ptCenter.x = lpRect->left + (lpRect->right - lpRect->left)/2;
	ptCenter.y = lpRect->top + (lpRect->bottom - lpRect->top)/2;

	Pen = CreatePen(PS_SOLID,1,PenColor);
	oldPen = (HPEN)SelectObject(hDC,Pen);

//画顶部实线小刻度	
	top = lpRect->top + 15;
	MoveToEx(hDC,ptCenter.x,top - 5,NULL);
	LineTo(hDC,ptCenter.x,top - 15);
	j = 0;
	for(i = ptCenter.x + 20; i <= lpRect->right;i += 20)
	{
		j++;
		MoveToEx(hDC,i,top - 5,NULL);
		if(j % 5 == 0)
			LineTo(hDC,i,top - 15);
		else
			LineTo(hDC,i,top - 8);
	}
	j = 0;
	for(i = ptCenter.x - 20; i >= lpRect->left;i -= 20)
	{
		j++;
		MoveToEx(hDC,i,top - 5,NULL);
		if(j % 5 == 0)
			LineTo(hDC,i,top - 15);
		else
			LineTo(hDC,i,top - 8);
	}
	SelectObject(hDC,oldPen);
	DeleteObject(Pen);
}
*/

void DrawGroupGridLA(HDC hDC,CONST RECT* lpRect,COLORREF PenColor,USHORT nHoriGridNum,USHORT nVertGridNum,USHORT IsGrid,BOOL bLimit)
{
	HPEN oldPen,Pen;
	int i = 0;
//	ptCenter.x = lpRect->left + (lpRect->right - lpRect->left)/2;
//	ptCenter.y = lpRect->top + (lpRect->bottom - lpRect->top)/2;
	Pen = CreatePen(PS_SOLID,1,PenColor);
	oldPen = (HPEN)SelectObject(hDC,Pen);

	
	for(i = lpRect->top; i <= lpRect->bottom; i += 20)
	{
		MoveToEx(hDC,lpRect->left,i,NULL);
		LineTo(hDC,lpRect->right,i);
	}

	SelectObject(hDC,oldPen);
	DeleteObject(Pen);
}





DLL_API void WINAPI HTDrawGridLA(HDC hDC,int nLeft,int nTop,int nRight,int nBottom,USHORT nHoriGridNum,USHORT nVertGridNum,USHORT nBright,USHORT IsGrid)
{
	RECT Rect;

	Rect.left = nLeft;
	Rect.top = nTop;
	Rect.right = nRight;
	Rect.bottom = nBottom;

	if(nBright > 255)
	{
		nBright = 255;
	}
	//Draw Grid Scale
	DrawGridScaleLA(hDC,&Rect,RGB(nBright,nBright,nBright),nHoriGridNum,nVertGridNum,IsGrid,TRUE);//Draw grid Scale
}

DLL_API void WINAPI HTDrawGroupGridLA(HDC hDC,int nLeft,int nTop,int nRight,int nBottom,USHORT nHoriGridNum,USHORT nVertGridNum,USHORT nBright,USHORT IsGrid)
{
	RECT Rect;

	Rect.left = nLeft;
	Rect.top = nTop;
	Rect.right = nRight;
	Rect.bottom = nBottom;

	if(nBright > 255)
	{
		nBright = 255;
	}
	//Draw Grid
	DrawGroupGridLA(hDC,&Rect,RGB(nBright,nBright,nBright),nHoriGridNum,nVertGridNum,IsGrid,TRUE);//Draw grid Scale
}

DLL_API void WINAPI HTDrawLASquareSignal(HDC hDC,CONST RECT* lpRect,COLORREF SignalColor,ULONG* pData,ULONG nDataLen,ULONG nCenterData,double fPT/*两个点之间的水平间距*/,ULONG nBit)//画LA方波
{
	DrawLASquareSignal(hDC,lpRect,SignalColor,pData,nDataLen,nCenterData,fPT,nBit);
}
