// MeasDll.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

//���ݵ�ʵ�ʵ�ѹת��
double TransformVolt(short nData,double dbVoltDIV,short nMaxData)
{
	return nData * dbVoltDIV * V_GRID_NUM / nMaxData;
}
DLL_API double WINAPI CalAverage(const short* pSrcData,ULONG nSrcDataLen)
{
	ULONG i;
	__int64 nSum = 0;
	if(nSrcDataLen>500)
	{
		ULONG nCount=0;
		for(i=16;i<nSrcDataLen-16;i++)
		{
			nSum += *(pSrcData+i); 
			nCount++;
		}
		return nSum*1.0/nCount;
	}
	else
	{
		for(i=0;i<nSrcDataLen;i++)
		{
			nSum += *(pSrcData+i);  
		}
	}
	return nSum*1.0/nSrcDataLen;
}
ULONG FindFirstFall(short* pSrcData,ULONG nSrcDataLen,short nMidPoint)
{
	ULONG i;
	short PreStatus,NowStatus,StatusCount;
	if(*(pSrcData) >= nMidPoint)
	{
		NowStatus = 1;
	}
	else
	{
		NowStatus = 0;
	}
	PreStatus = NowStatus;
	StatusCount = 0;

	if(NowStatus == 1)//�����1������е��
	{
		for(i=1;i<nSrcDataLen;i++)
		{
			if(*(pSrcData+i) <= nMidPoint)
			{
				return i;
			}
		}
	}
	else
	{
		for(i=1;i<nSrcDataLen;i++)
		{
			if(*(pSrcData+i) >= nMidPoint)
			{
				PreStatus = NowStatus;
				NowStatus = 1;
			}
			else
			{
				PreStatus = NowStatus;
				NowStatus = 0;
			}
			if(StatusCount == 0 && PreStatus == 0 && NowStatus == 1)//�ҵ���������
			{
				StatusCount = 1;
			}
			else if(StatusCount == 1 && PreStatus == 1 && NowStatus == 0)//�ҵ��½�����
			{
				return i;
			}
		}
	}
	return -1;
}

DLL_API void WINAPI FindPeriod(ULONG* PeriodInfo,const short* pMaxData,const short* pMinData,const short* pSrcData,ULONG nSrcDataLen)
{
	short PreStatus,NowStatus,StatusCount;
	short midpoint;//�е�
	ULONG i;

	for(i=0;i<3;i++)//�����
	{
		*(PeriodInfo + i) = 0;
	}
	StatusCount = 0;
	midpoint = (*(pMaxData+4) + *(pMinData+4))/2;
	if(*(pSrcData) > midpoint)
		NowStatus = 1;
	else
		NowStatus = 0;
	PreStatus = NowStatus;

	for(i=1;i<nSrcDataLen;i+=2)//��2����
	{
		if(*(pSrcData+i)+6 < midpoint)
		{
			PreStatus = NowStatus;
			NowStatus = 0;
		}
		else if(*(pSrcData+i)-6 > midpoint)
		{
			PreStatus = NowStatus;
			NowStatus = 1;
		}
		if(StatusCount == 0 && PreStatus == 0 && NowStatus == 1)//�ҵ���һ��������
		{
			*PeriodInfo = i;//��ʼ��
			StatusCount = 1;
		}
		else if(StatusCount == 1 && PreStatus == 1 && NowStatus == 0)//�ҵ��½���
		{
			*(PeriodInfo+1) = i;//�е�
			StatusCount = 2;
		}
		else if(StatusCount == 2 && PreStatus == 0 && NowStatus == 1)//�ҵ��ڶ���������
		{
			*(PeriodInfo+2) = i;//������
			break;
		}
	}
	if(*(PeriodInfo+2)==0)//˵��û���ҵ�
	{
		*(PeriodInfo+0) = 0;
		*(PeriodInfo+1) = 0;
	}
}

/////////
//������õ�ǰ5���ֵ��ǰ5��Сֵ
DLL_API void WINAPI PreMeas(short* pMaxData,short* pMinData,const short* pSrcData,ULONG nSrcDataLen)
{
	ULONG i;

	for(i=0;i<5;i++)
	{
		*(pMaxData+i) = -32767;//-MAX_DATA;
		*(pMinData+i) = 32767;//MAX_DATA;
	}
	for(i=0;i<nSrcDataLen;i++)
	{
		if(*(pSrcData+i)>*pMaxData)
			*pMaxData = *(pSrcData+i);
		else if(*(pSrcData+i)>*(pMaxData+1))
			*(pMaxData+1) = *(pSrcData+i);
		else if(*(pSrcData+i)>*(pMaxData+2))
			*(pMaxData+2) = *(pSrcData+i);
		else if(*(pSrcData+i)>*(pMaxData+3))
			*(pMaxData+3) = *(pSrcData+i);
		else if(*(pSrcData+i)>*(pMaxData+4))
			*(pMaxData+4) = *(pSrcData+i);
		else if(*(pSrcData+i)<*pMinData)
			*pMinData = *(pSrcData+i);
		else if(*(pSrcData+i)<*(pMinData+1))
			*(pMinData+1) = *(pSrcData+i);
		else if(*(pSrcData+i)<*(pMinData+2))
			*(pMinData+2) = *(pSrcData+i);
		else if(*(pSrcData+i)<*(pMinData+3))
			*(pMinData+3) = *(pSrcData+i);
		else if(*(pSrcData+i)<*(pMinData+4))
			*(pMinData+4) = *(pSrcData+i);
	}
}
/***************************************************************ˮƽ(ʱ��)*****************************************************************************/
//����Ƶ��
DLL_API double WINAPI CalFrequency(const ULONG* PeriodInfo,double dbTimeInterval)//��λhz
{
	double f = 0.0;
	if(dbTimeInterval != 0 && *(PeriodInfo+2) != 0)
	{
		f = (*(PeriodInfo+2) - *(PeriodInfo)) * dbTimeInterval;
	}
	if(f > 0.0)
	{
		return 1.0/f;
	}
	else
	{
		return -1;
	}
}
//��������
DLL_API double WINAPI CalPeriod(const ULONG* PeriodInfo,double dbTimeInterval)//��λs
{
	double f = 0.0;
	if(dbTimeInterval != 0 && *(PeriodInfo+2) != 0)
	{
		f = (*(PeriodInfo+2) - *(PeriodInfo)) * dbTimeInterval;
	}
	if(f > 0.0)
	{
		return f;
	}
	else
	{
		return -1;
	}
}
//��������ʱ��
DLL_API double WINAPI CalRiseTime(const short* pMaxData,const short* pMinData,const short* pSrcData,ULONG nSrcDataLen,double dbTimeInterval,float fTop,float fBottom)
{
	short TopValue,BottomValue,RefValue;
	short PreStatus,NowStatus,StatusCount;
	ULONG Start=0,End=0,i;

	TopValue = short(*(pMinData+4) + (*(pMaxData+4) - *(pMinData+4))*fTop);
	BottomValue = short(*(pMinData+4) + (*(pMaxData+4) - *(pMinData+4))*fBottom);

	RefValue = BottomValue;
	StatusCount = 0;
	if(RefValue<*pSrcData)
		NowStatus = 1;
	else
		NowStatus = 0;
	PreStatus = NowStatus;

	for(i=1;i<nSrcDataLen;i++)
	{
		if(RefValue<=*(pSrcData+i))
		{
			PreStatus = NowStatus;
			NowStatus = 1;
		}
		else if(RefValue>*(pSrcData+i))
		{
			PreStatus = NowStatus;
			NowStatus = 0;
		}
		if(StatusCount==0 && PreStatus==0 && NowStatus==1)
		{
			Start = i;
			StatusCount = 1;
			RefValue = TopValue;
			if(RefValue<=*(pSrcData+i))//����˵�ֱ�Ӵ���TopValue
			{
				End = i;
				break;
			}
			else
			{
				NowStatus = 0;
			}
			PreStatus = NowStatus;
		}
		else if(StatusCount==1 && PreStatus==0 && NowStatus==1)
		{
			End = i;
			break;
		}
	}
	if(End >= Start)
	{
		return (End - Start) * dbTimeInterval;
	}
	else
	{
		return -1;
	}
}
//�����½�ʱ��
DLL_API double WINAPI CalFallTime(const short* pMaxData,const short* pMinData,const short* pSrcData,ULONG nSrcDataLen,double dbTimeInterval,float fTop,float fBottom)
{
	short TopValue,BottomValue,RefValue;
	short PreStatus,NowStatus,StatusCount;
	ULONG Start=0,End=0,i;

	TopValue = short(*(pMinData+4) + (*(pMaxData+4) - *(pMinData+4))*fTop);//90%
	BottomValue = short(*(pMinData+4) + (*(pMaxData+4) - *(pMinData+4))*fBottom);//10%

	RefValue = TopValue;
	StatusCount = 0;
	if(RefValue<*pSrcData)
		NowStatus = 1;
	else
		NowStatus = 0;
	PreStatus = NowStatus;

	for(i=1;i<nSrcDataLen;i++)
	{
		if(RefValue<=*(pSrcData+i))
		{
			PreStatus = NowStatus;
			NowStatus = 1;
		}
		else if(RefValue>*(pSrcData+i))
		{
			PreStatus = NowStatus;
			NowStatus = 0;
		}
		if(StatusCount==0 && PreStatus==1 && NowStatus==0)
		{
			Start = i;
			StatusCount = 1;
			RefValue = BottomValue;
			if(RefValue>=*(pSrcData+i))//����˵�ֱ��С��BottomValue
			{
				End = i;
				break;
			}
			else
				NowStatus = 1;
			PreStatus = NowStatus;
		}
		else if(StatusCount==1 && PreStatus==1 && NowStatus==0)
		{
			End = i;
			break;
		}
	}
	if(End >= Start)
	{
		return (End - Start) * dbTimeInterval;
	}
	else
	{
		return -1;
	}
}
//������ռ�ձ�
DLL_API double WINAPI CalPDutyCycle(const ULONG* PeriodInfo)
{
	ULONG a=0,b=0;

	a = *(PeriodInfo+1) - *PeriodInfo;
	b = *(PeriodInfo+2) - *PeriodInfo;

	if(a==0 || b==0)
	{
		return -1;
	}
	else
	{
		return a*1.0/b;
	}
}
//���㸺ռ�ձ�
DLL_API double WINAPI CalNDutyCycle(const ULONG* PeriodInfo)
{
	ULONG a=0,b=0;

	a = *(PeriodInfo+2) - *(PeriodInfo+1);
	b = *(PeriodInfo+2) - *PeriodInfo;

	if(a==0 || b==0)
	{
		return -1;
	}
	else
	{
		return a*1.0/b;
	}
}
//����������
DLL_API double WINAPI CalPPulseWidth(const ULONG* PeriodInfo,double dbTimeInterval)
{
	double fTime = 0;

	fTime = (*(PeriodInfo+1) - *PeriodInfo) * dbTimeInterval;

	if(fTime <= 0)
	{
		return -1;
	}
	else
	{
		return fTime;
	}
}
//���㸺����
DLL_API double WINAPI CalNPulseWidth(const ULONG* PeriodInfo,double dbTimeInterval)
{
	double fTime = 0;

	fTime = (*(PeriodInfo+2) - *(PeriodInfo+1)) * dbTimeInterval;

	if(fTime <= 0)
	{
		return -1;
	}
	else
	{
		return fTime;
	}
}
//����CH1/CH2���������ӳٲ�
DLL_API double WINAPI CalPDelay12(ULONG* CH1PeriodInfo,ULONG* CH2PeriodInfo,double dbCH1Interval,double dbCH2Interval)
{
	double dbCH1Time = *CH1PeriodInfo * dbCH1Interval;
	double dbCH2Time = *CH2PeriodInfo * dbCH2Interval;

	return dbCH2Time - dbCH1Time;
}
//����CH1/CH2���½����ӳٲ�
DLL_API double WINAPI CalNDelay12(short* pCH1MaxData,short* pCH1MinData,
								  short* pCH2MaxData,short* pCH2MinData,
								  short* pCH1SrcData,ULONG nCH1SrcDataLen,
								  short* pCH2SrcData,ULONG nCH2SrcDataLen,
								  double dbCH1Interval,double dbCH2Interval)
{
	
	short CH1MidPoint,CH2MidPoint;//�е�
	ULONG nCH1FirstFall,nCH2FirstFall;
	CH1MidPoint = (*(pCH1MaxData+4) + *(pCH1MinData+4))/2;
	CH2MidPoint = (*(pCH2MaxData+4) + *(pCH2MinData+4))/2;
	
	nCH1FirstFall = FindFirstFall(pCH1SrcData,nCH1SrcDataLen,CH1MidPoint);
	nCH2FirstFall = FindFirstFall(pCH2SrcData,nCH2SrcDataLen,CH2MidPoint);
	
	if(nCH1FirstFall != -1 && nCH1FirstFall != -1)
	{
		return nCH2FirstFall * dbCH2Interval - nCH1FirstFall * dbCH1Interval;
	}
	else
	{
		return -1;
	}
}

/***************************************************************��ֱ(��ѹ)*****************************************************************************/
//�������ֵ
DLL_API double WINAPI CalMaxVolt(const short* pMaxData,double dbVoltDIV,short nMaxData)
{
	return TransformVolt(*pMaxData,dbVoltDIV,nMaxData);
}
//������Сֵ
DLL_API double WINAPI CalMinVolt(const short* pMinData,double dbVoltDIV,short nMaxData)
{
	return TransformVolt(*pMinData,dbVoltDIV,nMaxData);
}
//������ֵ
DLL_API double WINAPI CalVpp(const short* pMaxData,short* pMinData,double dbVoltDIV,short nMaxData)
{
	return (*pMaxData - *pMinData) * (dbVoltDIV*V_GRID_NUM/nMaxData);
}
//���㶥��ֵ
DLL_API double WINAPI CalTopVolt(const short* pMaxData,double dbVoltDIV,short nMaxData)
{
	return TransformVolt(*(pMaxData+4),dbVoltDIV,nMaxData);
}
//����ײ�ֵ
DLL_API double WINAPI CalBaseVolt(const short* pMinData,double dbVoltDIV,short nMaxData)
{
	return TransformVolt(*(pMinData+4),dbVoltDIV,nMaxData);
}
//�����м�ֵ
DLL_API double WINAPI CalMidVolt(const short* pMaxData,const short* pMinData,double dbVoltDIV,short nMaxData)
{
	return ((*(pMaxData+4) - *(pMinData+4)) / 2.0 + *(pMinData+4)) * dbVoltDIV * V_GRID_NUM / nMaxData;
}
//������Чֵ
DLL_API double WINAPI CalRMS(const short* pSrcData,ULONG nSrcDataLen,double dbVoltDIV,short nMaxData)
{
	ULONG i;
	double sum = 0;

	for(i=0;i<nSrcDataLen;i++)
	{
		sum += pow(*(pSrcData+i)*dbVoltDIV*V_GRID_NUM/nMaxData, 2);
	}
	return sqrt(sum/nSrcDataLen);
}

DLL_API double WINAPI CalCRMS(const ULONG* PeriodInfo,const short* pSrcData,double dbVoltDIV,short nMaxData)
{
	float fResult = 0;
	double sum = 0;
	ULONG i;
	ULONG Points=0,StartIndex=0,EndIndex=0;


	StartIndex = *PeriodInfo;
	EndIndex = *(PeriodInfo+2);
	Points = *(PeriodInfo+2) - *PeriodInfo + 1;

	if(EndIndex!=0)
	{
		for(i=StartIndex;i<=EndIndex;i++)
		{
			sum += pow(*(pSrcData+i)*dbVoltDIV*V_GRID_NUM/nMaxData, 2);
		}
		return sqrt(sum/Points);
	}
	else
	{
		return 0;
	}
}
//�������ֵ
DLL_API double WINAPI CalAmplitude(const short* pMaxData,const short* pMinData,double dbVoltDIV,short nMaxData)
{
	return (*(pMaxData+4) - *(pMinData+4)) * dbVoltDIV * V_GRID_NUM / nMaxData;
}
//����ƽ��ֵ
DLL_API double WINAPI CalMean(const short* pSrcData,ULONG nSrcDataLen,double dbVoltDIV,short nMaxData)
{
	return CalAverage(pSrcData,nSrcDataLen)* dbVoltDIV * V_GRID_NUM / nMaxData;
}
//��������ƽ��ֵ
DLL_API double WINAPI CalCMean(const ULONG* PeriodInfo,const short* pSrcData,double dbVoltDIV,short nMaxData)
{
	float fResult = 0;
	double sum = 0;
	ULONG i;
	ULONG Points=0,StartIndex=0,EndIndex=0;


	StartIndex = *PeriodInfo;
	EndIndex = *(PeriodInfo+2);
	Points = *(PeriodInfo+2) - *PeriodInfo + 1;

	if(EndIndex!=0)
	{
		for(i=StartIndex;i<=EndIndex;i++)
		{
			sum = sum + *(pSrcData+i);
		}
		return (sum/Points)* dbVoltDIV * V_GRID_NUM / nMaxData;
	}
	else
	{
		return 0;
	}
}
//����Ԥ��
DLL_API double WINAPI CalPreShoot(const short* pMaxData,const short* pMinData)
{
	double sub=0,amp=0;

	sub = double(*(pMinData+4) - *pMinData);
	amp = double(*(pMaxData+4) - *(pMinData+4));

	if(amp!=0)
	{
		return sub/amp;
	}
	else
	{
		return -1;
	}
}

//�������
DLL_API double WINAPI CalOverShoot(const short* pMaxData,const short* pMinData)
{
	double sub=0,amp=0;

	sub = double(*pMaxData - *(pMaxData+4));
	amp = double(*(pMaxData+4) - *(pMinData+4));

	if(amp!=0)
	{
		return sub/amp;
	}
	else
	{
		return -1;
	}
	
}

//Cursor Measure..............

DLL_API double WINAPI CursorTime(long x1, long x2, double dbTime)
{
	return fabs(x2 - x1) * dbTime;
}

DLL_API double WINAPI CursorFrequency(long x1, long x2, double dbTime)
{
	double dbRe = CursorTime(x1,x2,dbTime);
	if(dbRe != 0)
	{
		return 1/dbRe;
	}
	else
	{
		return 0;
	}
}

DLL_API double WINAPI CursorFFTFrequency(long x1,long x2,double dbFreq)
{
	return fabs(x2 - x1) * dbFreq;
}

DLL_API double WINAPI CursorVoltage(long y1,long y2,double dbVolt)
{
	return (y2 - y1) * dbVolt;
}

DLL_API double WINAPI CursorTraceVoltage(short nData,double dbVolt)
{
	return nData * dbVolt;
}

DLL_API USHORT WINAPI GetMaxMinData(short* pMaxData,short* pMinData,const short* pSrcData,ULONG nSrcDataLen)
{
	short nMaxData = -MAX_DATA,nMinData = MAX_DATA;
	ULONG i=0;
	ULONG nEnd=nSrcDataLen;
	if(nSrcDataLen>2048){
		i=512;
		nEnd=nSrcDataLen-512;
	}

	for(;i<nEnd;i++)
	{
		if(*(pSrcData+i) > nMaxData)
		{
			nMaxData = *(pSrcData+i);
		}
		else if(*(pSrcData+i) < nMinData)
		{
			nMinData = *(pSrcData+i);
		}
	}
	*pMaxData = nMaxData;
	*pMinData = nMinData;
	if(nMaxData > nMinData)
	{
		return nMaxData - nMinData;
	}
	else
	{
		return 0;
	}
}

DLL_API short WINAPI GetAmpData(const short* pSrcData,ULONG nSrcDataLen)
{
	short nMaxData = -MAX_DATA,nMinData = MAX_DATA;

	for(ULONG i=0;i<nSrcDataLen;i++)
	{
		if(*(pSrcData+i) > nMaxData)
		{
			nMaxData = *(pSrcData+i);
		}
		else if(*(pSrcData+i) < nMinData)
		{
			nMinData = *(pSrcData+i);
		}
	}
	return nMaxData - nMinData;
}
//*****************************************Automotive********************************************************************************************************************
//Secondary Ignition Plug Lead
DLL_API double WINAPI AMCalSecondaryIgnitionPlugLead(const short* pSrcData,ULONG nSrcDataLen,double dbTimeInterval,double* pdbDUR,short* pVpp,double* pdbBurnTime,short* pBurnVolt)
{
	ULONG i = 0;
	short nMaxData = -MAX_DATA,nMinData = MAX_DATA,nTmpData = 0;
	ULONG nMaxIndex = 0,nMinIndex = 0,nTmpIndex = 0,nBurnIndex = 0;
//�ҷ��ֵ(��������ѹ = nMaxData���պϿ�ʼ�� = nMinIndex)
	for(i=0;i<nSrcDataLen;i++)
	{
		if(*(pSrcData+i) > nMaxData)
		{
			nMaxData = *(pSrcData+i);
			nMaxIndex = i;
		}
		else if(*(pSrcData+i) < nMinData)
		{
			nMinData = *(pSrcData+i);
			nMinIndex = i;
		}
	}

	if(abs(nMaxData) < abs(nMinData))
	{
		nTmpData = nMaxData;
		nMaxData = nMinData;
		nMinData = nTmpData;
		//
		nTmpIndex = nMaxIndex;
		nMaxIndex = nMinIndex;
		nMinIndex = nTmpIndex;
	}
	*pVpp = abs(nMaxData - nMinData);//�õ����ֵ
	if(nMaxIndex > nMinIndex)
	{
		*pdbDUR = (nMaxIndex - nMinIndex) * dbTimeInterval;//��ñպϽǳ���
	}
	else
	{
		nMaxIndex = nMinIndex;
		*pdbDUR = 0;
	}
//��ʼ��ȼ�ս�����(nBurnIndex)
	nTmpData = 0;
	for(i = nMaxIndex+50;i<nSrcDataLen;i++)
	{
		if(abs(*(pSrcData+i)) > nTmpData)
		{
			nTmpData = abs(*(pSrcData+i));
			nBurnIndex = i;
		}
	}
	if(nBurnIndex > nMaxIndex)
	{
		*pdbBurnTime = (nBurnIndex - nMaxIndex) * dbTimeInterval;//���ȼ��ʱ��
	}
	else
	{
		nBurnIndex = nMaxIndex;
		*pdbBurnTime = 0;
	}
//��ȼ�յ�ѹ
	long nSum = 0;
	for(i=nMaxIndex+1;i<nBurnIndex;i++)
	{
		nSum += *(pSrcData+i);
	}
	if(nBurnIndex > nMaxIndex)
	{
		*pBurnVolt = short(nSum/(nBurnIndex-nMaxIndex));
	}
	else
	{
		*pBurnVolt = 0;
	}
//
	return 1;
}
