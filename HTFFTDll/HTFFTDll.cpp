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
/*������FFT�е�λת��*/
/*��һ��������Ҫת���ĳ�ʼ����ֵ*/
/*�ڶ��������Ǵ���������Ϊ2�ļ�����*/
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
		if ((Index & a)!=0)//��λ��λ������
		{
			*(Temp+(BitNo - 1 - i)) = 1;
		}
		else
		{
			*(Temp+(BitNo - 1 - i)) = 0;
		}
	}
	LL = 0;
	//������ź������ֵ
	for(i=0;i<BitNo;i++)
	{
		LL = LL +*(Temp+i) * (int)pow(2,i);
	}
	delete Temp;
	return LL;//��������ֵ
}
/*******************************************/
/*FFT��ʼ��*/
/*��һ�����������ݸ���*/
/*�ڶ��������Ǵ�������*/
/*�����������Ǵ���������Ϊ2�ļ�����*/
/*���ĸ����������洢���ں�����Ȩֵ*/
/******************************************/
void FFTWindowInitial(int n,int WinType,double *win)
{
	int i;
	double Factor ,arg;
	
	//����2*pi/(N-1)����ֵ
	Factor = 8.0 * atan(1.0) / (n - 1);
	//���ղ�ͬ�Ĵ��ں������������N����ֵ�µļ�Ȩֵ
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
/*FFT���ڼ���ʱ����ֵ��*/
/*��һ��������ԭʼʱ������*/
/*�ڶ���������ʵ�����������ı�־(ͨ������1����ʱ��)*/
/*������������X�����������ֵ*/
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
	m = (int)(log(SamplePoints)/log(2) +0.001);//���ݵ�����2�ļ�����
	X = new HTCOMPEX[SamplePoints];//	X = (HTCOMPEX*)malloc(SamplePoints * sizeof(HTCOMPEX));
	win = new double[SamplePoints];//	win = (double*)malloc(SamplePoints * sizeof(double));
	//�ȼ����������ֵ
	FFTWindowInitial(SamplePoints,nWindowType,win);
	//��ԭ����ʱ������ָ����������ʵ��
	for(i=0;i<SamplePoints;i++)
	{
		X[i].Real = *(VoltageData+i);
	}
	//���½���ʱ�����ݵĴ��ڴ���
	FFTWindow(X,1,SamplePoints,win);//ֻ��ʵ��������Windowing
	
	//���½���ʵ�ʵ�FFT����
	/////////////////////////////////////////////////////////
	n = (int)pow(2,m);//���ݳ���
	Le = n/2;//��Ϊ������
	W = new HTCOMPEX[Le];//	W = (HTCOMPEX*)malloc(Le * sizeof(HTCOMPEX));
	arg = 4.0 * atan(1.0) / Le ;
	w_Real = cos(arg);
	wrecur_Real = w_Real;
	w_Cmpx = -sin(arg);
	wrecur_Cmpx = w_Cmpx;

	//�����е�Wn���м����
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
	
	//����ͼ��������
	Le = n;
	WIndex = 1;
	for (LL=0;LL<m;LL++)
	{
		Le = Le / 2;	//ÿ�εļ�������Ϊǰ�ε�һ��
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
		WIndex = 2 * WIndex;//ָ������
	}
	//λ����
	fRe = new bool[(int)pow(2,m)];//	fRe = (bool*)malloc((int)pow(2,m) * sizeof(bool));
	for(i=0;i<pow(2,m);i++)
	{
		*(fRe+i) = false;
	}

	for(i=0;i<n-1;i++)
	{
		if (!*(fRe+i))
		{
			j = BitReverse(i,m);//����λ���ŵ�����ֵ
			*(fRe+j) = true;
			//��Ԫ�ص�λ�õ���
			Temp = X[j];
			X[j] = X[i];
			X[i] = Temp;
		}
	}
	///////////////////////////////////////////////////////////////////////
	//FFT�������

	//���¼���У����ֵ
	CalValue = 0;
	for(i=0;i<SamplePoints;i++)
	{
		CalValue = CalValue + win[i];
	}
	//��ͬFFT�ĵ���һ����У��
	CalValue = CalValue / 2;
	//��ת����ɵ���ֵ���ԭ����
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
