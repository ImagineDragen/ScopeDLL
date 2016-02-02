// HTFFTDll.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

/*******************************************/
/*用来作FFT中的位转换*/
/*第一个参数是要转换的初始索引值*/
/*第二个参数是次数据序列为2的几次幂*/
/******************************************/
int BitReverse(int Index, int BitNo)
{
	int *Temp;
	int i,LL;
	int a;

//	Temp = (int*)malloc(BitNo * sizeof (int));
	Temp = new int[BitNo];
	for (i=0;i<BitNo;i++)
	{
		a = (int)pow(2,i);
		if ((Index & a)!=0)//将位的位置重排
		{
			*(Temp+(BitNo - 1 - i)) = 1;
		}
		else
		{
			*(Temp+(BitNo - 1 - i)) = 0;
		}
	}
	LL = 0;
	//算出重排后的索引值
	for(i=0;i<BitNo;i++)
	{
		LL = LL +*(Temp+i) * (int)pow(2,i);
	}
	delete Temp;
	return LL;//返回索引值
}
/*******************************************/
/*FFT初始化*/
/*第一个参数是数据个数*/
/*第二个参数是窗口种类*/
/*第三个参数是次数据序列为2的几次幂*/
/*第四个参数用来存储窗口函数加权值*/
/******************************************/
void FFTWindowInitial(int n,int WinType,double *win)
{
	int i;
	double Factor ,arg;
	
	//定义2*pi/(N-1)的数值
	Factor = 8.0 * atan(1.0) / (n - 1);
	//按照不同的窗口函数定义计算在N点数值下的加权值
	switch(WinType)
	{
	case 0://Rectangle
		for(i=0;i<n;i++)
		{
			*(win+i) = 1;
		}
		break;
	case 1://Hanning
		for(i=0;i<n;i++)
		{
			arg = Factor * i;
			*(win+i) = 0.5 - 0.5 * cos(arg);
		}
		break;
	case 2://Hamming
		for(i=0;i<n;i++)
		{
			arg = Factor * i;
			*(win+i) = 0.54 - 0.46 * cos(arg);
		}
		break;
	case 3://Blackman
		for(i=0;i<n;i++)
		{
			arg = Factor * i;
			*(win+i) = 0.42 - 0.5 * cos(arg) + 0.08 * cos(2*arg);
		}
		break;
	case 4://Triang
		for(i=0;i< n / 2;i++)
		{
			*(win+i) = i * 2.0 / (n - 1);
		}
		for(i=0;i<n;i++)
		{
			*(win+i) = 2.0 - i * 2.0 / (n - 1);
		}
		break;
	case 5://Flat_Top
		for(i=0;i<n;i++)
		{
			arg = Factor * i;
			*(win+i) = (1.0-1.93*cos(arg)+1.29*cos(2*arg)-0.388*cos(2*arg)+0.0322*cos(4*arg))/2;
		}
		break;
	default:
		for(i=0;i<n;i++)
		{
			*(win+i) = 1;
		}
		break;
	}
}
/*****************************************************/
/*FFT窗口加入时域数值中*/
/*第一个参数是原始时域数据*/
/*第二个参数是实数或者虚数的标志(通常采用1，表时域)*/
/*第三个参数是X数组最大索引值*/
/*****************************************************/
void FFTWindow(HTCOMPEX *X,int fReal,int TNo,double *win)
{
	int i;
	if (fReal == 1)
	{
		for(i=0;i<TNo;i++)
		{
			(X+i)->Real = (X+i)->Real * (*(win+i));
			(X+i)->Cmpx = 0;
		}
	}
	else
	{
		for(i=0;i<TNo;i++)
		{
			(X+i)->Real = (X+i)->Real * (*(win+i));
			(X+i)->Cmpx = (X+i)->Cmpx * (*(win+i));
		}
	}
}

DLL_API WORD WINAPI dsoHTFFT(double* pSourceData,ULONG nSourceDataLen,USHORT nWindowType,double *FFTResult,USHORT nScale)
{
	HTCOMPEX* X;
	HTCOMPEX* W;
	HTCOMPEX U,Temp,Tm;
	ULONG m,n,LL,Le,WIndex;
	ULONG i,j,SamplePoints;
	double *win;
	double arg,w_Real,w_Cmpx,wrecur_Real,wrecur_Cmpx,wtemp_Real,CalValue;
	bool *fRe;
	double *VoltageData;

	SamplePoints = nSourceDataLen;
	VoltageData = pSourceData;//(double*)malloc(SamplePoints * sizeof(double));
	m = (int)(log(SamplePoints)/log(2) +0.001);//数据点数是2的几次幂
	X = new HTCOMPEX[SamplePoints];//	X = (HTCOMPEX*)malloc(SamplePoints * sizeof(HTCOMPEX));
	win = new double[SamplePoints];//	win = (double*)malloc(SamplePoints * sizeof(double));
	//先计算出窗口数值
	FFTWindowInitial(SamplePoints,nWindowType,win);
	//将原来的时域数据指定给复数的实部
	for(i=0;i<SamplePoints;i++)
	{
		X[i].Real = *(VoltageData+i);
	}
	//以下进行时域数据的窗口处理
	FFTWindow(X,1,SamplePoints,win);//只按实数部分做Windowing
	
	//以下进行实质的FFT运算
	/////////////////////////////////////////////////////////
	n = (int)pow(2,m);//数据长度
	Le = n/2;//分为两部分
	W = new HTCOMPEX[Le];//	W = (HTCOMPEX*)malloc(Le * sizeof(HTCOMPEX));
	arg = 4.0 * atan(1.0) / Le ;
	w_Real = cos(arg);
	wrecur_Real = w_Real;
	w_Cmpx = -sin(arg);
	wrecur_Cmpx = w_Cmpx;

	//将所有的Wn先行计算出
	W[0].Real = 1.0;
	W[0].Cmpx = 0.0;
	for (j = 1;j < Le ;j++)
	{
		W[j].Real = wrecur_Real;
		W[j].Cmpx = wrecur_Cmpx;
		wtemp_Real = wrecur_Real * w_Real - wrecur_Cmpx * w_Cmpx;
		wrecur_Cmpx = wrecur_Real * w_Cmpx +wrecur_Cmpx * w_Real;
		wrecur_Real = wtemp_Real;
	}
	
	//蝴蝶图各级计算
	Le = n;
	WIndex = 1;
	for (LL=0;LL<m;LL++)
	{
		Le = Le / 2;	//每次的计算组数为前次的一半
		for(j = 0 ;j < Le ;j++)
		{
			U = W [j * WIndex];
			for (i = j; i < n;i= i+2*Le)
			{
				Temp.Real = X[i].Real + X[i+Le].Real;
				Temp.Cmpx = X[i].Cmpx + X[i+Le].Cmpx;
				Tm.Real = X[i].Real - X[i+Le].Real;
				Tm.Cmpx = X[i].Cmpx - X[i+Le].Cmpx;
				X[i+Le].Real = Tm.Real * U.Real - Tm.Cmpx * U.Cmpx;
				X[i+Le].Cmpx = Tm.Real * U.Cmpx + Tm.Cmpx * U.Real;
				X[i] = Temp;
			}
		}
		WIndex = 2 * WIndex;//指数倍增
	}
	//位重排
	fRe = new bool[(int)pow(2,m)];//	fRe = (bool*)malloc((int)pow(2,m) * sizeof(bool));
	for(i=0;i<pow(2,m);i++)
	{
		*(fRe+i) = false;
	}

	for(i=0;i<n-1;i++)
	{
		if (!*(fRe+i))
		{
			j = BitReverse(i,m);//计算位重排的索引值
			*(fRe+j) = true;
			//将元素的位置调换
			Temp = X[j];
			X[j] = X[i];
			X[i] = Temp;
		}
	}
	///////////////////////////////////////////////////////////////////////
	//FFT运算结束

	//以下计算校正数值
	CalValue = 0;
	for(i=0;i<SamplePoints;i++)
	{
		CalValue = CalValue + win[i];
	}
	//连同FFT的点数一并作校正
	CalValue = CalValue / 2;
	//将转换完成的数值存回原数组
	for(i=0;i<SamplePoints/2;i++)
	{
		*(FFTResult+i) = sqrt(pow(X[i].Real,2) +pow(X[i].Cmpx,2)) / CalValue;
	}

	if(nScale == 1)//dBrms
	{
		for(i=0;i<SamplePoints/2;i++)
		{
			if(*(FFTResult+i) != 0)
			{
				*(FFTResult+i) = log(*(FFTResult+i))/log(10)*20;
			}
			else
			{
				if(i >= 1)
				{
					*(FFTResult+i) = *(FFTResult+i-1);
				}
				else
				{
					*(FFTResult+i) = 0;
				}
			}
		}
	}

	delete W;
	delete X;
	delete win;
	delete fRe;

	return 1;
}
