// HTSoftDll.cpp : Defines the entry point for the DLL application.
//


/*
函数 dsoSFChooseData 多加了个参数 "ULONG nSrcDataLen"

20100203
增加函数dsoCalPassFailData
20100204
增加函数dsoSFPassFail
20100221
增加函数 dsoSFProcessALTData4CH--暂时无用














*/
#include "stdafx.h"

#ifndef FILE_TEST
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}
#endif

//软件找触发
ULONG SoftFindTrigger(WORD* SourceData,WORD* BufferData,PCONTROLDATA pControl,double dInsertNUM)
{//SourceData 采集深度  BufferData插值之后数据长度要求其为SourceData 最少4倍
    BOOL isFind=FALSE;
	#define  ERROR_TRIGGER  5
	int sum=0;
	char pErroneous[ERROR_TRIGGER];
	memset(pErroneous,0,ERROR_TRIGGER*sizeof(char));
    double p=pControl->nHTriggerPos/100.0;
    ULONG nDataLen = pControl->nReadDataLen;
    ULONG startFindPoint,endPoint;
    ULONG nTriggerIndex = 0;
    ULONG nSourceDataLen=0;//源数据长度
    nSourceDataLen= ULONG(dInsertNUM*nDataLen+1<BUF_40K_LEN?(dInsertNUM*(nDataLen-2*SIN_FACTOR-1)):(BUF_40K_LEN-dInsertNUM));
    USHORT nStep= 1, nResult = MAX_DATA,nPreResult = MAX_DATA;
    USHORT TriggerLever = MAX_DATA-pControl->nVTriggerPos;//MAX_DATA-pControl->nVTriggerPos to pControl->nVTriggerPos
    startFindPoint  =(ULONG)(nDataLen*p)+nStep;//开始索引  计算的最小值应该是nStep
    endPoint        =nSourceDataLen-ULONG(nDataLen*(1-p)+nStep+1);//结束索引  计算的最大值应该是nSourceDataLen-nStep-1
    USHORT  nLastResult=*(SourceData+startFindPoint);
    if(pControl->nTriggerSlope==RISE||pControl->nTriggerSlope==FALL)
    {
        for(;startFindPoint<=endPoint;startFindPoint++)
        {           
            nResult = abs(*(SourceData+startFindPoint) - TriggerLever);
            if(nResult < 10)//接近触发点
            {
				int tempx;
                if(nResult==nLastResult)
                {
                    continue;
                }

                nLastResult=nResult;
                if(RISE == pControl->nTriggerSlope)  //上升沿
                {

                    if((*(SourceData+startFindPoint-nStep))<(*(SourceData+startFindPoint)))
                    {
						memmove(pErroneous,pErroneous+1,(ERROR_TRIGGER-1)*sizeof(char));
						pErroneous[ERROR_TRIGGER-1]=1;
						sum=0;
						for(int i=0;i<ERROR_TRIGGER;i++)
							sum+=pErroneous[i];
                        if(nResult == 0&&sum-ERROR_TRIGGER<=2)//与触发位置相等，直接退出
                        {
                            
							for(tempx=1;tempx<100;tempx++)
							{
								if(*(SourceData+startFindPoint+tempx)!=*(SourceData+startFindPoint))
									break;
							}
                   
							nTriggerIndex=startFindPoint+tempx/2;
							isFind=TRUE;
                            break;
                        }
                        else if(nResult < nPreResult)//找到最接近的点
                        {
                            nPreResult = nResult;
                            nTriggerIndex = startFindPoint;
                        }
                    }
					else
					{
						memmove(pErroneous,pErroneous+1,(ERROR_TRIGGER-1)*sizeof(char));
						pErroneous[ERROR_TRIGGER-1]=-1;
					}
                }
                else
                {
                    if((*(SourceData+startFindPoint-nStep))>(*(SourceData+startFindPoint)))
                    {
						memmove(pErroneous,pErroneous+1,(ERROR_TRIGGER-1)*sizeof(char));
						pErroneous[ERROR_TRIGGER-1]=-1;
						sum=0;
						for(int i=0;i<ERROR_TRIGGER;i++)
							sum+=pErroneous[i];
                        if(nResult == 0&&sum+ERROR_TRIGGER<=2)//与触发位置相等，直接退出
                        {
							for(tempx=1;tempx<100;tempx++)
							{
								if(*(SourceData+startFindPoint+tempx)!=*(SourceData+startFindPoint))
									break;
							}
                   
							nTriggerIndex=startFindPoint+tempx/2;
                            isFind=TRUE;
                            break;
                        }
                        else if(nResult < nPreResult)//找到最接近的点
                        {
                            nPreResult = nResult;
                            nTriggerIndex = startFindPoint;
                        }
                    }
					else
					{
						memmove(pErroneous,pErroneous+1,(ERROR_TRIGGER-1)*sizeof(char));
						pErroneous[ERROR_TRIGGER-1]=1;
					}
				
						
                } 
			
                
            }
        }
    }
    if(!isFind)//没有找到
    {
        if(!nTriggerIndex)
            nTriggerIndex=ULONG(nSourceDataLen*p+1);//没有找到触发点，则直接等于开始找触发的起始点
    }
	if(*(SourceData+nTriggerIndex)<*(SourceData+nTriggerIndex+50))
	{
		int xxxxxx=10;
	}
    ULONG nStartCopyPoint=nTriggerIndex-ULONG(nDataLen*p+0.5);
    if(nStartCopyPoint+nDataLen>=nSourceDataLen)
    {
        nStartCopyPoint=nSourceDataLen-nDataLen-1;
    }
    //memcpy(BufferData,SourceData+nStartCopyPoint,(nDataLen)*sizeof(WORD));
    return nStartCopyPoint;
}
WORD SoftFindTriggerCopy(WORD* SourceData,WORD* BufferData,PCONTROLDATA pControl,ULONG TriggerPoint)
{
	USHORT* mid_data;
	ULONG LeftPoints,nDataLen,i;

	nDataLen = pControl->nReadDataLen;
	mid_data = new USHORT[nDataLen];
	LeftPoints = ULONG((pControl->nHTriggerPos)*nDataLen/100.0);

	if(LeftPoints>(nDataLen-1))
	{
		LeftPoints = nDataLen - 1;
	}
	for(i=0;i<LeftPoints;i++)
	{
		if(TriggerPoint-1-i>=0)
		{
			*(mid_data+LeftPoints-1-i) = *(SourceData+TriggerPoint-1-i);
		}
		else
		{
			break;
		}
	}
	for(i=0;i<nDataLen-LeftPoints;i++)
	{
		if(TriggerPoint+i<(nDataLen-DEF_READ_DATA_LEN + BUF_40K_LEN))
		{
			*(mid_data+LeftPoints+i) = *(SourceData+TriggerPoint+i);
		}
		else
		{
			break;
		}
	}
	for(i=0;i<nDataLen;i++)
	{
		*(BufferData+i) = *(mid_data+i);
	}
	delete mid_data;
	return 1;
}

void CalSinSheet(double div_data,double* dbSinSheet)
{
	ULONG i=0;
	long j=0;
	double dbTemp = 0;
	for(i=1;i<div_data;i++)
	{
		dbTemp = i/div_data;
		for(j=-SIN_FACTOR;j<=SIN_FACTOR;j++)
		{
			 dbSinSheet[(i-1)*21+(j+SIN_FACTOR)] = (sin((dbTemp-j)*PI)/((dbTemp-j)*PI));;//dbSinSheet[i-1][j+SIN_FACTOR] = (sin((dbTemp-j)*PI)/((dbTemp-j)*PI));
		}
	}
}
/*
void InsertDataStep(WORD* SourceData,WORD* pBuffer,double div_data,PCONTROLDATA pControl)
{
	ULONG i = 0,count = 0;
	ULONG midTriggerAddress = 0,OutTriggerAddress = 0;
	ULONG nDataLen = DEF_READ_DATA_LEN;

	nDataLen = pControl->nReadDataLen;
	if(pControl->nHTriggerPos == 100)
	{
		midTriggerAddress = nDataLen - 1;
	}
	else
	{
		midTriggerAddress = ULONG(pControl->nHTriggerPos * nDataLen /100.0);
	}

	if(midTriggerAddress < 40)
	{
		midTriggerAddress = 40;
	}

	if(pControl->nTimeDIV <= MAX_SF_T_TIMEDIV)//软件找Trigger
	{
		count = nDataLen - DEF_READ_DATA_LEN + BUF_40K_LEN;
		if(pControl->nHTriggerPos == 100)
		{
			OutTriggerAddress = count - 1;
		}
		else
		{
			OutTriggerAddress = ULONG(pControl->nHTriggerPos * (count/100.0));
		}
	}
	else
	{
		count = nDataLen;
		OutTriggerAddress = midTriggerAddress;
	}
	for(i=0;i<count-OutTriggerAddress;i++)
	{
		*(pBuffer+OutTriggerAddress+i)=*(SourceData+midTriggerAddress+LONG(i/div_data));
	}
	for(i=1;i<=OutTriggerAddress;i++)
	{
		if(midTriggerAddress >= ULONG(i/div_data))
		{
			*(pBuffer+OutTriggerAddress-i)=*(SourceData+midTriggerAddress-ULONG(i/div_data));
		}
	}
}
*/
void InsertDataStep(WORD* SourceData,WORD* BufferData,double dbInsertNum,PCONTROLDATA pControl)
{
	ULONG i=0,j=0,nReadDataLen = 0;
    short k=0;
    //short *TempData;
    ULONG StartPoint =0;//起始点
    ULONG EndPoint=0;//结束点
    double sum = 0;
    ULONG nTotalNum;//输出总点数
    ULONG nCounter = 0;//源数据中总共进行插值的点数
    nReadDataLen = pControl->nReadDataLen;
    nTotalNum = BUF_INSERT_LEN;//插值Buffer
    double p=pControl->nHTriggerPos/100.0;
	if(nReadDataLen*dbInsertNum+1<nTotalNum)
    {
        StartPoint=0;//左右预留数据用来插值注意：虽然可以从0开始插值但是无意义，从
        EndPoint=nReadDataLen-1;
    }
    else
    {
        StartPoint=ULONG(p*(nReadDataLen-nTotalNum/dbInsertNum));
        EndPoint=ULONG(p*nReadDataLen+(1-p)*nTotalNum/dbInsertNum-1);
    }        
	nCounter=EndPoint-StartPoint+1;//
    for(i=0;i<nCounter*dbInsertNum;i++)	//原始值需要+1
	{    
		if((StartPoint+ULONG(i/dbInsertNum)>=nReadDataLen)||i>=nTotalNum)	
			break;
        BufferData[i] = *(SourceData+StartPoint+ULONG(i/dbInsertNum));
	}
}
void InsertDataLine(WORD* SourceData,WORD* pBuffer,double div_data,PCONTROLDATA pControl)
{
	ULONG i = 0,j = 0,count = 0;
	double gain = 0,offset = 0;
	WORD pre_data = 0,last_data = 0;
	ULONG midTriggerAddress = 0,OutTriggerAddress = 0,nDataLen = DEF_READ_DATA_LEN;

#ifdef FILE_TEST
	{
		CStdioFile file;	
		CString str;
		if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
		{
			str=_T("Cann't save Test file");
			//					AfxMessageBox(str);
			return;
		}
		file.SeekToEnd();
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			str.Format(_T("In the InsertDataLine(Pre Deal)::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
		}
		file.WriteString(str);
		file.Close();
		Sleep(5);
	}
#endif

	nDataLen = pControl->nReadDataLen;
	if(pControl->nHTriggerPos == 100)
	{
		midTriggerAddress = nDataLen - 1;
	}
	else
	{
		midTriggerAddress = ULONG(pControl->nHTriggerPos * nDataLen / 100.0 + 1);
	}

	if(midTriggerAddress < 40)
	{
		midTriggerAddress = 40;
	}

	if(pControl->nTimeDIV <= MAX_SF_T_TIMEDIV)// < 软件找触发
	{
		count = nDataLen - DEF_READ_DATA_LEN + BUF_40K_LEN;
		if(pControl->nHTriggerPos == 100)
		{
			OutTriggerAddress = count - 1;
		}
		else
		{
			OutTriggerAddress = ULONG(pControl->nHTriggerPos * (count / 100.0));
		}
	}
	else
	{
		count = nDataLen;
		OutTriggerAddress = midTriggerAddress;
	}

#ifdef FILE_TEST
	{
		CStdioFile file;	
		CString str;
		if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
		{
			str=_T("Cann't save Test file");
			//					AfxMessageBox(str);
			return;
		}
		file.SeekToEnd();
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			str.Format(_T("In the InsertDataLine(start Insert)::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
		}
		file.WriteString(str);
		file.Close();
		Sleep(5);
	}
#endif
//	memset(pBuffer,0,count*sizeof(WORD));
//	return;


	pre_data=*(SourceData+midTriggerAddress);
	ULONG nRight = (ULONG)(1.0*(count-OutTriggerAddress)/div_data + 1);
	for(j=0;j<nRight;j++)//触发点右边数据
	{
		if (j+1+midTriggerAddress >= nDataLen)
		{
			last_data = *(SourceData+(nDataLen-1));
		}
		else
		{
			last_data=*(SourceData+midTriggerAddress+j+1);
		}
		gain=1.0*(last_data-pre_data)/div_data;
		offset=pre_data;
		for(i=0;1.0*i<div_data;i++)
		{
			ULONG nValue = OutTriggerAddress+(ULONG)(j*div_data)+i;
			WORD nData = WORD(gain*i+offset);
			if(nValue<count)
			{
				*(pBuffer+nValue) = nData;
			}
			else
			{
				break;
			}
		}
		pre_data=last_data;
	}

#ifdef FILE_TEST
	{
		CStdioFile file;	
		CString str;
		if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
		{
			str=_T("Cann't save Test file");
			//					AfxMessageBox(str);
			return;
		}
		file.SeekToEnd();
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			str.Format(_T("In the InsertDataLine(Right Over)::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
		}
		file.WriteString(str);
		file.Close();
		Sleep(5);
	}
#endif

	pre_data=*(SourceData+midTriggerAddress);
	ULONG nLeft = (ULONG)(1.0*OutTriggerAddress/div_data);

#ifdef FILE_TEST
	{
		CStdioFile file;	
		CString str;
		if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
		{
			str=_T("Cann't save Test file");
			//					AfxMessageBox(str);
			return;
		}
		file.SeekToEnd();
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			CString strInfo;
			strInfo.Format(_T("midTriggerAddress:%-8ld,OutTriggerAddress:%-8ld,nDataLen;%-8ld,nLeft:%-8ld,div_data:%8.3f\n"),midTriggerAddress,OutTriggerAddress,nDataLen,nLeft,div_data);
			str.Format(_T("Time::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
			str = strInfo + str;
		}
		file.WriteString(str);
		file.Close();
		Sleep(5);
	}
#endif

	ULONG nValue;
	WORD nData;
	for(j=0;j<nLeft;j++)//触发点左边数据
	{
		if (midTriggerAddress <= (j+1))
		{
			last_data = *SourceData;
		}
		else
		{
			last_data=*(SourceData+midTriggerAddress-j-1);
		}
		gain=1.0*(last_data-pre_data)/div_data;
		offset=pre_data;
		for(i=0;1.0*i<div_data;i++)
		{
			nValue = ULONG(j*div_data+i);
			nData = WORD(gain*i+offset);
			if(OutTriggerAddress >= nValue)
			{
//				*(pBuffer+OutT riggerAddress-nValue) = 12;
				*(pBuffer+OutTriggerAddress-nValue) = (WORD)(nData);
			}
			else
			{
				break;
			}
		}
		pre_data=last_data;
	}
#ifdef FILE_TEST
	{
		CStdioFile file;	
		CString str;
		if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
		{
			str=_T("Cann't save Test file");
			//					AfxMessageBox(str);
			return;
		}
		file.SeekToEnd();
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			str.Format(_T("In the InsertDataLine(Left Over)::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
		}
		file.WriteString(str);
		file.Close();
		Sleep(5);
	}
#endif

	*pBuffer = *(pBuffer+1);
}

void InsertDataSin(WORD* SourceData,WORD* BufferData,PCONTROLDATA pControl, double dbInsertNum,double* dbSinSheet,USHORT nCH)//正弦插值
{
    ULONG i=0,j=0,nReadDataLen = 0;
    short k=0;
    //short *TempData;
    ULONG StartPoint =0;//起始点
    ULONG EndPoint=0;//结束点
    double sum = 0;
    ULONG nTotalNum;//输出总点数
    ULONG nCounter = 0;//源数据中总共进行插值的点数
    nReadDataLen = pControl->nReadDataLen;
    nTotalNum = BUF_INSERT_LEN;//插值Buffer
    //TempData = (short*)malloc(sizeof(short)*nTotalNum);  //zhang
    //memset(TempData,0,sizeof(short)*nTotalNum);
    double p=pControl->nHTriggerPos/100.0;
    if(nReadDataLen*dbInsertNum+1<nTotalNum)
    {
        StartPoint=SIN_FACTOR;//左右预留数据用来插值注意：虽然可以从0开始插值但是无意义，从
        EndPoint=nReadDataLen-1-SIN_FACTOR;
        nCounter=EndPoint-StartPoint+1;//
        for(i=0;i<nCounter;i++)	//原始值需要+1
		{
            //TempData[ULONG(i*dbInsertNum)] = *(SourceData+StartPoint+i);
            BufferData[ULONG(i*dbInsertNum)] = *(SourceData+StartPoint+i);
		}
    }
    else
    {
        int nOffset=0;//为了预留SIN_FACTOR个插值计算点
        StartPoint=ULONG(p*(nReadDataLen-nTotalNum/dbInsertNum));
        EndPoint=ULONG(p*nReadDataLen+(1-p)*nTotalNum/dbInsertNum-1);
        if(StartPoint<SIN_FACTOR)
        {
            nOffset+=SIN_FACTOR-StartPoint;
        }
        else if(nTotalNum-EndPoint-1<SIN_FACTOR)
        {
            nOffset-=SIN_FACTOR-(nTotalNum-EndPoint-1);
        }
        StartPoint+=nOffset;//最小值SIN_Factor  //End-Start=nTotalNum/dbInsertNum-1
        EndPoint+=nOffset;//最大值nReadDataLen-1-SIN_Factor
        nCounter=EndPoint-StartPoint;//删除+1 //少了最后一个插值点的计算
        for(i=0;i<nCounter+1;i++)	//原始值要+1
		{
            //TempData[ULONG(i*dbInsertNum)] = *(SourceData+StartPoint+i); //SourceData+StartPoint+i的最大索引值是EndPoint;最小索引值StartPoint
            BufferData[ULONG(i*dbInsertNum)] = *(SourceData+StartPoint+i); //SourceData+StartPoint+i的最大索引值是EndPoint;最小索引值StartPoint
		}
    }
    for(i=0;i<nCounter;i++)//注意插值的时候最后一个点后面不插这里少了一个插值点导致100%的时候有零电平
    {
        unsigned int step=ULONG((i+1)*dbInsertNum)-ULONG(i*dbInsertNum);//实际插值点数是step-1
        if(*(SourceData+StartPoint)% MAX_DATA== 0 && *(SourceData+StartPoint+1) %MAX_DATA==0 )//如果前后都是255或者0，则本段内数据都是255或者0
        {
            for(j=1;j<step;j++)
            {
                //TempData[ULONG(i*dbInsertNum)+j] = *(SourceData+StartPoint);
                BufferData[ULONG(i*dbInsertNum)+j] = *(SourceData+StartPoint);
            }
        }
        else
        {
            for(j=1;j<step;j++)
            {
                sum = 0;
                for(k=(-SIN_FACTOR);k<=SIN_FACTOR;k++)
                {
                    if(StartPoint+k >= nReadDataLen)
                    {
                        sum = sum + *(SourceData+nReadDataLen-1) * dbSinSheet[(j-1)*21+(k+SIN_FACTOR)];//dbSinSheet[j-1][k+SIN_FACTOR];//查表
                    }
                    else if(StartPoint+k < 0)
                    {
                        sum = sum + *(SourceData) * dbSinSheet[(j-1)*21+(k+SIN_FACTOR)];//dbSinSheet[j-1][k+SIN_FACTOR];//查表
                    }
                    else
                    {
                        sum = sum + *(SourceData+StartPoint+k) * dbSinSheet[(j-1)*21+(k+SIN_FACTOR)];//dbSinSheet[j-1][k+SIN_FACTOR];//查表
                    }
                }
                //TempData[ULONG(i*dbInsertNum)+j] = short(sum);
                BufferData[ULONG(i*dbInsertNum)+j] = short(sum);
            }
        }
        StartPoint++;
    }
    //memcpy(BufferData,TempData,sizeof(WORD)*nTotalNum);
    //free(TempData);
}

void GetTriggerPos(USHORT nTimeDIV,USHORT nTriggerSource,ULONG* pState)//for SDK and launch
{
	if(nTimeDIV == 0)
	{
		if(nTriggerSource == CH1)
		{
			*(pState+CH2) = *(pState+CH1) + 1000;
			*(pState+CH3) = *(pState+CH1) - 0;//2250;
			*(pState+CH4) = *(pState+CH1) - 1300;//3700;
		}
		else if(nTriggerSource == CH2)
		{
			*(pState+CH1) = *(pState+CH2) - 1000;
			*(pState+CH3) = *(pState+CH2) - 1000;
			*(pState+CH4) = *(pState+CH2) - 2200;
		}
		else if(nTriggerSource == CH3)
		{
			*(pState+CH1) = *(pState+CH3) + 0;
			*(pState+CH2) = *(pState+CH3) + 1000;
			*(pState+CH4) = *(pState+CH3) - 1200;
		}
		else if(nTriggerSource == CH4)
		{
			*(pState+CH1) = *(pState+CH4) + 1250;
			*(pState+CH2) = *(pState+CH4) + 2250;
			*(pState+CH3) = *(pState+CH4) + 1250;
		}
	}
	else if(nTimeDIV == 1)
	{
		if(nTriggerSource == CH1)
		{
			*(pState+CH2) = *(pState+CH1) + 0;
			*(pState+CH3) = *(pState+CH1) - 0;
			*(pState+CH4) = *(pState+CH1) - 0;
		}
		else if(nTriggerSource == CH2)
		{
			*(pState+CH1) = *(pState+CH2) + 400;
			*(pState+CH3) = *(pState+CH2) - 750;
			*(pState+CH4) = *(pState+CH2) - 1450;
		}
		else if(nTriggerSource == CH3)
		{
			*(pState+CH1) = *(pState+CH3) + 1150;
			*(pState+CH2) = *(pState+CH3) + 750;
			*(pState+CH4) = *(pState+CH3) - 750;
		}
		else if(nTriggerSource == CH4)
		{
			*(pState+CH1) = *(pState+CH4) + 1850;
			*(pState+CH2) = *(pState+CH4) + 1450;
			*(pState+CH3) = *(pState+CH4) + 750;
		}
	}
	else if(nTimeDIV == 2)
	{
		if(nTriggerSource == CH1)
		{
			*(pState+CH2) = *(pState+CH1) - 200;
			*(pState+CH3) = *(pState+CH1) - 600;
			*(pState+CH4) = *(pState+CH1) - 950;
		}
		else if(nTriggerSource == CH2)
		{
			*(pState+CH1) = *(pState+CH2) + 200;
			*(pState+CH3) = *(pState+CH2) - 400;
			*(pState+CH4) = *(pState+CH2) - 750;
		}
		else if(nTriggerSource == CH3)
		{
			*(pState+CH1) = *(pState+CH3) + 600;
			*(pState+CH2) = *(pState+CH3) + 400;
			*(pState+CH4) = *(pState+CH3) - 350;
		}
		else if(nTriggerSource == CH4)
		{
			*(pState+CH1) = *(pState+CH4) + 950;
			*(pState+CH2) = *(pState+CH4) + 750;
			*(pState+CH3) = *(pState+CH4) + 350;
		}
	}
	else if(nTimeDIV == 3)
	{
		if(nTriggerSource == CH1)
		{
			*(pState+CH2) = *(pState+CH1) - 60;
			*(pState+CH3) = *(pState+CH1) - 260;
			*(pState+CH4) = *(pState+CH1) - 400;
		}
		else if(nTriggerSource == CH2)
		{
			*(pState+CH1) = *(pState+CH2) + 10;
			*(pState+CH3) = *(pState+CH2) - 350;
			*(pState+CH4) = *(pState+CH2) - 590;
		}
		else if(nTriggerSource == CH3)
		{
			*(pState+CH1) = *(pState+CH3) + 360;
			*(pState+CH2) = *(pState+CH3) + 350;
			*(pState+CH4) = *(pState+CH3) - 240;
		}
		else if(nTriggerSource == CH4)
		{
			*(pState+CH1) = *(pState+CH4) + 600;
			*(pState+CH2) = *(pState+CH4) + 590;
			*(pState+CH3) = *(pState+CH4) + 240;
		}
	}
	else if(nTimeDIV == 4)
	{
		if(nTriggerSource == CH1)
		{
			*(pState+CH2) = *(pState+CH1) - 0;
			*(pState+CH3) = *(pState+CH1) - 200;
			*(pState+CH4) = *(pState+CH1) - 360;
		}
		else if(nTriggerSource == CH2)
		{
			*(pState+CH1) = *(pState+CH2) + 0;
			*(pState+CH3) = *(pState+CH2) - 200;
			*(pState+CH4) = *(pState+CH2) - 360;
		}
		else if(nTriggerSource == CH3)
		{
			*(pState+CH1) = *(pState+CH3) + 200;
			*(pState+CH2) = *(pState+CH3) + 200;
			*(pState+CH4) = *(pState+CH3) - 160;
		}
		else if(nTriggerSource == CH4)
		{
			*(pState+CH1) = *(pState+CH4) + 360;
			*(pState+CH2) = *(pState+CH4) + 360;
			*(pState+CH3) = *(pState+CH4) + 160;
		}
	}
	else if(nTimeDIV == 5)
	{
		if(nTriggerSource == CH1)
		{
			*(pState+CH2) = *(pState+CH1) - 0;
			*(pState+CH3) = *(pState+CH1) - 200;
			*(pState+CH4) = *(pState+CH1) - 360;
		}
		else if(nTriggerSource == CH2)
		{
			*(pState+CH1) = *(pState+CH2) + 0;
			*(pState+CH3) = *(pState+CH2) - 200;
			*(pState+CH4) = *(pState+CH2) - 360;
		}
		else if(nTriggerSource == CH3)
		{
			*(pState+CH1) = *(pState+CH3) + 200;
			*(pState+CH2) = *(pState+CH3) + 200;
			*(pState+CH4) = *(pState+CH3) - 160;
		}
		else if(nTriggerSource == CH4)
		{
			*(pState+CH1) = *(pState+CH4) + 360;
			*(pState+CH2) = *(pState+CH4) + 360;
			*(pState+CH3) = *(pState+CH4) + 160;
		}
	}
	else if(nTimeDIV == 6)
	{
		if(nTriggerSource == CH1)
		{
			*(pState+CH2) = *(pState+CH1) - 0;
			*(pState+CH3) = *(pState+CH1) - 200;
			*(pState+CH4) = *(pState+CH1) - 360;
		}
		else if(nTriggerSource == CH2)
		{
			*(pState+CH1) = *(pState+CH2) + 0;
			*(pState+CH3) = *(pState+CH2) - 200;
			*(pState+CH4) = *(pState+CH2) - 360;
		}
		else if(nTriggerSource == CH3)
		{
			*(pState+CH1) = *(pState+CH3) + 200;
			*(pState+CH2) = *(pState+CH3) + 200;
			*(pState+CH4) = *(pState+CH3) - 160;
		}
		else if(nTriggerSource == CH4)
		{
			*(pState+CH1) = *(pState+CH4) + 360;
			*(pState+CH2) = *(pState+CH4) + 360;
			*(pState+CH3) = *(pState+CH4) + 160;
		}
	}
	else if(nTimeDIV == 7)
	{
		if(nTriggerSource == CH1)
		{
			*(pState+CH2) = *(pState+CH1) - 0;
			*(pState+CH3) = *(pState+CH1) - 200;
			*(pState+CH4) = *(pState+CH1) - 360;
		}
		else if(nTriggerSource == CH2)
		{
			*(pState+CH1) = *(pState+CH2) + 0;
			*(pState+CH3) = *(pState+CH2) - 200;
			*(pState+CH4) = *(pState+CH2) - 360;
		}
		else if(nTriggerSource == CH3)
		{
			*(pState+CH1) = *(pState+CH3) + 200;
			*(pState+CH2) = *(pState+CH3) + 200;
			*(pState+CH4) = *(pState+CH3) - 160;
		}
		else if(nTriggerSource == CH4)
		{
			*(pState+CH1) = *(pState+CH4) + 360;
			*(pState+CH2) = *(pState+CH4) + 360;
			*(pState+CH3) = *(pState+CH4) + 160;
		}
	}
	else// if(nTimeDIV == )
	{
		for(int i=0;i<MAX_CH_NUM;i++)
		{
			if(i == nTriggerSource)
			{
				continue;
			}
			*(pState+i) = *(pState+nTriggerSource);
		}
	}
}


void InsertData(WORD* SourceData,WORD* BufferData,short InsertMode,double div_data,PCONTROLDATA pControl,WORD nCH,ULONG* pState)
{/*
	WORD* pBuffer;
	if(pControl->nTimeDIV <= MAX_SF_T_TIMEDIV)// 软件找触发
	{
		pBuffer = new WORD[pControl->nReadDataLen - DEF_READ_DATA_LEN + BUF_40K_LEN];
	}
	else
	{
		pBuffer = BufferData;
	}
	*/
	//InsertMode = 10;
	if(pControl->nTimeDIV > MAX_SINE_TIMEDIV)//最后4个档位才使用多种差值方
	{
		InsertMode = 2;
	}
	if(InsertMode==0)//台阶
	{
		InsertDataStep(SourceData,BufferData,div_data,pControl);
	}
	else if(InsertMode==1)//斜线
	{
		InsertDataLine(SourceData,BufferData,div_data,pControl);
	}
	else//if(InsertMode == 2)//SIN内插
	{
		//计算插值表
		double* dbSinSheet = new double[105000];
		CalSinSheet(div_data,dbSinSheet);
		InsertDataSin(SourceData,BufferData,pControl,div_data,dbSinSheet,0);
		delete dbSinSheet;
	}
/*
	if(pControl->nTimeDIV <= MAX_SF_T_TIMEDIV)//找触发
	{
		if(pControl->nALT == 1)//交替
		{
			if(nCH == pControl->nTriggerSource)
			{
				*pState = SoftFindTrigger(pBuffer,BufferData,pControl);
			}
		}
		else
		{
			if(pControl->nTriggerSource < MAX_CH_NUM)
			{
				if(nCH == pControl->nTriggerSource)
				{
					*(pState+nCH) = SoftFindTrigger(pBuffer,BufferData,pControl);
					GetTriggerPos(pControl->nTimeDIV,pControl->nTriggerSource,pState);

				}
				else
				{
					SoftFindTriggerCopy(pBuffer,BufferData,pControl,*(pState+nCH));
				}
			}
			else//EXT,EXT/10
			{
				ULONG n = ULONG((pControl->nReadDataLen - DEF_READ_DATA_LEN + BUF_40K_LEN) * (pControl->nHTriggerPos/100.0));
				SoftFindTriggerCopy(pBuffer,BufferData,pControl,n);
			}
		}
		delete pBuffer;
	}
	*/
}

//用于修改4250软件找触发死机
void InsertData_DSO4250(WORD* SourceData,WORD* BufferData,short InsertMode,double div_data,PCONTROLDATA Control,WORD nCH,ULONG* pState)
{
	WORD* pBuffer;
	if(Control->nTimeDIV <= MAX_SF_T_TIMEDIV)// 软件找触发
	{
		pBuffer = new WORD[Control->nReadDataLen - DEF_READ_DATA_LEN + BUF_40K_LEN];
		ULONG nBufLen = Control->nReadDataLen - DEF_READ_DATA_LEN + BUF_40K_LEN;
		memset(pBuffer,0,nBufLen*sizeof(WORD));
		
		if(Control->nTimeDIV > MAX_SINE_TIMEDIV)//最后4个档位才使用多种差值方
		{
			InsertMode = 2;
		}
		if(InsertMode==0)//台阶
		{
#ifdef FILE_TEST
		{
			CStdioFile file;	
			CString str;
			if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
			{
				str=_T("Cann't save Test file");
				//					AfxMessageBox(str);
				return;
			}
			file.SeekToEnd();
			{
				SYSTEMTIME st;
				GetLocalTime(&st);
				str.Format(_T("InsertDataStep_In::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
			}
			file.WriteString(str);
			file.Close();
			Sleep(5);
		}
#endif

			InsertDataStep(SourceData,pBuffer,div_data,Control);

#ifdef FILE_TEST
		{
			CStdioFile file;	
			CString str;
			if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
			{
				str=_T("Cann't save Test file");
				//					AfxMessageBox(str);
				return;
			}
			file.SeekToEnd();
			{
				SYSTEMTIME st;
				GetLocalTime(&st);
				str.Format(_T("InsertDataStep_Out::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
			}
			file.WriteString(str);
			file.Close();
			Sleep(5);
		}
#endif
		}
		else if(InsertMode==1)//斜线
		{
#ifdef FILE_TEST
		{
			CStdioFile file;	
			CString str;
			if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
			{
				str=_T("Cann't save Test file");
				//					AfxMessageBox(str);
				return;
			}
			file.SeekToEnd();
			{
				SYSTEMTIME st;
				GetLocalTime(&st);
				str.Format(_T("InsertDataLine_In::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
			}
			file.WriteString(str);
			file.Close();
			Sleep(5);
		}
#endif

			InsertDataLine(SourceData,pBuffer,div_data,Control);

#ifdef FILE_TEST
		{
			CStdioFile file;	
			CString str;
			if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
			{
				str=_T("Cann't save Test file");
				//					AfxMessageBox(str);
				return;
			}
			file.SeekToEnd();
			{
				SYSTEMTIME st;
				GetLocalTime(&st);
				str.Format(_T("InsertDataLine_Out::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
			}
			file.WriteString(str);
			file.Close();
			Sleep(5);
		}
#endif
		}
		else//if(InsertMode == 2)//SIN内插
		{
#ifdef FILE_TEST
		{
			CStdioFile file;	
			CString str;
			if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
			{
				str=_T("Cann't save Test file");
				//					AfxMessageBox(str);
				return;
			}
			file.SeekToEnd();
			{
				SYSTEMTIME st;
				GetLocalTime(&st);
				str.Format(_T("InsertDataSin_In::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
			}
			file.WriteString(str);
			file.Close();
			Sleep(5);
		}
#endif

			//计算插值表
			double* dbSinSheet = new double[105000];
			CalSinSheet(div_data,dbSinSheet);
			InsertDataSin(SourceData,pBuffer,Control,div_data,dbSinSheet,0);
			delete dbSinSheet;

#ifdef FILE_TEST
		{
			CStdioFile file;	
			CString str;
			if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
			{
				str=_T("Cann't save Test file");
				//					AfxMessageBox(str);
				return;
			}
			file.SeekToEnd();
			{
				SYSTEMTIME st;
				GetLocalTime(&st);
				str.Format(_T("InsertDataSin_Out::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
			}
			file.WriteString(str);
			file.Close();
			Sleep(5);
		}
#endif
		}	
		
		if(Control->nALT == 1)//交替
		{
			if(nCH == Control->nTriggerSource)
			{
#ifdef FILE_TEST
				{
					CStdioFile file;	
					CString str;
					if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
					{
						str=_T("Cann't save Test file");
						//					AfxMessageBox(str);
						return;
					}
					file.SeekToEnd();
					{
						SYSTEMTIME st;
						GetLocalTime(&st);
						str.Format(_T("SoftFindTrigger_In::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
					}
					file.WriteString(str);
					file.Close();
					Sleep(5);
				}
#endif

				*pState = SoftFindTrigger(pBuffer,BufferData,Control,4);//zhang  随便赋值因为DSO6104不会用这个

#ifdef FILE_TEST
				{
					CStdioFile file;	
					CString str;
					if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
					{
						str=_T("Cann't save Test file");
						//					AfxMessageBox(str);
						return;
					}
					file.SeekToEnd();
					{
						SYSTEMTIME st;
						GetLocalTime(&st);
						str.Format(_T("SoftFindTrigger_Out::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
					}
					file.WriteString(str);
					file.Close();
					Sleep(5);
				}
#endif
			}
		}
		else
		{
			if(Control->nTriggerSource < MAX_CH_NUM)
			{
				if(nCH == Control->nTriggerSource)
				{
#ifdef FILE_TEST
				{
					CStdioFile file;	
					CString str;
					if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
					{
						str=_T("Cann't save Test file");
						//					AfxMessageBox(str);
						return;
					}
					file.SeekToEnd();
					{
						SYSTEMTIME st;
						GetLocalTime(&st);
						str.Format(_T("SoftFindTrigger_In::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
					}
					file.WriteString(str);
					file.Close();
					Sleep(5);
				}
#endif

					*(pState+nCH) = SoftFindTrigger(pBuffer,BufferData,Control,4);  //zhang  随便赋值 因为DSO6104不会调用这个函数

#ifdef FILE_TEST
				{
					CStdioFile file;	
					CString str;
					if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
					{
						str=_T("Cann't save Test file");
						//					AfxMessageBox(str);
						return;
					}
					file.SeekToEnd();
					{
						SYSTEMTIME st;
						GetLocalTime(&st);
						str.Format(_T("SoftFindTrigger_Out::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
					}
					file.WriteString(str);
					file.Close();
					Sleep(5);
				}
#endif

#ifdef FILE_TEST
				{
					CStdioFile file;	
					CString str;
					if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
					{
						str=_T("Cann't save Test file");
						//					AfxMessageBox(str);
						return;
					}
					file.SeekToEnd();
					{
						SYSTEMTIME st;
						GetLocalTime(&st);
						str.Format(_T("GetTriggerPos_In::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
					}
					file.WriteString(str);
					file.Close();
					Sleep(5);
				}
#endif
				
#ifdef HANA_FPGA_SOFT_FIND_POS		//0x0248
					dsoGetSoftTriggerPosNew(Control,pState,0x0248);
#else
					GetTriggerPos(Control->nTimeDIV,Control->nTriggerSource,pState);
#endif

#ifdef FILE_TEST
				{
					CStdioFile file;	
					CString str;
					if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
					{
						str=_T("Cann't save Test file");
						//					AfxMessageBox(str);
						return;
					}
					file.SeekToEnd();
					{
						SYSTEMTIME st;
						GetLocalTime(&st);
						str.Format(_T("GetTriggerPos_Out::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
					}
					file.WriteString(str);
					file.Close();
					Sleep(5);
				}
#endif
				}
				else
				{
					SoftFindTriggerCopy(pBuffer,BufferData,Control,*(pState+nCH));
				}
			}
			else//EXT,EXT/10
			{
				ULONG n = ULONG((Control->nReadDataLen - DEF_READ_DATA_LEN + BUF_40K_LEN) * (Control->nHTriggerPos/100.0));
				SoftFindTriggerCopy(pBuffer,BufferData,Control,n);
			}
		}

		delete pBuffer;
	}
	else
	{
		pBuffer = BufferData;
#ifdef FILE_TEST
		{
			CStdioFile file;	
			CString str;
			if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
			{
				str=_T("Cann't save Test file");
				//					AfxMessageBox(str);
				return;
			}
			file.SeekToEnd();
			{
				SYSTEMTIME st;
				GetLocalTime(&st);
				str.Format(_T("InsertDataLine_In::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
			}
			file.WriteString(str);
			file.Close();
			Sleep(5);
		}
#endif

		InsertDataLine(SourceData,pBuffer,div_data,Control);

#ifdef FILE_TEST
		{
			CStdioFile file;	
			CString str;
			if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
			{
				str=_T("Cann't save Test file");
				//					AfxMessageBox(str);
				return;
			}
			file.SeekToEnd();
			{
				SYSTEMTIME st;
				GetLocalTime(&st);
				str.Format(_T("InsertDataLine_Out::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
			}
			file.WriteString(str);
			file.Close();
			Sleep(5);
		}
#endif
	}
}


//Export............................................................................................................................
//插值计算
DLL_API WORD WINAPI dsoSFInsert(WORD* SourceData,WORD* BufferData,PCONTROLDATA pControl,WORD nInsertMode,WORD nCH,ULONG* pState)
{
	ULONG i = 0;
	BOOL bSingleCH = FALSE;
	BOOL bCH[MAX_CH_NUM];
	double dbInsertNum = 0;
//CH1
	for(i=0;i<MAX_CH_NUM;i++)
	{
		if(((pControl->nCHSet >> i) & 0x01) == 1)
		{
			bCH[i] = TRUE;
		}
		else
		{
			bCH[i] = FALSE;
		}
	}
//
	if(nCH == CH1)
	{
		if(bCH[CH2] == FALSE)
		{
			bSingleCH = TRUE;
		}
	}
	else if(nCH == CH2)
	{
		if(bCH[CH1] == FALSE)
		{
			bSingleCH = TRUE;
		}
	}
	else if(nCH == CH3)
	{
		if(bCH[CH4] == FALSE)
		{
			bSingleCH = TRUE;
		}
	}
	else if(nCH == CH4)
	{
		if(bCH[CH3] == FALSE)
		{
			bSingleCH = TRUE;
		}
	}

#ifdef FILE_TEST
	{
		CStdioFile file;	
		CString str;
		if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
		{
			str=_T("Cann't save Test file");
			//					AfxMessageBox(str);
			return 0;
		}
		file.SeekToEnd();
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			str.Format(_T("dsoSFGetInsertNum_In::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
		}
		file.WriteString(str);
		file.Close();
		Sleep(5);
	}
#endif

	dbInsertNum = dsoSFGetInsertNum(pControl->nTimeDIV,pControl->nALT,pControl->nCHSet);//modified by zhang  to fit 6104  from "bool bSingle" to "nCHSet"

#ifdef FILE_TEST
	{
		CStdioFile file;	
		CString str;
		if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
		{
			str=_T("Cann't save Test file");
			//					AfxMessageBox(str);
			return 0;
		}
		file.SeekToEnd();
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			str.Format(_T("dsoSFGetInsertNum_Out::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
		}
		file.WriteString(str);
		file.Close();
		Sleep(5);
	}
#endif

	if(dbInsertNum > 1)
	{
#ifdef _LAUNCH

#ifdef FILE_TEST
		{
			CStdioFile file;	
			CString str;
			if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
			{
				str=_T("Cann't save Test file");
				//					AfxMessageBox(str);
				return 0;
			}
			file.SeekToEnd();
			{
				SYSTEMTIME st;
				GetLocalTime(&st);
				str.Format(_T("InsertData_DSO4250_In::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
			}
			file.WriteString(str);
			file.Close();
			Sleep(5);
		}
#endif

		InsertData_DSO4250(SourceData,BufferData,nInsertMode,dbInsertNum,pControl,nCH,pState);

#ifdef FILE_TEST
		{
			CStdioFile file;	
			CString str;
			if(!file.Open(_T("test.txt"),CFile::modeWrite,NULL))
			{
				str=_T("Cann't save Test file");
				//					AfxMessageBox(str);
				return 0;
			}
			file.SeekToEnd();
			{
				SYSTEMTIME st;
				GetLocalTime(&st);
				str.Format(_T("InsertData_DSO4250_Out::%d-%d-%d %d:%d:%d %d\n"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond,st.wMilliseconds);
			}
			file.WriteString(str);
			file.Close();
			Sleep(5);
		}
#endif

#else
		InsertData(SourceData,BufferData,nInsertMode,dbInsertNum,pControl,nCH,pState);
#endif
		//20130712  测试
	}
	else//无需差值
	{
		for(ULONG i = 0;i < pControl->nReadDataLen;i++)
		{
			BufferData[i] = SourceData[i];
		}
	}
	return 1;
}

DLL_API WORD WINAPI dsoSFProcessALTData(WORD* pOutData,WORD* pInData1,WORD* pInData2,ULONG nDataLen,WORD nHTriggerPos,WORD nCalData)
{
	WORD* pData;
	ULONG nInTrigger,nOutTrigger;
	ULONG i = 0;

	if(nHTriggerPos != 100)
	{
		nOutTrigger = ULONG(nDataLen * nHTriggerPos/100.0);
		nInTrigger = nOutTrigger * 2;
	}
	else
	{
		nOutTrigger = nDataLen-1;
		nInTrigger = nDataLen*2-1;
	}
	pData = new USHORT[nDataLen*2];
	for(i=0;i<nDataLen;i++)
	{
		*(pData+2*i+0) = *(pInData1+i);
		if(*(pData+2*i+1) < nCalData)
		{
			*(pData+2*i+1) = 0;
		}
		else if(*(pData+2*i+1) >= MAX_DATA)
		{
			*(pData+2*i+1) = *(pInData2+i);
		}
		else
		{
			*(pData+2*i+1) = *(pInData2+i) - nCalData;
		}
		*(pData+2*i+1) = *(pInData2+i);
	}
//触发点左边数据	
	for(i=0;i<=nOutTrigger;i++)
	{
		*(pOutData+nOutTrigger-i) = *(pData+nInTrigger-i);
	}
//触发点右边数据
	for(i=0;i<nDataLen-nOutTrigger;i++)
	{
		*(pOutData+nOutTrigger+i) = *(pData+nInTrigger+i);
	}
	delete pData;
	return 1;
}
/*dsoSFProcessALTData4CH函数暂时不使用*/
DLL_API WORD WINAPI dsoSFProcessALTData4CH(WORD* pOutData,WORD* pInData1,WORD* pInData2,WORD* pInData3,WORD* pInData4,ULONG nDataLen,WORD nHTriggerPos,WORD nCalData)
{
	WORD* pData;
	ULONG nInTrigger,nOutTrigger;
	ULONG i = 0;

	if(nHTriggerPos != 100)
	{
		nOutTrigger = ULONG(nDataLen * nHTriggerPos/100.0);
		nInTrigger = nOutTrigger * 4;
	}
	else
	{
		nOutTrigger = nDataLen-1;
		nInTrigger = nDataLen * 4-1;
	}
	pData = new USHORT[nDataLen * 4];
	for(i=0;i<nDataLen;i++)
	{
		*(pData+4*i+0) = *(pInData1+i);
		*(pData+4*i+1) = *(pInData2+i);
		*(pData+4*i+2) = *(pInData3+i);
		*(pData+4*i+3) = *(pInData4+i);
	/*	if(*(pData+4*i+1) < nCalData)
		{
			*(pData+4*i+1) = 0;
		}
		else if(*(pData+4*i+1) >= MAX_DATA)
		{
			*(pData+4*i+1) = *(pInData2+i);
		}
		else
		{
			*(pData+4*i+1) = *(pInData2+i) - nCalData;
		}
		*/
	}
//触发点左边数据	
	for(i=0;i<=nOutTrigger;i++)
	{
		*(pOutData+nOutTrigger-i) = *(pData+nInTrigger-i);
	}
//触发点右边数据
	for(i=0;i<nDataLen-nOutTrigger;i++)
	{
		*(pOutData+nOutTrigger+i) = *(pData+nInTrigger+i);
	}
	delete pData;
	return 1;
}

DLL_API ULONG WINAPI dsoSFFindTrigger(WORD* SourceData,WORD* BufferData,PCONTROLDATA Control,double dInsert)//zhang addd dInsert
{
	return SoftFindTrigger(SourceData,BufferData,Control,dInsert);
}

DLL_API WORD WINAPI dsoSFFindTriggerCopy(WORD* SourceData,WORD* BufferData,PCONTROLDATA Control,ULONG TriggerPoint)
{
	return SoftFindTriggerCopy(SourceData,BufferData,Control,TriggerPoint);
}

DLL_API WORD WINAPI dsoSFInsertDataStep(WORD* SourceData,WORD* pBuffer,double div_data,PCONTROLDATA Control)
{
	InsertDataStep(SourceData,pBuffer,div_data,Control);
	return 1;
}

DLL_API WORD WINAPI dsoSFInsertDataLine(WORD* SourceData,WORD* pBuffer,double div_data,PCONTROLDATA Control)
{
	InsertDataLine(SourceData,pBuffer,div_data,Control);
	return 1;
}

DLL_API WORD WINAPI dsoSFInsertDataSin(WORD* SourceData,WORD* pBuffer,PCONTROLDATA Control, double dbInsertNum,double* dbSinSheet)
{
	InsertDataSin(SourceData,pBuffer,Control,dbInsertNum,dbSinSheet,0);
	return 1;
}

DLL_API WORD WINAPI dsoSFCalSinSheet(double div_data,double* dbSinSheet)
{
	CalSinSheet(div_data,dbSinSheet);
	return 1;
}

DLL_API double WINAPI dsoSFGetInsertNum(WORD nTimeDIV, WORD nALT, WORD nCHSet)    //modified by zhang to fit dso6104
{	
    int activeCH=0;
    for(int itemp=0;itemp<MAX_CH_NUM;itemp++)
    {
        if((nCHSet>>itemp)&0x01)
        {
            activeCH++;
        }
    }
    switch (nTimeDIV) {
    case 7://500nS
        return activeCH>=3?2:1;
        break;
    case 6://200nS
        return activeCH>=3?5 :(activeCH==2?2.5:1.25);
        break;
    case 5://100nS
        return activeCH>=3?10:(activeCH==2?5  :2.5);
        break;
    case 4://50nS
        return activeCH>=3?20:(activeCH==2?10 :5 );
        break;
    case 3://20nS
        return activeCH>=3?50:(activeCH==2?25 :12.5);
        break;
    case 2://10nS
        return activeCH>=3?100:(activeCH==2?50 :25);
        break;
    case 1://5nS
        return activeCH>=3?200:(activeCH==2?100:50);
        break;
	case 0://2nS
        return activeCH>=3?500:(activeCH==2?250:125);
        break;
    default:
        return 1;
        break;
    }
}

DLL_API void WINAPI dsoSFProcessInsertData(WORD* SourceData,WORD* BufferData,PCONTROLDATA pControl,WORD nInsertMode,BOOL bSingleCH,double* dbSinSheet,USHORT nCH)
{
	ULONG nDataLen=DEF_READ_DATA_LEN;
	double dbInsertNum = dsoSFGetInsertNum(pControl->nTimeDIV,pControl->nALT,pControl->nCHSet);  //modified by zhang  to fit 6104  from "bool bSingle" to "nCHSet"
	nDataLen = pControl->nReadDataLen;
	if(dbInsertNum == 0)//不需要插值
	{
		ULONG i=0;
		for(i=0;i<nDataLen;i++)
		{
			*(BufferData+i) = *(SourceData+i);
		}
	}
	else
	{
		USHORT InsertMode = nInsertMode;
		if(pControl->nTimeDIV <= MAX_SINE_TIMEDIV)//最后4个档位才使用多种差值方式
		{
			InsertMode = nInsertMode;
			//InsertMode = 1;  //zhang 一直用斜率
		}
		if(InsertMode==INSERT_MODE_STEP)//台阶
		{
			InsertDataStep(SourceData,BufferData,dbInsertNum,pControl);
		}
		else if(InsertMode==INSERT_MODE_LINEAR)//斜线
		{
			InsertDataLine(SourceData,BufferData,dbInsertNum,pControl);
		}
		else//if(InsertMode == INSERT_MODE_SIN)//SIN内插
		{
			InsertDataSin(SourceData,BufferData,pControl,dbInsertNum,dbSinSheet,nCH);
		}
	}
}

DLL_API short WINAPI HTPosConvertToScale(int x,int y)
{
	double dd,xx,yy;		

	xx=x - 87/2.0;
	yy=y - 85/2.0;
	dd=atan2(xx,-yy);
	dd=dd*50.0/(PI);
	dd=(dd+50.0-25.0/2.0)/0.75;
	
	if (dd<0) dd=0;
	if (dd>100) dd=100;
	return (short)dd;
}

DLL_API POINT WINAPI HTGetTracePoint(RECT Rect,POINT pt,short nData,USHORT nDisLeverPos)
{
	POINT ptTrace;
	ptTrace.x = pt.x;
	ptTrace.y = ULONG(Rect.top + (nDisLeverPos-nData)*((Rect.bottom - Rect.top)*1.0/MAX_DATA));
	
	return ptTrace;
}

DLL_API ULONG WINAPI HTGetTracePointIndex(RECT Rect,POINT pt,ULONG nDisDataLen,ULONG nSrcDataLen,USHORT nHTriggerPos)
{
	ULONG nIndex = 0;
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

	nIndex = ULONG((pt.x - Rect.left)*1.0/(Rect.right-Rect.left)*nDisDataLen + nHTriggerPos/100.0*(nSrcDataLen-nDisDataLen));
	return nIndex;
}

DLL_API ULONG WINAPI dsoSFGetSampleRate(WORD nTimeDIV,WORD nALT,WORD nCHState,WORD nCH)
{
	ULONG nRate = 0;
	BOOL bSingleCH = FALSE;
	BOOL bCH[MAX_CH_NUM];
//CH1
	for(int i=0;i<MAX_CH_NUM;i++)
	{
		if(((nCHState >> i) & 0x01) == 1)
		{
			bCH[i] = TRUE;
		}
		else
		{
			bCH[i] = FALSE;
		}
	}

	if(nCH == CH1)
	{
		if(bCH[CH2] == FALSE)
		{
			bSingleCH = TRUE;
		}
	}
	else if(nCH == CH2)
	{
		if(bCH[CH1] == FALSE)
		{
			bSingleCH = TRUE;
		}
	}
	else if(nCH == CH3)
	{
		if(bCH[CH4] == FALSE)
		{
			bSingleCH = TRUE;
		}
	}
	else if(nCH == CH4)
	{
		if(bCH[CH3] == FALSE)
		{
			bSingleCH = TRUE;
		}
	}

	if(nTimeDIV <= (MAX_INSERT_TIMEDIV + TIMEDIV_OFFSET))//5ns ~ 5us
	{
		if(nALT == 1 || bSingleCH == TRUE)
		{
			nRate = 200000000;
		}
		else
		{
			nRate = 100000000;
		}
	}
	else if(nTimeDIV == (10 + TIMEDIV_OFFSET))//10us
	{
		nRate = 100000000;
	}
	else if(nTimeDIV == (11 + TIMEDIV_OFFSET))//20us
	{
		nRate = 50000000;
	}
	else if(nTimeDIV == (12 + TIMEDIV_OFFSET))//50us
	{
		nRate = 20000000;
	}
	else if(nTimeDIV == (13 + TIMEDIV_OFFSET))//100us
	{
		nRate = 10000000;
	}
	else if(nTimeDIV == (14 + TIMEDIV_OFFSET))//200us
	{
		nRate = 5000000;
	}
	else if(nTimeDIV == (15 + TIMEDIV_OFFSET))//500us
	{
		nRate = 2000000;
	}
	else if(nTimeDIV == (16 + TIMEDIV_OFFSET))//1ms
	{
		nRate = 1000000;
	}
	else if(nTimeDIV == (17 + TIMEDIV_OFFSET))//2ms
	{
		nRate = 500000;
	}
	else if(nTimeDIV == (18 + TIMEDIV_OFFSET))//5ms
	{
		nRate = 200000;
	}
	else if(nTimeDIV == (19 + TIMEDIV_OFFSET))//10ms
	{
		nRate = 100000;
	}
	else if(nTimeDIV == (20 + TIMEDIV_OFFSET))//20ms
	{
		nRate = 50000;
	}
	else if(nTimeDIV == (21 + TIMEDIV_OFFSET))//50ms
	{
		nRate = 20000;
	}
	else if(nTimeDIV == (22 + TIMEDIV_OFFSET))//100ms
	{
		nRate = 10000;
	}
	else if(nTimeDIV == (23 + TIMEDIV_OFFSET))//200ms
	{
		nRate = 5000;
	}
	else if(nTimeDIV == (24 + TIMEDIV_OFFSET))//500ms
	{
		nRate = 2000;//2k
	}
	else if(nTimeDIV == (25 + TIMEDIV_OFFSET))//1s
	{
		nRate = 1000;//1k
	}
	else if(nTimeDIV == (26 + TIMEDIV_OFFSET))//2s
	{
		nRate = 500;//500
	}
	else if(nTimeDIV == (27 + TIMEDIV_OFFSET))//5s
	{
		nRate = 200;//200
	}
	else if(nTimeDIV == (28 + TIMEDIV_OFFSET))//10s
	{
		nRate = 100;//100
	}
	else if(nTimeDIV == (29 + TIMEDIV_OFFSET))//20s
	{
		nRate = 50;//50
	}
	else if(nTimeDIV == (30 + TIMEDIV_OFFSET))//50s
	{
		nRate = 20;//20
	}
	else if(nTimeDIV == (31 + TIMEDIV_OFFSET))//100s
	{
		nRate = 10;//10
	}
	else if(nTimeDIV == (32 + TIMEDIV_OFFSET))//200s
	{
		nRate = 5;//5
	}
	else if(nTimeDIV == (33 + TIMEDIV_OFFSET))//500s
	{
		nRate = 2;//2
	}
	else if(nTimeDIV == (34 + TIMEDIV_OFFSET))//1000s
	{
		nRate = 1;//1
	}
	else if(nTimeDIV == (35 + TIMEDIV_OFFSET))//2000s
	{
		nRate = 1;//1
	}
	else if(nTimeDIV == (36 + TIMEDIV_OFFSET))//5000s
	{
		nRate = 1;//1
	}
	else if(nTimeDIV == (37 + TIMEDIV_OFFSET))//10000s
	{
		nRate = 1;//1
	}
	return nRate;
}

DLL_API WORD WINAPI dsoSFMathOperate(USHORT nOperate,const short* pSrcA,const short* pSrcB,short* pMathData,ULONG nDataLen)
{
	ULONG i=0;
	if(nOperate == MATH_ADD)
	{
		for(i=0;i<nDataLen;i++)
		{
			*(pMathData+i) = *(pSrcA+i) + *(pSrcB+i);
		}
	}
	else if(nOperate == MATH_SUB)
	{
		for(i=0;i<nDataLen;i++)
		{
			*(pMathData+i) = *(pSrcA+i) - *(pSrcB+i);
		}
	}
	else if(nOperate == MATH_MUL)
	{
		for(i=0;i<nDataLen;i++)
		{
			*(pMathData+i) = short(*(pSrcA+i)/10.0 * *(pSrcB+i));
		}
	}
	else if(nOperate == MATH_DIV)
	{
		for(i=0;i<nDataLen;i++)
		{
			if(*(pSrcB+i) != 0)//保证分母不是0
			{
				*(pMathData+i) = short(*(pSrcA+i)*1.0 / *(pSrcB+i));
			}
			else
			{
				*(pMathData+i) = *(pSrcA+i);
			}
		}
	}
	else
	{
		return 0;
	}
	return 1;
}

DLL_API WORD WINAPI dsoSFChooseData(WORD* pSrcData,ULONG nSrcDataLen,ULONG nChoose,WORD* pOutData,ULONG nOutDataLen)
{
	ULONG nMaxDataIndex=0,nMinDataIndex=0;
	WORD nMaxData=MIN_DATA,nMinData=MAX_DATA;
	ULONG i=0,j=0;
	ULONG nLast = 0;
	nLast = nOutDataLen % 2;//取余
	nOutDataLen /= 2;
	for(j=0;j<nOutDataLen;j++)
	{
		nMaxDataIndex = nMinDataIndex = 0;
		nMinData = nMaxData = *(pSrcData+j*nChoose);
		for(i=0;i<nChoose;i++) 
		{ 
			if(i+j*nChoose < nSrcDataLen)
			{
				if (nMinData > *(pSrcData+i+j*nChoose))
				{
					nMinData=*(pSrcData+i+j*nChoose);
					nMinDataIndex = i;
				}
				else if (nMaxData < *(pSrcData+i+j*nChoose))
				{
					nMaxData=*(pSrcData+i+j*nChoose);
					nMaxDataIndex = i;
				}
			}
			else
			{
				break;
			}
		}
		if(nMaxDataIndex > nMinDataIndex)
		{
			*(pOutData+j*2)=nMinData;
			*(pOutData+j*2+1)=nMaxData;
		}
		else
		{
			*(pOutData+j*2)=nMaxData;
			*(pOutData+j*2+1)=nMinData;
		}
	}
	if(nLast == 1)
	{
		*(pOutData+(nOutDataLen*2)) = *(pOutData+(nOutDataLen*2-1));
	}
	return 1;
}
//挑点函数
WORD ChooseShortData(short* pSrcData,ULONG nSrcDataLen,ULONG nChoose,short* pOutData,ULONG nOutDataLen)
{
	ULONG nMaxDataIndex=0,nMinDataIndex=0;
	short nMaxData = -MAX_DATA,nMinData = MAX_DATA;
	ULONG i=0,j=0;
	ULONG nLast = 0;
	nLast = nOutDataLen % 2;//取余
	nOutDataLen /= 2;
	for(j=0;j<nOutDataLen;j++)
	{
		nMaxDataIndex = nMinDataIndex = 0;
		nMinData = nMaxData = *(pSrcData+j*nChoose);
		for(i=0;i<nChoose;i++) 
		{ 
			if(i+j*nChoose < nSrcDataLen)
			{
				if (nMinData > *(pSrcData+i+j*nChoose))
				{
					nMinData=*(pSrcData+i+j*nChoose);
					nMaxDataIndex = i;
				}
				else if (nMaxData < *(pSrcData+i+j*nChoose))
				{
					nMaxData=*(pSrcData+i+j*nChoose);
					nMinDataIndex = i;
				}
			}
			else
			{
				break;
			}
		}
		if(nMaxDataIndex > nMinDataIndex)
		{
			*(pOutData+j*2)=nMinData;
			*(pOutData+j*2+1)=nMaxData;
		}
		else
		{
			*(pOutData+j*2)=nMaxData;
			*(pOutData+j*2+1)=nMinData;
		}
	}
	if(nLast == 1)
	{
		*(pOutData+(nOutDataLen*2)) = *(pOutData+(nOutDataLen*2-1));
	}
	return 1;
}

DLL_API WORD WINAPI dsoSFGetFFTSrcData(short* pSrcData,ULONG nCenterData,short* pFFTSrcData)
{
	short* pData;
//	pData = pSrcData + (nCenterData - (BUF_10K_LEN / 2));
//	return ChooseShortData(pData,BUF_10K_LEN,40,pFFTSrcData,FFT_NUM*2);

	if(nCenterData >= FFT_SRC_DATA_LEN/2)
	{
		pData = pSrcData + nCenterData - FFT_SRC_DATA_LEN/2;
		for(ULONG i=0;i<FFT_SRC_DATA_LEN;i++)
		{
			pFFTSrcData[i] = pData[i];
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

DLL_API double WINAPI dsoSFGetFFTSa(double dbTime)//返回的是Hz
{
	if(dbTime > 0)
	{
		return 1000.0 / dbTime;
	}
	else
	{
		return 0;
	}
}

DLL_API double WINAPI dsoSFGetMiniScopeFFTSa(USHORT nTimeDIV,USHORT nCHNum,USHORT nMode)//返回的是Hz
{
	double Sa = 0.0;
	
	if(nCHNum == 0)
	{
		return 0.0;
	}
	if(nMode == YT_ROLL)
	{
		nCHNum += 1;
	}

	if(nTimeDIV <= 15)//2.4M
	{
		Sa = 2400000.0 / nCHNum;
	}
	else if(nTimeDIV == 16)
	{
		Sa = 1200000.0 / nCHNum;
	}
	else if(nTimeDIV == 17)
	{
		Sa = 800000.0 / nCHNum;
	}
	else if(nTimeDIV == 18)
	{
		Sa = 400000.0 / nCHNum;
	}
	else if(nTimeDIV == 19)
	{
		Sa = 200000.0 / nCHNum;
	}
	else if(nTimeDIV == 20)
	{
		Sa = 75000.0 / nCHNum;
	}
	else if(nTimeDIV == 21)
	{
		Sa = 37037.0 / nCHNum;
	}
	else if(nTimeDIV == 22)
	{
		Sa = 17875.0 / nCHNum;
	}
	else if(nTimeDIV == 23)
	{
		Sa = 7937.0 / nCHNum;
	}
	else if(nTimeDIV == 24)
	{
		Sa = 4000.0 / nCHNum;
	}
	else if(nTimeDIV == 25)
	{
		Sa = 2000.0 / nCHNum;
	}
	else if(nTimeDIV == 26)
	{
		Sa = 800.0 / nCHNum;
	}
	else if(nTimeDIV == 27)
	{
		Sa = 400.0 / nCHNum;
	}
	else if(nTimeDIV == 28)
	{
		Sa = 200.0 / nCHNum;
	}
	else if(nTimeDIV == 29)
	{
		Sa =80.0 / nCHNum;
	}
	else if(nTimeDIV == 30)
	{
		Sa = 40.0 / nCHNum;
	}
	else if(nTimeDIV == 31)
	{
		Sa = 20.0 / nCHNum;
	}
	else if(nTimeDIV == 32)
	{
		Sa = 8.0 / nCHNum;
	}
	else if(nTimeDIV == 33)
	{
		Sa = 4.0 / nCHNum;
	}
	else if(nTimeDIV == 34)
	{
		Sa = 2.0 / nCHNum;
	}
	else if(nTimeDIV == 35)
	{
		Sa = 0.8 / nCHNum;
	}
	else if(nTimeDIV == 36)
	{
		Sa = 0.4 / nCHNum;
	}
	else if(nTimeDIV == 37)
	{
		Sa = 0.2 / nCHNum;
	}
	else if(nTimeDIV == 38)
	{
		Sa = 0.08 / nCHNum;
	}
	else if(nTimeDIV == 39)
	{
		Sa = 0.04 / nCHNum;
	}
	else if(nTimeDIV == 40)
	{
		Sa = 0.02 / nCHNum;
	}
	else
	{
		Sa = 0.0 / nCHNum;
	}
	return Sa;
}

DLL_API BOOL WINAPI dsoSFCalPassFailData(short* pSrcData,ULONG nSrcDataLen,short* pOutData,float fH,float fV)
{
	ULONG i = 0,j = 0,nL = 0,nR = 0,m = 0;
	ULONG nHorizontal = ULONG(fH * (DEF_DRAW_DATA_LEN / H_GRID_NUM)) * 2;
	USHORT nVertical = USHORT(fV * (MAX_DATA / V_GRID_NUM));
	short* pTempData;
	ULONG nTempLen = nSrcDataLen*2;

	pTempData = new short[nTempLen];
//垂直
	for(i=0;i<nSrcDataLen;i++)
	{
		*(pTempData + j) = *(pSrcData+i) + nVertical;//大值
		*(pOutData + j) = *(pTempData + j);
		*(pTempData + j + 1) = *(pSrcData+i) - nVertical;//小值
		*(pOutData + j + 1) = *(pTempData + j + 1);
		j += 2;
	}
//水平
	for(i=0;i<nTempLen;i += 2)
	{
		for(j=2;j<nHorizontal;j += 2)
		{
			if(i + j >= nTempLen)
			{
				break;
			}
			if(i + j + 1 >= nTempLen)
			{
				break;
			}
			if(*(pTempData + i) > *(pOutData + i + j))
			{
				*(pOutData + i + j) = *(pTempData + i);
			}
			if(*(pTempData + i + 1) < *(pOutData + i + j + 1))
			{
				*(pOutData + i + j + 1) = *(pTempData + i + 1);
			}
		}
		for(j=2;j<nHorizontal;j += 2)
		{
			if(i < j)
			{
				break;
			}
			if(i < j + 1)
			{
				break;
			}
			if(*(pTempData + i) > *(pOutData + i - j))
			{
				*(pOutData + i - j) = *(pTempData + i);
			}
			if(*(pTempData + i - 1) < *(pOutData + i - j - 1))
			{
				*(pOutData + i - j - 1) = *(pTempData + i - 1);
			}
		}
	}
//挑点

	delete pTempData;

	return TRUE;
}

DLL_API BOOL WINAPI dsoSFPassFail(short* pPFData,short* pSrcData,ULONG nDataLen)
{
	ULONG j = 0;
	for(ULONG i = 0;i < nDataLen ;i++)
	{
		if(*(pSrcData + i) > *(pPFData + j))// > 大值
		{
			return FALSE;
		}
		else if(*(pSrcData + i) < *(pPFData + j + 1))// < 小值
		{
			return FALSE;
		}
		j += 2;
	}
	return TRUE;
}


void HTGetTriggerPos(USHORT nTimeDIV,USHORT nTriggerSource,ULONG* pState,USHORT nFPGAVersion)//For hantek
{
	if(nFPGAVersion == 0x0219)
	{
		if(nTimeDIV == 0)//5ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH2) = *(pState+CH1) - 800;	//因软件找触发的时候，左右两边的预留最少4000点的数据长度，所以不必担心出现负数或超出上限
				*(pState+CH3) = *(pState+CH1) - 2200;
				*(pState+CH4) = *(pState+CH1) - 3200;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 800;	//因软件找触发的时候，左右两边的预留最少4000点的数据长度，所以不必担心出现负数或超出上限
				*(pState+CH3) = *(pState+CH2) - 1400;
				*(pState+CH4) = *(pState+CH2) - 2400;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) + 2200;
				*(pState+CH2) = *(pState+CH3) + 1400;
				*(pState+CH4) = *(pState+CH3) - 1000;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 3200;
				*(pState+CH2) = *(pState+CH4) + 2400;
				*(pState+CH3) = *(pState+CH4) + 1000;
			}
		}
		else if(nTimeDIV == 1)//10ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH2) = *(pState+CH1) - 400;
				*(pState+CH3) = *(pState+CH1) - 1180;
				*(pState+CH4) = *(pState+CH1) - 1690;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 400;
				*(pState+CH3) = *(pState+CH2) - 730;
				*(pState+CH4) = *(pState+CH2) - 1140;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) + 1180;
				*(pState+CH2) = *(pState+CH3) + 730;
				*(pState+CH4) = *(pState+CH3) - 410;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 1690;
				*(pState+CH2) = *(pState+CH4) + 1200;
				*(pState+CH3) = *(pState+CH4) + 460;
			}
		}
		else if(nTimeDIV == 2)//20ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH2) = *(pState+CH1) - 170;
				*(pState+CH3) = *(pState+CH1) - 500;
				*(pState+CH4) = *(pState+CH1) - 748;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 170;
				*(pState+CH3) = *(pState+CH2) - 330;
				*(pState+CH4) = *(pState+CH2) - 578;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) + 500;
				*(pState+CH2) = *(pState+CH3) + 330;
				*(pState+CH4) = *(pState+CH3) - 248;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 768;
				*(pState+CH2) = *(pState+CH4) + 578;
				*(pState+CH3) = *(pState+CH4) + 190;
			}
		}
		else if(nTimeDIV == 3)//50ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH2) = *(pState+CH1) - 50;
				*(pState+CH3) = *(pState+CH1) - 220;
				*(pState+CH4) = *(pState+CH1) - 310;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 50;
				*(pState+CH3) = *(pState+CH2) - 170;
				*(pState+CH4) = *(pState+CH2) - 260;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) + 220;
				*(pState+CH2) = *(pState+CH3) + 170;
				*(pState+CH4) = *(pState+CH3) - 90;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 310;
				*(pState+CH2) = *(pState+CH4) + 260;
				*(pState+CH3) = *(pState+CH4) + 90;
			}
		}
		else if(nTimeDIV == 4)//100ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH2) = *(pState+CH1) - 40;
				*(pState+CH3) = *(pState+CH1) - 110;
				*(pState+CH4) = *(pState+CH1) - 155;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 40;
				*(pState+CH3) = *(pState+CH2) - 70;
				*(pState+CH4) = *(pState+CH2) - 115;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) + 110;
				*(pState+CH2) = *(pState+CH3) + 70;
				*(pState+CH4) = *(pState+CH3) - 45;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 155;
				*(pState+CH2) = *(pState+CH4) + 115;
				*(pState+CH3) = *(pState+CH4) + 45;
			}
		}
		else if(nTimeDIV == 5)//200ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH2) = *(pState+CH1) - 23;
				*(pState+CH3) = *(pState+CH1) - 63;
				*(pState+CH4) = *(pState+CH1) - 80;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 10;
				*(pState+CH3) = *(pState+CH2) - 40;
				*(pState+CH4) = *(pState+CH2) - 67;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) + 63;
				*(pState+CH2) = *(pState+CH3) + 40;
				*(pState+CH4) = *(pState+CH3) - 17;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 80;
				*(pState+CH2) = *(pState+CH4) + 57;
				*(pState+CH3) = *(pState+CH4) + 17;
			}
		}
		else if(nTimeDIV == 6)//500ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH2) = *(pState+CH1) - 7;
				*(pState+CH3) = *(pState+CH1) - 57;
				*(pState+CH4) = *(pState+CH1) - 45;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 8;
				*(pState+CH3) = *(pState+CH2) - 17;
				*(pState+CH4) = *(pState+CH2) - 25;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) + 26;
				*(pState+CH2) = *(pState+CH3) + 15;
				*(pState+CH4) = *(pState+CH3) - 4;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 30;
				*(pState+CH2) = *(pState+CH4) + 27;
				*(pState+CH3) = *(pState+CH4) + 2;
			}
		}
		else if(nTimeDIV == 7)//1us
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH2) = *(pState+CH1) - 8;
				*(pState+CH3) = *(pState+CH1) - 14;
				*(pState+CH4) = *(pState+CH1) - 14;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 8;
				*(pState+CH3) = *(pState+CH2) - 6;
				*(pState+CH4) = *(pState+CH2) - 6;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) + 14;
				*(pState+CH2) = *(pState+CH3) + 6;
				*(pState+CH4) = *(pState+CH3) + 0;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 14;
				*(pState+CH2) = *(pState+CH4) + 6;
				*(pState+CH3) = *(pState+CH4) + 0;
			}
		}
		else// if(nTimeDIV == )
		{
			for(int i=0;i<MAX_CH_NUM;i++)
			{
				if(i == nTriggerSource)
				{
					continue;
				}
				*(pState+i) = *(pState+nTriggerSource);
			}
		}
	}
//以修正的时基显示1个周期的信号为基准调整
	else if(nFPGAVersion == 0x0248)
	{
		if(nTimeDIV == 0)//5ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH2) = *(pState+CH1) - 1100;	//因软件找触发的时候，左右两边的预留最少4000点的数据长度，所以不必担心出现负数或超出上限
				*(pState+CH3) = *(pState+CH1) - 400;
				*(pState+CH4) = *(pState+CH1) - 1250;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 1050;	//因软件找触发的时候，左右两边的预留最少4000点的数据长度，所以不必担心出现负数或超出上限
				*(pState+CH3) = *(pState+CH2) + 750;
				*(pState+CH4) = *(pState+CH2) - 150;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) + 300;
				*(pState+CH2) = *(pState+CH3) - 750;
				*(pState+CH4) = *(pState+CH3) - 850;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 1250;
				*(pState+CH2) = *(pState+CH4) + 200;
				*(pState+CH3) = *(pState+CH4) + 950;
			}
		}
		else if(nTimeDIV == 1)//10ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1) - 200;
				*(pState+CH2) = *(pState+CH1) - 550;
				*(pState+CH4) = *(pState+CH1) - 600;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 550;
				*(pState+CH3) = *(pState+CH2) + 350;
				*(pState+CH4) = *(pState+CH2) - 100;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) + 200;
				*(pState+CH2) = *(pState+CH3) - 350;
				*(pState+CH4) = *(pState+CH3) - 450;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 650;
				*(pState+CH2) = *(pState+CH4) + 100;
				*(pState+CH3) = *(pState+CH4) + 450;
			}
		}
		else if(nTimeDIV == 2)//20ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1) - 100;
				*(pState+CH2) = *(pState+CH1) - 250;
				*(pState+CH4) = *(pState+CH1) - 300;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 250;
				*(pState+CH3) = *(pState+CH2) + 200;
				*(pState+CH4) = *(pState+CH2) - 0;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) + 100;
				*(pState+CH2) = *(pState+CH3) - 150;
				*(pState+CH4) = *(pState+CH3) - 200;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 300;
				*(pState+CH2) = *(pState+CH4) + 50;
				*(pState+CH3) = *(pState+CH4) + 200;
			}
		}
		else if(nTimeDIV == 3)//50ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) - 50;
				*(pState+CH4) = *(pState+CH1) - 100;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 50;
				*(pState+CH3) = *(pState+CH2) + 50;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 50;
				*(pState+CH4) = *(pState+CH3) - 100;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 100;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH4) + 100;
			}
		}
		else if(nTimeDIV == 4)//100ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH1);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 50;
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3);
				*(pState+CH4) = *(pState+CH3) - 50;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 50;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else if(nTimeDIV == 5)//200ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1) + 130;
				*(pState+CH2) = *(pState+CH1) + 105;
				*(pState+CH4) = *(pState+CH1) + 100;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) - 105;
				*(pState+CH3) = *(pState+CH2);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 25;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 25;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else if(nTimeDIV == 6)//500ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1) + 50;
				*(pState+CH2) = *(pState+CH3);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) - 50;
				*(pState+CH3) = *(pState+CH2) - 5;
				*(pState+CH4) = *(pState+CH2) - 8;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) - 50;
				*(pState+CH2) = *(pState+CH3) + 10;
				*(pState+CH4) = *(pState+CH3) - 10;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) - 50;
				*(pState+CH2) = *(pState+CH4) + 10;
				*(pState+CH3) = *(pState+CH4) + 10;
			}
		}
		else if(nTimeDIV == 7)//1us
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1) + 35;
				*(pState+CH2) = *(pState+CH1) + 38;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) - 38;
				*(pState+CH3) = *(pState+CH1) + 20;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) - 15;
				*(pState+CH2) = *(pState+CH3);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) - 15;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH4);
			}
		}
		else// if(nTimeDIV == )
		{
			for(int i=0;i<MAX_CH_NUM;i++)
			{
				if(i == nTriggerSource)
				{
					continue;
				}
				*(pState+i) = *(pState+nTriggerSource);
			}
		}
	}
	else if(nFPGAVersion == 0x0217 || nFPGAVersion == 0x0218)
	{
		if(nTimeDIV == 0)//5ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH2) = *(pState+CH1) - 1100;	//因软件找触发的时候，左右两边的预留最少4000点的数据长度，所以不必担心出现负数或超出上限
				*(pState+CH3) = *(pState+CH1) - 400;
				*(pState+CH4) = *(pState+CH1) - 1250;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 1050;	//因软件找触发的时候，左右两边的预留最少4000点的数据长度，所以不必担心出现负数或超出上限
				*(pState+CH3) = *(pState+CH2) + 750;
				*(pState+CH4) = *(pState+CH2) - 150;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) + 300;
				*(pState+CH2) = *(pState+CH3) - 750;
				*(pState+CH4) = *(pState+CH3) - 850;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 1250;
				*(pState+CH2) = *(pState+CH4) + 200;
				*(pState+CH3) = *(pState+CH4) + 950;
			}
		}
		else if(nTimeDIV == 1)//10ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1) - 200;
				*(pState+CH2) = *(pState+CH1) - 550;
				*(pState+CH4) = *(pState+CH1) - 600;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 550;
				*(pState+CH3) = *(pState+CH2) + 350;
				*(pState+CH4) = *(pState+CH2) - 100;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) + 200;
				*(pState+CH2) = *(pState+CH3) - 350;
				*(pState+CH4) = *(pState+CH3) - 450;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 650;
				*(pState+CH2) = *(pState+CH4) + 100;
				*(pState+CH3) = *(pState+CH4) + 450;
			}
		}
		else if(nTimeDIV == 2)//20ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1) - 100;
				*(pState+CH2) = *(pState+CH1) - 250;
				*(pState+CH4) = *(pState+CH1) - 300;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 250;
				*(pState+CH3) = *(pState+CH2) + 200;
				*(pState+CH4) = *(pState+CH2) - 0;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) + 100;
				*(pState+CH2) = *(pState+CH3) - 150;
				*(pState+CH4) = *(pState+CH3) - 200;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 300;
				*(pState+CH2) = *(pState+CH4) + 50;
				*(pState+CH3) = *(pState+CH4) + 200;
			}
		}
		else if(nTimeDIV == 3)//50ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) - 50;
				*(pState+CH4) = *(pState+CH1) - 100;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 50;
				*(pState+CH3) = *(pState+CH2) + 50;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 50;
				*(pState+CH4) = *(pState+CH3) - 100;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 100;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH4) + 100;
			}
		}
		else if(nTimeDIV == 4)//100ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH1);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 50;
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3);
				*(pState+CH4) = *(pState+CH3) - 50;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 50;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else if(nTimeDIV == 5)//200ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1) + 130;
				*(pState+CH2) = *(pState+CH1) + 105;
				*(pState+CH4) = *(pState+CH1) + 100;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) - 105;
				*(pState+CH3) = *(pState+CH2);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 25;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 25;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else if(nTimeDIV == 6)//500ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1) + 50;
				*(pState+CH2) = *(pState+CH3);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) - 50;
				*(pState+CH3) = *(pState+CH2) - 5;
				*(pState+CH4) = *(pState+CH2) - 8;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) - 50;
				*(pState+CH2) = *(pState+CH3) + 10;
				*(pState+CH4) = *(pState+CH3) - 10;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) - 50;
				*(pState+CH2) = *(pState+CH4) + 10;
				*(pState+CH3) = *(pState+CH4) + 10;
			}
		}
		else if(nTimeDIV == 7)//1us
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1) + 35;
				*(pState+CH2) = *(pState+CH1) + 38;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) - 38;
				*(pState+CH3) = *(pState+CH1) + 20;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) - 15;
				*(pState+CH2) = *(pState+CH3);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) - 15;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH4);
			}
		}
		else// if(nTimeDIV == )
		{
			for(int i=0;i<MAX_CH_NUM;i++)
			{
				if(i == nTriggerSource)
				{
					continue;
				}
				*(pState+i) = *(pState+nTriggerSource);
			}
		}
	}
	else if(nFPGAVersion == 0x0117)
	{
		if(nTimeDIV == 0)//5ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1) - 450;
				*(pState+CH2) = *(pState+CH1) - 1100;	//因软件找触发的时候，左右两边的预留最少4000点的数据长度，所以不必担心出现负数或超出上限
				*(pState+CH4) = *(pState+CH1) - 1350;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 1200;	//因软件找触发的时候，左右两边的预留最少4000点的数据长度，所以不必担心出现负数或超出上限
				*(pState+CH3) = *(pState+CH2) + 700;
				*(pState+CH4) = *(pState+CH2) - 200;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) + 450;
				*(pState+CH2) = *(pState+CH3) - 700;
				*(pState+CH4) = *(pState+CH3) - 900;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 1400;
				*(pState+CH2) = *(pState+CH4) + 200;
				*(pState+CH3) = *(pState+CH4) + 950;
			}
		}
		else if(nTimeDIV == 1)//10ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1) - 200;
				*(pState+CH2) = *(pState+CH1) - 550;
				*(pState+CH4) = *(pState+CH1) - 600;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 550;
				*(pState+CH3) = *(pState+CH2) + 350;
				*(pState+CH4) = *(pState+CH2) - 100;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) + 200;
				*(pState+CH2) = *(pState+CH3) - 350;
				*(pState+CH4) = *(pState+CH3) - 450;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 650;
				*(pState+CH2) = *(pState+CH4) + 100;
				*(pState+CH3) = *(pState+CH4) + 500;
			}
		}
		else if(nTimeDIV == 2)//20ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1) - 100;
				*(pState+CH2) = *(pState+CH1) - 250;
				*(pState+CH4) = *(pState+CH1) - 300;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 250;
				*(pState+CH3) = *(pState+CH2) + 150;
				*(pState+CH4) = *(pState+CH2) - 100;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) + 100;
				*(pState+CH2) = *(pState+CH3) - 150;
				*(pState+CH4) = *(pState+CH3) - 200;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 300;
				*(pState+CH2) = *(pState+CH4) + 50;
				*(pState+CH3) = *(pState+CH4) + 200;
			}
		}
		else if(nTimeDIV == 3)//50ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) - 50;
				*(pState+CH4) = *(pState+CH1) - 100;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 50;
				*(pState+CH3) = *(pState+CH2) + 50;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 50;
				*(pState+CH4) = *(pState+CH3) - 100;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 100;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH4) + 100;
			}
		}
		else if(nTimeDIV == 4)//100ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH1);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 50;
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3);
				*(pState+CH4) = *(pState+CH3) - 50;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 50;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else if(nTimeDIV == 5)//200ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2);
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 25;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 25;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else if(nTimeDIV == 6)//500ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) + 50;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) - 50;
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3);
				*(pState+CH4) = *(pState+CH3);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4);
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH4);
			}
		}
		else if(nTimeDIV == 7)//1us
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) + 50;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) - 50;
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 5;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 5;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else// if(nTimeDIV == )
		{
			for(int i=0;i<MAX_CH_NUM;i++)
			{
				if(i == nTriggerSource)
				{
					continue;
				}
				*(pState+i) = *(pState+nTriggerSource);
			}
		}
	}
	else //if(nFPGAVersion <= 0x116)
	{
		if(nTimeDIV == 0)//5ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) - 950;	//因软件找触发的时候，左右两边的预留最少4000点的数据长度，所以不必担心出现负数或超出上限
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 950;	//因软件找触发的时候，左右两边的预留最少4000点的数据长度，所以不必担心出现负数或超出上限
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 950;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 950;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else if(nTimeDIV == 1)//10ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) - 450;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 450;
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 450;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 450;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else if(nTimeDIV == 2)//20ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) - 250;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 250;
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 250;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 250;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else if(nTimeDIV == 3)//50ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) - 100;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 100;
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 100;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 100;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else if(nTimeDIV == 4)//100ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) - 50;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 50;
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 50;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 50;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else if(nTimeDIV == 5)//200ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) - 25;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 25;
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 25;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 25;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else if(nTimeDIV == 6)//500ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) - 10;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 10;
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 10;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 10;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else if(nTimeDIV == 7)//1us
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) - 5;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 5;
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 5;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 5;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else// if(nTimeDIV == )
		{
			for(int i=0;i<MAX_CH_NUM;i++)
			{
				if(i == nTriggerSource)
				{
					continue;
				}
				*(pState+i) = *(pState+nTriggerSource);
			}
		}
	}
}

DLL_API void WINAPI dsoGetSoftTriggerPos(USHORT nTimeDIV,USHORT nTriggerSource,ULONG* pState,USHORT nFPGAVersion)
{
	HTGetTriggerPos(nTimeDIV,nTriggerSource,pState,nFPGAVersion);
}

void HTGetTriggerPosNew(USHORT nTimeDIV,USHORT nTriggerSource,ULONG* pState,USHORT nFPGAVersion,USHORT nCHSet)//add by pl 20130427
{
//以修正的时基显示1个周期的信号为基准调整
	if(nFPGAVersion == 0x0217 || nFPGAVersion == 0x0218)
	{//因软件找触发的时候，左右两边的预留最少4000点的数据长度，所以不必担心出现负数或超出上限
		if(nTimeDIV == 0)//5ns
		{
			if(nTriggerSource == CH1)
			{
				if((nCHSet & 0x02) == 0x02)		//打开CH2
				{
					*(pState+CH2) = *(pState+CH1) - 1040;
				}
				else							//关闭CH2
				{
					*(pState+CH2) = *(pState+CH1) - 100;
				}
				nCHSet = nCHSet & 0x0c;
				if(nCHSet == 0x0c || nCHSet == 0x00)	//CH3和CH4同时打开或者关闭
				{
					*(pState+CH3) = *(pState+CH2) + 700;
					*(pState+CH4) = *(pState+CH2) - 260;
				}
				else if(nCHSet == 0x04)					//打开CH3,关闭CH4
				{
					*(pState+CH3) = *(pState+CH2) - 260;
					*(pState+CH4) = *(pState+CH2);
				}
				else if(nCHSet == 0x08)					//关闭CH3,打开CH4
				{
					*(pState+CH3) = *(pState+CH2);
					*(pState+CH4) = *(pState+CH2) - 260;
				}
			}
			else if(nTriggerSource == CH2)
			{
				if((nCHSet & 0x01) == 0x01)		//打开CH1
				{
					*(pState+CH1) = *(pState+CH2) + 1080;
				}
				else							//关闭CH1
				{
					*(pState+CH1) = *(pState+CH2) + 1030;
				}
				nCHSet = nCHSet & 0x0c;		
				if(nCHSet == 0x0c || nCHSet == 0x00)	//CH3和CH4同时打开或者关闭
				{
					*(pState+CH3) = *(pState+CH1) - 400;
					*(pState+CH4) = *(pState+CH1) - 1290;
				}
				else if(nCHSet == 0x04)					//打开CH3,关闭CH4
				{
					*(pState+CH3) = *(pState+CH1) - 1300;
					*(pState+CH4) = *(pState+CH1);
				}
				else if(nCHSet == 0x08)					//关闭CH3,打开CH4
				{
					*(pState+CH3) = *(pState+CH1);
					*(pState+CH4) = *(pState+CH1) - 1300;
				}
			}
			else if(nTriggerSource == CH3)
			{
				if((nCHSet & 0x08) == 0x08)		//打开CH4
				{
					*(pState+CH4) = *(pState+CH3) - 950;
				}
				else							//关闭CH4
				{
					*(pState+CH4) = *(pState+CH3);
				}
				nCHSet = nCHSet & 0x03;		
				if(nCHSet == 0x03 || nCHSet == 0x00)	//CH1和CH2同时打开或者关闭
				{
					*(pState+CH1) = *(pState+CH4) + 1270;
					*(pState+CH2) = *(pState+CH4) + 260;
				}
				else if(nCHSet == 0x01)					//打开CH1,关闭CH2
				{
					*(pState+CH1) = *(pState+CH4) + 310;
					*(pState+CH2) = *(pState+CH4);
				}
				else if(nCHSet == 0x02)					//关闭CH1,打开CH2
				{
					*(pState+CH1) = *(pState+CH4);
					*(pState+CH2) = *(pState+CH4) + 310;
				}
			}
			else if(nTriggerSource == CH4)
			{	
				if((nCHSet & 0x04) == 0x04)		//打开CH3
				{
					*(pState+CH3) = *(pState+CH4) + 970;
				}
				else							//关闭CH3
				{
					*(pState+CH3) = *(pState+CH4) + 1010;
				}
				nCHSet = nCHSet & 0x03;		
				if(nCHSet == 0x03 || nCHSet == 0x00)	//CH1和CH2同时打开或者关闭
				{
					*(pState+CH1) = *(pState+CH3) + 310;
					*(pState+CH2) = *(pState+CH3) - 710;
				}
				else if(nCHSet == 0x01)					//打开CH1,关闭CH2
				{
					*(pState+CH1) = *(pState+CH3) - 660;
					*(pState+CH2) = *(pState+CH3);
				}
				else if(nCHSet == 0x02)					//关闭CH1,打开CH2
				{
					*(pState+CH1) = *(pState+CH3);
					*(pState+CH2) = *(pState+CH3) - 660;
				}
			}
		}
		else if(nTimeDIV == 1)//10ns
		{
			if(nTriggerSource == CH1)
			{
				if((nCHSet & 0x02) == 0x02)		//打开CH2
				{
					*(pState+CH2) = *(pState+CH1) - 520;
				}
				else							//关闭CH2
				{
					*(pState+CH2) = *(pState+CH1) - 20;
				}
				nCHSet = nCHSet & 0x0c;
				if(nCHSet == 0x0c || nCHSet == 0x00)	//CH3和CH4同时打开或者关闭
				{
					*(pState+CH3) = *(pState+CH2) + 350;
					*(pState+CH4) = *(pState+CH2) - 150;
				}
				else if(nCHSet == 0x04)					//打开CH3,关闭CH4
				{
					*(pState+CH3) = *(pState+CH2) - 120;
					*(pState+CH4) = *(pState+CH2);
				}
				else if(nCHSet == 0x08)					//关闭CH3,打开CH4
				{
					*(pState+CH3) = *(pState+CH2);
					*(pState+CH4) = *(pState+CH2) - 140;
				}
			}
			else if(nTriggerSource == CH2)
			{
				if((nCHSet & 0x01) == 0x01)		//打开CH1
				{
					*(pState+CH1) = *(pState+CH2) + 510;
				}
				else							//关闭CH1
				{
					*(pState+CH1) = *(pState+CH2) + 500;
				}
				nCHSet = nCHSet & 0x0c;		
				if(nCHSet == 0x0c || nCHSet == 0x00)	//CH3和CH4同时打开或者关闭
				{
					*(pState+CH3) = *(pState+CH1) - 160;
					*(pState+CH4) = *(pState+CH1) - 640;
				}
				else if(nCHSet == 0x04)					//打开CH3,关闭CH4
				{
					*(pState+CH3) = *(pState+CH1) - 640;
					*(pState+CH4) = *(pState+CH1);
				}
				else if(nCHSet == 0x08)					//关闭CH3,打开CH4
				{
					*(pState+CH3) = *(pState+CH1);
					*(pState+CH4) = *(pState+CH1) - 640;
				}
			}
			else if(nTriggerSource == CH3)
			{
				if((nCHSet & 0x08) == 0x08)		//打开CH4
				{
					*(pState+CH4) = *(pState+CH3) - 480;
				}
				else							//关闭CH4
				{
					*(pState+CH4) = *(pState+CH3);
				}
				nCHSet = nCHSet & 0x03;		
				if(nCHSet == 0x03 || nCHSet == 0x00)	//CH1和CH2同时打开或者关闭
				{
					*(pState+CH1) = *(pState+CH4) + 650;
					*(pState+CH2) = *(pState+CH4) + 140;
				}
				else if(nCHSet == 0x01)					//打开CH1,关闭CH2
				{
					*(pState+CH1) = *(pState+CH4) + 180;
					*(pState+CH2) = *(pState+CH4);
				}
				else if(nCHSet == 0x02)					//关闭CH1,打开CH2
				{
					*(pState+CH1) = *(pState+CH4);
					*(pState+CH2) = *(pState+CH4) + 170;
				}
			}
			else if(nTriggerSource == CH4)
			{	
				if((nCHSet & 0x04) == 0x04)		//打开CH3
				{
					*(pState+CH3) = *(pState+CH4) + 480;
				}
				else							//关闭CH3
				{
					*(pState+CH3) = *(pState+CH4) + 460;
				}
				nCHSet = nCHSet & 0x03;		
				if(nCHSet == 0x03 || nCHSet == 0x00)	//CH1和CH2同时打开或者关闭
				{
					*(pState+CH1) = *(pState+CH3) + 180;
					*(pState+CH2) = *(pState+CH3) - 345;
				}
				else if(nCHSet == 0x01)					//打开CH1,关闭CH2
				{
					*(pState+CH1) = *(pState+CH3) - 310;
					*(pState+CH2) = *(pState+CH3);
				}
				else if(nCHSet == 0x02)					//关闭CH1,打开CH2
				{
					*(pState+CH1) = *(pState+CH3);
					*(pState+CH2) = *(pState+CH3) - 310;
				}
			}
		}
		else if(nTimeDIV == 2)//20ns
		{
			if(nTriggerSource == CH1)
			{
				if((nCHSet & 0x02) == 0x02)		//打开CH2
				{
					*(pState+CH2) = *(pState+CH1) - 270;
				}
				else							//关闭CH2
				{
					*(pState+CH2) = *(pState+CH1) - 280;
				}
				nCHSet = nCHSet & 0x0c;
				if(nCHSet == 0x0c || nCHSet == 0x00)	//CH3和CH4同时打开或者关闭
				{
					*(pState+CH3) = *(pState+CH2) + 180;
					*(pState+CH4) = *(pState+CH2) - 60;
				}
				else if(nCHSet == 0x04)					//打开CH3,关闭CH4
				{
					*(pState+CH3) = *(pState+CH2) + 200;
					*(pState+CH4) = *(pState+CH2);
				}
				else if(nCHSet == 0x08)					//关闭CH3,打开CH4
				{
					*(pState+CH3) = *(pState+CH2);
					*(pState+CH4) = *(pState+CH2) + 200;
				}
			}
			else if(nTriggerSource == CH2)
			{
				if((nCHSet & 0x01) == 0x01)		//打开CH1
				{
					*(pState+CH1) = *(pState+CH2) + 270;
				}
				else							//关闭CH1
				{
					*(pState+CH1) = *(pState+CH2) - 20;
				}
				nCHSet = nCHSet & 0x0c;		
				if(nCHSet == 0x0c || nCHSet == 0x00)	//CH3和CH4同时打开或者关闭
				{
					*(pState+CH3) = *(pState+CH1) - 100;
					*(pState+CH4) = *(pState+CH1) - 330;
				}
				else if(nCHSet == 0x04)					//打开CH3,关闭CH4
				{
					*(pState+CH3) = *(pState+CH1) - 100;
					*(pState+CH4) = *(pState+CH1);
				}
				else if(nCHSet == 0x08)					//关闭CH3,打开CH4
				{
					*(pState+CH3) = *(pState+CH1);
					*(pState+CH4) = *(pState+CH1) - 100;
				}
			}
			else if(nTriggerSource == CH3)
			{		
				if((nCHSet & 0x08) == 0x08)		//打开CH4
				{
					*(pState+CH4) = *(pState+CH3) - 250;
				}
				else							//关闭CH4
				{
					*(pState+CH4) = *(pState+CH3) - 270;
				}
				nCHSet = nCHSet & 0x03;		
				if(nCHSet == 0x03 || nCHSet == 0x00)	//CH1和CH2同时打开或者关闭
				{
					*(pState+CH1) = *(pState+CH4) + 350;
					*(pState+CH2) = *(pState+CH4) + 80;
				}
				else if(nCHSet == 0x01)					//打开CH1,关闭CH2
				{
					*(pState+CH1) = *(pState+CH4) + 350;
					*(pState+CH2) = *(pState+CH4);
				}
				else if(nCHSet == 0x02)					//关闭CH1,打开CH2
				{
					*(pState+CH1) = *(pState+CH4);
					*(pState+CH2) = *(pState+CH4) + 350;
				}
			}
			else if(nTriggerSource == CH4)
			{	
				if((nCHSet & 0x04) == 0x04)		//打开CH3
				{
					*(pState+CH3) = *(pState+CH4) + 250;
				}
				else							//关闭CH3
				{
					*(pState+CH3) = *(pState+CH4);
				}
				nCHSet = nCHSet & 0x03;		
				if(nCHSet == 0x03 || nCHSet == 0x00)	//CH1和CH2同时打开或者关闭
				{
					*(pState+CH1) = *(pState+CH3) + 90;
					*(pState+CH2) = *(pState+CH3) - 180;
				}
				else if(nCHSet == 0x01)					//打开CH1,关闭CH2
				{
					*(pState+CH1) = *(pState+CH3) + 90;
					*(pState+CH2) = *(pState+CH3);
				}
				else if(nCHSet == 0x02)					//关闭CH1,打开CH2
				{
					*(pState+CH1) = *(pState+CH3);
					*(pState+CH2) = *(pState+CH3) + 90;
				}
			}
		}
		else if(nTimeDIV == 3)//50ns
		{
			if(nTriggerSource == CH1)
			{
				if((nCHSet & 0x02) == 0x02)		//打开CH2
				{
					*(pState+CH2) = *(pState+CH1) - 110;
				}
				else							//关闭CH2
				{
					*(pState+CH2) = *(pState+CH1) - 130;
				}
				nCHSet = nCHSet & 0x0c;
				if(nCHSet == 0x0c || nCHSet == 0x00)	//CH3和CH4同时打开或者关闭
				{
					*(pState+CH3) = *(pState+CH2) + 80;
					*(pState+CH4) = *(pState+CH2) - 30;
				}
				else if(nCHSet == 0x04)					//打开CH3,关闭CH4
				{
					*(pState+CH3) = *(pState+CH2) - 10;
					*(pState+CH4) = *(pState+CH2);
				}
				else if(nCHSet == 0x08)					//关闭CH3,打开CH4
				{
					*(pState+CH3) = *(pState+CH2) ;
					*(pState+CH4) = *(pState+CH2) - 10;
				}
			}
			else if(nTriggerSource == CH2)
			{
				if((nCHSet & 0x01) == 0x01)		//打开CH1
				{
					*(pState+CH1) = *(pState+CH2) + 95;
				}
				else							//关闭CH1
				{
					*(pState+CH1) = *(pState+CH2);
				}
				nCHSet = nCHSet & 0x0c;		
				if(nCHSet == 0x0c || nCHSet == 0x00)	//CH3和CH4同时打开或者关闭
				{
					*(pState+CH3) = *(pState+CH1) - 60;
					*(pState+CH4) = *(pState+CH1) - 130;
				}
				else if(nCHSet == 0x04)					//打开CH3,关闭CH4
				{
					*(pState+CH3) = *(pState+CH1) - 50;
					*(pState+CH4) = *(pState+CH1);
				}
				else if(nCHSet == 0x08)					//关闭CH3,打开CH4
				{
					*(pState+CH3) = *(pState+CH1);
					*(pState+CH4) = *(pState+CH1) - 50;
				}
			}
			else if(nTriggerSource == CH3)
			{
				if((nCHSet & 0x08) == 0x08)		//打开CH4
				{
					*(pState+CH4) = *(pState+CH3) - 100;
				}
				else							//关闭CH4
				{
					*(pState+CH4) = *(pState+CH3) - 100;
				}
				nCHSet = nCHSet & 0x03;		
				if(nCHSet == 0x03 || nCHSet == 0x00)	//CH1和CH2同时打开或者关闭
				{
					*(pState+CH1) = *(pState+CH4) + 150;
					*(pState+CH2) = *(pState+CH4) + 30;
				}
				else if(nCHSet == 0x01)					//打开CH1,关闭CH2
				{
					*(pState+CH1) = *(pState+CH4) + 130;
					*(pState+CH2) = *(pState+CH4);
				}
				else if(nCHSet == 0x02)					//关闭CH1,打开CH2
				{
					*(pState+CH1) = *(pState+CH4);
					*(pState+CH2) = *(pState+CH4) + 130;
				}
			}
			else if(nTriggerSource == CH4)
			{	
				if((nCHSet & 0x04) == 0x04)		//打开CH3
				{
					*(pState+CH3) = *(pState+CH4) + 120;
				}
				else							//关闭CH3
				{
					*(pState+CH3) = *(pState+CH4) + 35;	
				}
				nCHSet = nCHSet & 0x03;		
				if(nCHSet == 0x03 || nCHSet == 0x00)	//CH1和CH2同时打开或者关闭
				{
					*(pState+CH1) = *(pState+CH3) + 30 ;
					*(pState+CH2) = *(pState+CH3) - 60;
				}
				else if(nCHSet == 0x01)					//打开CH1,关闭CH2
				{
					*(pState+CH1) = *(pState+CH3) + 40;
					*(pState+CH2) = *(pState+CH3);
				}
				else if(nCHSet == 0x02)					//关闭CH1,打开CH2
				{
					*(pState+CH1) = *(pState+CH3);
					*(pState+CH2) = *(pState+CH3) + 50;
				}
			}
		}
		else if(nTimeDIV == 4)//100ns
		{
			if(nTriggerSource == CH1)
			{
				if((nCHSet & 0x02) == 0x02)		//打开CH2
				{
					*(pState+CH2) = *(pState+CH1) - 50;
				}
				else							//关闭CH2
				{
					*(pState+CH2) = *(pState+CH1) - 40;
				}
				nCHSet = nCHSet & 0x0c;
				if(nCHSet == 0x0c || nCHSet == 0x00)	//CH3和CH4同时打开或者关闭
				{
					*(pState+CH3) = *(pState+CH2) + 20;
					*(pState+CH4) = *(pState+CH2) - 25;
				}
				else if(nCHSet == 0x04)					//打开CH3,关闭CH4
				{
					*(pState+CH3) = *(pState+CH2) + 10;
					*(pState+CH4) = *(pState+CH2);
				}
				else if(nCHSet == 0x08)					//关闭CH3,打开CH4
				{
					*(pState+CH3) = *(pState+CH2);
					*(pState+CH4) = *(pState+CH2) + 25;
				}
			}
			else if(nTriggerSource == CH2)
			{
				if((nCHSet & 0x01) == 0x01)		//打开CH1
				{
					*(pState+CH1) = *(pState+CH2) + 55;
				}
				else							//关闭CH1
				{
					*(pState+CH1) = *(pState+CH2);
				}
				nCHSet = nCHSet & 0x0c;		
				if(nCHSet == 0x0c || nCHSet == 0x00)	//CH3和CH4同时打开或者关闭
				{
					*(pState+CH3) = *(pState+CH1) - 20;
					*(pState+CH4) = *(pState+CH1) - 60;
				}
				else if(nCHSet == 0x04)					//打开CH3,关闭CH4
				{
					*(pState+CH3) = *(pState+CH1) - 20;
					*(pState+CH4) = *(pState+CH1);
				}
				else if(nCHSet == 0x08)					//关闭CH3,打开CH4
				{
					*(pState+CH3) = *(pState+CH1);
					*(pState+CH4) = *(pState+CH1) - 20;
				}
			}
			else if(nTriggerSource == CH3)
			{
				if((nCHSet & 0x08) == 0x08)		//打开CH4
				{
					*(pState+CH4) = *(pState+CH3) - 55;
				}
				else							//关闭CH4
				{
					*(pState+CH4) = *(pState+CH3) - 55;
				}
				nCHSet = nCHSet & 0x03;		
				if(nCHSet == 0x03 || nCHSet == 0x00)	//CH1和CH2同时打开或者关闭
				{
					*(pState+CH1) = *(pState+CH4) + 75;
					*(pState+CH2) = *(pState+CH4) + 30;
				}
				else if(nCHSet == 0x01)					//打开CH1,关闭CH2
				{
					*(pState+CH1) = *(pState+CH4) + 75;
					*(pState+CH2) = *(pState+CH4);
				}
				else if(nCHSet == 0x02)					//关闭CH1,打开CH2
				{
					*(pState+CH1) = *(pState+CH4);
					*(pState+CH2) = *(pState+CH4) + 75;
				}
			}
			else if(nTriggerSource == CH4)
			{	
				if((nCHSet & 0x04) == 0x04)		//打开CH3
				{
					*(pState+CH3) = *(pState+CH4) + 35;
				}
				else							//关闭CH3
				{
					*(pState+CH3) = *(pState+CH4) - 25;
				}
				nCHSet = nCHSet & 0x03;		
				if(nCHSet == 0x03 || nCHSet == 0x00)	//CH1和CH2同时打开或者关闭
				{
					*(pState+CH1) = *(pState+CH3) + 15;
					*(pState+CH2) = *(pState+CH3) - 30;
				}
				else if(nCHSet == 0x01)					//打开CH1,关闭CH2
				{
					*(pState+CH1) = *(pState+CH3) + 15;
					*(pState+CH2) = *(pState+CH3);
				}
				else if(nCHSet == 0x02)					//关闭CH1,打开CH2
				{
					*(pState+CH1) = *(pState+CH3);
					*(pState+CH2) = *(pState+CH3) + 20;
				}
			}
		}
		else if(nTimeDIV == 5)//200ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH2) = *(pState+CH1) - 30;
				*(pState+CH3) = *(pState+CH1) - 5;
				*(pState+CH4) = *(pState+CH1) - 25;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 30;	
				*(pState+CH3) = *(pState+CH2) + 10;
				*(pState+CH4) = *(pState+CH2) - 10;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) + 15;	
				*(pState+CH2) = *(pState+CH3) - 20;
				*(pState+CH4) = *(pState+CH3) - 20;
			}
			else if(nTriggerSource == CH4)
			{	
				*(pState+CH1) = *(pState+CH4) + 25;	
				*(pState+CH2) = *(pState+CH4) + 10;
				*(pState+CH3) = *(pState+CH4) + 20;
			}
		}
		else if(nTimeDIV == 6)//500ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1) - 10;
				*(pState+CH2) = *(pState+CH1) - 10;
				*(pState+CH4) = *(pState+CH1) - 10;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 10;
				*(pState+CH3) = *(pState+CH2);
				*(pState+CH4) = *(pState+CH2) - 20;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) + 10;
				*(pState+CH2) = *(pState+CH3);
				*(pState+CH4) = *(pState+CH3) - 20;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 10;
				*(pState+CH2) = *(pState+CH4) + 20;
				*(pState+CH3) = *(pState+CH4) + 20;
			}
		}
		else if(nTimeDIV == 7)//1us
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH2) = *(pState+CH1);
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH1);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2);
				*(pState+CH3) = *(pState+CH2);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3);
				*(pState+CH4) = *(pState+CH3);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4);
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH4);
			}
		}
		else// if(nTimeDIV == )
		{
			for(int i=0;i<MAX_CH_NUM;i++)
			{
				if(i == nTriggerSource)
				{
					continue;
				}
				*(pState+i) = *(pState+nTriggerSource);
			}
		}
	}
	else if(nFPGAVersion == 0x0117)
	{
		if(nTimeDIV == 0)//5ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1) - 450;
				*(pState+CH2) = *(pState+CH1) - 1100;	//因软件找触发的时候，左右两边的预留最少4000点的数据长度，所以不必担心出现负数或超出上限
				*(pState+CH4) = *(pState+CH1) - 1350;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 1200;	//因软件找触发的时候，左右两边的预留最少4000点的数据长度，所以不必担心出现负数或超出上限
				*(pState+CH3) = *(pState+CH2) + 700;
				*(pState+CH4) = *(pState+CH2) - 200;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) + 450;
				*(pState+CH2) = *(pState+CH3) - 700;
				*(pState+CH4) = *(pState+CH3) - 900;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 1400;
				*(pState+CH2) = *(pState+CH4) + 200;
				*(pState+CH3) = *(pState+CH4) + 950;
			}
		}
		else if(nTimeDIV == 1)//10ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1) - 200;
				*(pState+CH2) = *(pState+CH1) - 550;
				*(pState+CH4) = *(pState+CH1) - 600;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 550;
				*(pState+CH3) = *(pState+CH2) + 350;
				*(pState+CH4) = *(pState+CH2) - 100;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) + 200;
				*(pState+CH2) = *(pState+CH3) - 350;
				*(pState+CH4) = *(pState+CH3) - 450;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 650;
				*(pState+CH2) = *(pState+CH4) + 100;
				*(pState+CH3) = *(pState+CH4) + 500;
			}
		}
		else if(nTimeDIV == 2)//20ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1) - 100;
				*(pState+CH2) = *(pState+CH1) - 250;
				*(pState+CH4) = *(pState+CH1) - 300;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 250;
				*(pState+CH3) = *(pState+CH2) + 150;
				*(pState+CH4) = *(pState+CH2) - 100;
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3) + 100;
				*(pState+CH2) = *(pState+CH3) - 150;
				*(pState+CH4) = *(pState+CH3) - 200;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 300;
				*(pState+CH2) = *(pState+CH4) + 50;
				*(pState+CH3) = *(pState+CH4) + 200;
			}
		}
		else if(nTimeDIV == 3)//50ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) - 50;
				*(pState+CH4) = *(pState+CH1) - 100;
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 50;
				*(pState+CH3) = *(pState+CH2) + 50;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 50;
				*(pState+CH4) = *(pState+CH3) - 100;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 100;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH4) + 100;
			}
		}
		else if(nTimeDIV == 4)//100ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH1);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 50;
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3);
				*(pState+CH4) = *(pState+CH3) - 50;
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 50;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else if(nTimeDIV == 5)//200ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2);
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 25;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 25;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else if(nTimeDIV == 6)//500ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) + 50;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) - 50;
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3);
				*(pState+CH4) = *(pState+CH3);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4);
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH4);
			}
		}
		else if(nTimeDIV == 7)//1us
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) + 50;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) - 50;
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 5;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 5;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else// if(nTimeDIV == )
		{
			for(int i=0;i<MAX_CH_NUM;i++)
			{
				if(i == nTriggerSource)
				{
					continue;
				}
				*(pState+i) = *(pState+nTriggerSource);
			}
		}
	}
	else //if(nFPGAVersion <= 0x116)
	{
		if(nTimeDIV == 0)//5ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) - 950;	//因软件找触发的时候，左右两边的预留最少4000点的数据长度，所以不必担心出现负数或超出上限
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 950;	//因软件找触发的时候，左右两边的预留最少4000点的数据长度，所以不必担心出现负数或超出上限
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 950;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 950;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else if(nTimeDIV == 1)//10ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) - 450;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 450;
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 450;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 450;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else if(nTimeDIV == 2)//20ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) - 250;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 250;
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 250;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 250;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else if(nTimeDIV == 3)//50ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) - 100;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 100;
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 100;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 100;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else if(nTimeDIV == 4)//100ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) - 50;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 50;
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 50;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 50;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else if(nTimeDIV == 5)//200ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) - 25;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 25;
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 25;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 25;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else if(nTimeDIV == 6)//500ns
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) - 10;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 10;
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 10;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 10;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else if(nTimeDIV == 7)//1us
		{
			if(nTriggerSource == CH1)
			{
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH2) = *(pState+CH1) - 5;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH2)
			{
				*(pState+CH1) = *(pState+CH2) + 5;
				*(pState+CH3) = *(pState+CH1);
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH3)
			{
				*(pState+CH1) = *(pState+CH3);
				*(pState+CH2) = *(pState+CH3) - 5;
				*(pState+CH4) = *(pState+CH2);
			}
			else if(nTriggerSource == CH4)
			{
				*(pState+CH1) = *(pState+CH4) + 5;
				*(pState+CH2) = *(pState+CH4);
				*(pState+CH3) = *(pState+CH1);
			}
		}
		else// if(nTimeDIV == )
		{
			for(int i=0;i<MAX_CH_NUM;i++)
			{
				if(i == nTriggerSource)
				{
					continue;
				}
				*(pState+i) = *(pState+nTriggerSource);
			}
		}
	}
}

DLL_API void WINAPI dsoGetSoftTriggerPosNew(PCONTROLDATA Control,ULONG* pState,USHORT nFPGAVersion)
{
	HTGetTriggerPosNew(Control->nTimeDIV,Control->nTriggerSource,pState,nFPGAVersion,Control->nCHSet);
}

DLL_API void WINAPI dsoAdjustSquareWave(WORD* pSrcData,ULONG nSrcDataLen)
{
    WORD nCurData=*pSrcData,nNextData;
    ULONG  nEuareNum=0;
	//bool check;
    for(ULONG i=0;i<nSrcDataLen;i++){
        if(nCurData==pSrcData[i])
        {
            nEuareNum++;
        }
        else{
            nNextData=pSrcData[i];
			if(nNextData!=pSrcData[i+1]) 
				return;
            if(i+3<nSrcDataLen&&nEuareNum>=3){
				
                if(nNextData==pSrcData[i+1]&&nNextData==pSrcData[i+2]&&nNextData==pSrcData[i+3]){
					pSrcData[i-1]=nCurData+(nNextData-nCurData)/3;
					pSrcData[i]=nNextData+(nCurData-nNextData)/3;
					i++;
                }
				else{
					if(i+1<nSrcDataLen){
						pSrcData[i]=nNextData+(nCurData-nNextData)/3;
						pSrcData[i-1]=nCurData+(nNextData-nCurData)/3;
						break;
					}
				}
            }
            nCurData=nNextData;
            nEuareNum=0;
        }
    }
}
DLL_API void WINAPI dsoAdjustADC(WORD* pSrcData, ULONG nSrcDataLen)
{
	return ;
    WORD nFirstData;
    ULONG nFistIndex=0;

    ULONG nSumData1=0;
    ULONG nSumData2=0;
    ULONG nMinLen=50;
    int nMinHeight=32;
    ULONG nCurLen=0;
    nFirstData=* pSrcData;
    for(ULONG i=0;i<nSrcDataLen;i++){
        if(i%2==0)
            continue;
        if(abs(nFirstData-pSrcData[i])<nMinHeight&&i<nSrcDataLen-2)
        {
            nCurLen++;
            nSumData1+=pSrcData[i-1]  ;
            nSumData2+=pSrcData[i]  ;
        }
        else{
            if(nCurLen>=nMinLen)
			//if(0)
            {
                double fHarfOffset=nSumData1/(2.0*nCurLen)-nSumData2/(2.0*nCurLen);
				if(fHarfOffset>32){
					int xxxxxxxx=0;
				}
                for(UINT j=nFistIndex;j<nFistIndex+2*nCurLen;j++){
                    if(j%2==0)
						if(pSrcData[j]>fHarfOffset-0.5)
							pSrcData[j] =WORD(pSrcData[j] -fHarfOffset+0.5) ;
                    else
                        pSrcData[j] =WORD(pSrcData[j] +fHarfOffset+0.5);
                }
            }
            if(i+1>=nSrcDataLen)
                break;
            nFirstData=pSrcData[i+1];
            nFistIndex=i+1;
            nSumData2=0;
            nSumData1=0;
            nCurLen=0;
        }
    }
}
/*
#define PI 3.1415926
#define SIN_FACTOR	10	//差值因子，前10个实际点，后10个实际点
#define SHEET_FACTOR	21 (由SIN_FACTOR = 10 决定)
double dbInsertNum = 5.0;//差值个数，一般是整数,假设差4个点(去掉1个实际点)
ULONG nDataLen = 10240;//实际采集的数据长度，这里假设10K
ULONG nCounter = 101;//实际采集的数据中用于差值的数据个数
//SourceData ----->原始数据（实际采集的数据）数组
//TempData	----->差值后的数据数组
ULONG i = 0,j = 0;
long k = 0,sum = 0;

for(i=0;i<nCounter;i++)
{
	if(dbInsertNum > 0.0)
	{
		for(j=1;j<dbInsertNum;j++)
		{
			sum = 0;
			for(k=(-SIN_FACTOR);k<=SIN_FACTOR;k++)
			{
				if(StartPoint+k >= nDataLen)//判断上限
				{
					sum = sum + *(SourceData+nDataLen-1) * dbSinSheet[(j-1)*SHEET_FACTOR+(k+SIN_FACTOR)];//查表
				}
				else if(StartPoint+k < 0)//判断下限
				{
					sum = sum + *(SourceData) * dbSinSheet[(j-1)*SHEET_FACTOR+(k+SIN_FACTOR)];//查表
				}
				else
				{
					sum = sum + *(SourceData+StartPoint+k) * dbSinSheet[(j-1)*SHEET_FACTOR+(k+SIN_FACTOR)];//查表
				}
			}
			TempData[ULONG(i*dbInsertNum)+j] = short(sum);
		}
	}
}


//计算sin表

void CalcSinSheet(double dbInsertNum,double* dbSinSheet)
{
	ULONG i=0;
	long j=0;
	double dbTemp = 0;
	for(i=1;i<dbInsertNum;i++)
	{
		dbTemp = i/dbInsertNum;
		for(j=-SIN_FACTOR;j<=SIN_FACTOR;j++)
		{
			 dbSinSheet[(i-1)*SHEET_FACTOR+(j+SIN_FACTOR)] = (sin((dbTemp-j)*PI)/((dbTemp-j)*PI));
		}
	}
}

*/
