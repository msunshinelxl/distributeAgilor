// MonitorReceiveSocket.cpp: implementation of the CMonitorReceiveSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KernelMonitorv1.h"
#include "MonitorReceiveSocket.h"
#include "KernelMonitorv1Dlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonitorReceiveSocket::CMonitorReceiveSocket()
{

}

CMonitorReceiveSocket::~CMonitorReceiveSocket()
{

}
//void CMonitorReceiveSocket::parseBaseToken(BaseType* res,int *st,int ed,char *data){
//	char tmpType=data[*st];
//	(*st)+=1;
//	switch(tmpType){
//	case COMM_BOOL:
//	case COMM_INT8:
//		res->val.cv=data[*st];
//		(*st)+=1;
//		break;
//	case COMM_INT16:
//		res->val.sv=((data[(*st)+1] << 8) | (data[*st]));
//		(*st)+=2;
//		break;
//	case COMM_INT64:
//	case COMM_INT32:
//	case COMM_FLOAT:						
//		for(int i=0;i<4;i++)
//			res->val.tmp[i]=data[(*st)+i];
//		(*st)+=4;
//		break;
//	case COMM_STRING:
//		res->val.pv=calloc(sizeof(char),data[(*st)+1]+1);
//		memcpy(res->val.pv,data+(*st)+2,data[(*st)+1]);
//		(*st)+=data[(*st)+1]+2;
//		break;
//	}
//}


//void CMonitorReceiveSocket::OnConnect(int nErrorCode)
//{
//	//if (nErrorCode == 0)
//	//{
//	//	Command_Info commandInfo;
//	//	commandInfo.command_id = START_COMMAND;
//	//	this->PutData(&commandInfo, sizeof(Command_Info));
//	//	TAG_INFO tagInfo;
//	//	memset(&tagInfo,0,sizeof(TAG_INFO));
//	//	long tagID;
//	//	char tagName[64];
//	//	CString fullTagName;
//	//	HRECORDSET tags = Agpt_QueryTagsbyDevice("Agilor", ((CKernelMonitorv1Dlg*)m_pDlgPointer)->m_sDeviceName.GetBuffer(0));
//	//	while(Agpt_EnumTagName(tags,&tagID,tagName))
//	//	{
//	//		fullTagName = tagName;
//	//		fullTagName = _T("Agilor.") + fullTagName;
//	//		if (!Agpt_GetTagInfo(fullTagName.GetBuffer(0), &tagInfo))
//	//		{
//	//			if (strcmp(tagInfo.EngUnit, this->m_szUserName) == 0)//"dynamic"
//	//			{
//	//				commandInfo.command_id = ADD_PARAMETER_COMMAND;
//	//				commandInfo.tagid = tagID;
//	//				strcpy_s(commandInfo.szParamName, tagInfo.SourceTag);
//	//				commandInfo.type = tagInfo.TagType;
//	//				this->PutData(&commandInfo, sizeof(Command_Info));
//	//			}
//	//		}
//	//	}
//	//	commandInfo.command_id = END_COMMAND;
//	//	this->PutData(&commandInfo, sizeof(Command_Info));
//	//}
//}

void CMonitorReceiveSocket::OnConnect(int nErrorCode)
{

}

void CMonitorReceiveSocket::OnClose(int nErrorCode)
{
	Stop(FALSE);	// ?
	if (m_pListenSocket != NULL)
		m_pListenSocket->m_acceptCnt = (m_pListenSocket->m_acceptCnt > 0)? m_pListenSocket->m_acceptCnt - 1 : 0;

	// �ͻ���Ϣ���
	struct sockaddr_in _sa;
	int _len = sizeof(_sa);
	CString _ip, _port, _num;
	char _s[32];
	if(!getpeername(m_hSocket, (struct sockaddr *)&_sa, &_len))
	{
		_ip = inet_ntoa(_sa.sin_addr);
		_itoa_s(ntohs(_sa.sin_port), _s, 10);
		_port = _s;
		_itoa_s(m_pListenSocket->m_acceptCnt, _s, 10);
		_num = _s;
		CString _msg = "Delete Client Ip:" + _ip + ", Port:" + _port + ", Num of Clients:" + _num;
		((CKernelMonitorv1Dlg*)m_pDlgPointer)->SendAndStoreMessage(_msg);
	}
	
	Close();	// �رյ�ǰ�Ľ���socket����ʱm_hSocket == INVALID_SOCKET
}

void CMonitorReceiveSocket::OnReceive(int nErrorCode)
{
	int nBytes;
	do
	{
		nBytes = recv(m_hSocket, m_pBuffer + m_lPos, SOCKET_BUFFER_SIZE - m_lPos, 0);


		if ((nBytes != SOCKET_ERROR) && (nBytes > 0))
		{
			m_lPos += nBytes;
			ParsePackage();
		}
	}while ((nBytes != SOCKET_ERROR) && (nBytes > 0));
}

void CMonitorReceiveSocket::ParsePackage()
{
	BOOL bExit = FALSE;
	int iParsePos = 0;
	// struct Monitor_Info* temp;
	TAG_VALUE_LOCAL tagvalue;
	TAG_NODE tagNode;
	TAGVAL tv;
	TAG_INFO ti;
	long result=0;
	COMM_Head* pHead = NULL;
	LPSTR deviceName;
	int PHLength=0;
	LoggerPtr logger(Logger::getLogger(LOG_LOGGER_NAME));
	BaseType *baseType=new BaseType();
	BaseGetValueEntity *bgve=new BaseGetValueEntity();
	do
	{
		if ((m_lPos - iParsePos) < sizeof(COMM_Head))
		{
			bExit = TRUE;
		}
		else
		{
			pHead =(COMM_Head*)(m_pBuffer+iParsePos);
			PHLength=byte2Int(m_pBuffer+iParsePos+1)+5;
			if ((m_lPos - iParsePos) < PHLength)
			{
				bExit = TRUE;
			}
			else
			{
				memset(&tagvalue,0,sizeof(TAG_VALUE_LOCAL));
				//baseType->free();
				memset(baseType,0,sizeof(BaseType));
				switch(pHead->type)	// *(m_pBuffer + iParsePos)
				{
					case COMM_ADD_TAG:
						{
							int st=iParsePos;
							initTagVal(tagNode);
							st+=COMM_HEAD_SIZE;
							//init name sourcetag
							baseType->parseBaseToken(&st,iParsePos+PHLength,m_pBuffer);
							strcpy(tagNode.sourcetag,(char*)(baseType->pv));
							strcpy(tagNode.name,(char*)(baseType->pv));
							//baseType->free();
							//init device name/sourcegroup
							baseType->parseBaseToken(&st,iParsePos+PHLength,m_pBuffer);
							strcpy(tagNode.sourcegroup,(char*)(baseType->pv));
							strcpy(tagNode.sourceserver,(char*)(baseType->pv));
							//baseType->free();

							int addTmpRes=Agpt_AddNewTag(((CKernelMonitorv1Dlg*)m_pDlgPointer)->getACIServerName(),&tagNode);
							char res[6];
							res[0]=COMM_ADD_TAG;
							intToByte4J(1,(byte *)&(res[1]));
							res[5]=addTmpRes==0?RES_OK:RES_FAIL_COMMON;
							if(res[1]==0){
								int sizeTmp=((CKernelMonitorv1Dlg*)m_pDlgPointer)->deviceNames.size();
								int tmpI=0;
								for(tmpI=0;tmpI<sizeTmp;tmpI++){
									if(((CKernelMonitorv1Dlg*)m_pDlgPointer)->deviceNames[tmpI].compare(tagNode.sourceserver)==0){
										break;
									}
								}
								if(tmpI==sizeTmp){
									((CKernelMonitorv1Dlg*)m_pDlgPointer)->deviceNames.insert(((CKernelMonitorv1Dlg*)m_pDlgPointer)->deviceNames.begin(),tagNode.sourceserver);
									DRTDB_RegisterDevice(((CKernelMonitorv1Dlg*)m_pDlgPointer)->m_sServerIp.GetBuffer(0),((CKernelMonitorv1Dlg*)m_pDlgPointer)->m_uServerPort,tagNode.sourceserver);
								}
							}

							this->PutData(res,6);
							memset(&tagNode,0,sizeof(TAG_NODE));
						}
						break;
					case COMM_ADD_VALUE:
						{

							LOG4CXX_DEBUG(logger, "Enter COMM_ADD_VALUE");
							int st=iParsePos;
							st+=COMM_HEAD_SIZE;
							baseType->parseBaseToken(&st,iParsePos+PHLength,m_pBuffer);
							strcpy(tagvalue.szTagSource,(char*)(baseType->pv));

							LOG4CXX_DEBUG(logger, tagvalue.szTagSource);

							//baseType->free();
							baseType->parseBaseToken(&st,iParsePos+PHLength,m_pBuffer);
							deviceName=(char*)(baseType->pv);

							LOG4CXX_DEBUG(logger, deviceName);

#ifdef  LOGOPEN
	std::ostringstream ss;
	ss << tagvalue.fValue;
	LOG4CXX_DEBUG(logger, ss.str());
#endif
							
							
							char tmpType=baseType->parseBaseToken(&st,iParsePos+PHLength,m_pBuffer);
							getTagValueFromBaseType(tagvalue, *baseType,tmpType);

							result = DRTDB_MD_SendNewValue(deviceName,tagvalue, true);

							char res[6];
							res[0]=COMM_ADD_VALUE;
							intToByte4J(1,(byte *)&(res[1]));
							res[5]=result==0?RES_OK:RES_FAIL_COMMON;
							this->PutData(res,6);
							//this->SendData(0);
						}
						break;
					case COMM_GET_VALUE:
						{
							LOG4CXX_DEBUG(logger, "Enter COMM_GET_VALUE");
							bgve->init();
							int st=iParsePos+COMM_HEAD_SIZE;
							baseType->parseBaseToken(&st,iParsePos+PHLength,m_pBuffer);
							int stTime=baseType->val.lv;
							baseType->parseBaseToken(&st,iParsePos+PHLength,m_pBuffer);
							int edTime=baseType->val.lv;
							baseType->parseBaseToken(&st,iParsePos+PHLength,m_pBuffer);
							char* strs[3];
							strs[2]=(char *)(baseType->pv);
							strs[1]=".";
							CT2A ascii_tmp(((CKernelMonitorv1Dlg*)m_pDlgPointer)->getACIServerName());
							strs[0]=ascii_tmp.m_psz;
							char* tmpTagName=myStrCat(strs,3);
							int nRs=Agda_QueryTagHistory((LPCTSTR)tmpTagName,stTime,edTime);
							//free(tmpTagName);
							//long n = Agpt_GetTagInfo((char *)(baseType->pv), &ti);
							
							if(nRs > 0) //�����ѯ�ɹ����е���Ϣ��
							{
								while(Agda_GetNextTagValue(nRs,&tv)) //ȡ�ý������һ����Ϣ
								{  
									bgve->praseTagval(tv);
								}
							}
							
							char *outPut=(char *)calloc(bgve->totleSize+6+(nRs>0?bgve->totleSize!=0?6:0:0),sizeof(char));
							outPut[0]=COMM_GET_VALUE;
							intToByte4J(bgve->totleSize+1+(bgve->totleSize!=0?6:0),(byte *)&(outPut[1]));
							outPut[5]=nRs>0?bgve->totleSize!=0?RES_OK:RES_FAIL_RETURN_EMPTY:(nRs==-5?RES_FAIL_NO_TAG:RES_FAIL_COMMON);
							if(outPut[5]==RES_OK){
								outPut[6]='A';
								intToByte4J(bgve->totleSize,(byte *)outPut+7);
								outPut[11]=bgve->type;
								bgve->pushData(outPut+12);
							}
							this->PutData(outPut,bgve->totleSize+6+(outPut[5]==RES_OK?6:0));
							
							free(outPut);
						}
						break;
					//case COMM_UPDATE_STRING:
					//	{
					//		RealTimeData_S* p;
					//		p = (RealTimeData_S*)(m_pBuffer + iParsePos);
					//		strcpy(tagvalue.szTagSource,p->tagSourceName);
					//		tagvalue.cTagType = 'S';
					//		tagvalue.lTimeStamp = p->timestamp;
					//		deviceName=p->deviceName;
					//		strcpy_s(tagvalue.szValue, m_pBuffer + iParsePos + sizeof(RealTimeData_S));
					//	}
					//	break;
					//case COMM_UPDATE_LONG:
					//	{
					//		RealTimeData_L* p;
					//		p = (RealTimeData_L*)(m_pBuffer + iParsePos);
					//		//tagvalue.lTagID = p->tagid;
					//		strcpy(tagvalue.szTagSource,p->tagSourceName);
					//		tagvalue.cTagType = 'L';
					//		char* byteArray=m_pBuffer + iParsePos+162;
					//		int tmpInt=((byteArray[3]) << 24) | (byteArray[2] << 16) | (byteArray[1] << 8) | (byteArray[0]);
					//		//UStuff tmp;
					//		//for(int i=0;i<4;i++)
					//		//	tmp.c[i]=byteArray[i];
					//		tagvalue.lTimeStamp = tmpInt;
					//		deviceName=p->deviceName;
					//		byteArray=m_pBuffer + iParsePos+166;
					//		tmpInt=((byteArray[3]) << 24) | (byteArray[2] << 16) | (byteArray[1] << 8) | (byteArray[0]);
					//		//for(int i=0;i<4;i++)
					//		//	tmp.c[i]=byteArray[i];
					//		tagvalue.lValue = tmpInt;
					//	}
					//	break;
					//case COMM_UPDATE_BOOL:
					//	{
					//		RealTimeData_B* p;
					//		p = (RealTimeData_B*)(m_pBuffer + iParsePos);
					//		//tagvalue.lTagID = p->tagid;
					//		strcpy(tagvalue.szTagSource,p->tagSourceName);
					//		tagvalue.cTagType = 'B';
					//		tagvalue.lTimeStamp = p->timestamp;
					//		deviceName=p->deviceName;
					//		tagvalue.bValue = p->bval;
					//	}
					//	break;
					//case COMM_UPDATE_FLOAT:
					//	{
					//		RealTimeData_R* p;
					//		p = (RealTimeData_R*)(m_pBuffer + iParsePos);
					//		//tagvalue.lTagID = p->tagid;
					//		strcpy(tagvalue.szTagSource,p->tagSourceName);
					//		tagvalue.cTagType = 'R';	// -- jwj -- L
					//		char* byteArray=m_pBuffer + iParsePos+162;
					//		int tmpInt=((byteArray[3]) << 24) | (byteArray[2] << 16) | (byteArray[1] << 8) | (byteArray[0]);
					//		tagvalue.lTimeStamp =tmpInt;
					//		deviceName=p->deviceName;
					//		byteArray=m_pBuffer + iParsePos+166;
					//		UStuff tmp;
					//		for(int i=0;i<4;i++)
					//			tmp.c[i]=byteArray[i];
					//		tagvalue.fValue = tmp.f;
					//	}
					//	break;
					case COMM_PING:
						{
							((CKernelMonitorv1Dlg*)m_pDlgPointer)->pingCount=((CKernelMonitorv1Dlg*)m_pDlgPointer)->pingCount+1;
						}
						break;
					default:
						break;
				}
				//if (COMM_UPDATE_FLOAT == pHead->type||(COMM_UPDATE_BOOL == pHead->type)||(COMM_UPDATE_LONG == pHead->type)||(COMM_UPDATE_STRING==pHead->type)) {
				//	result = DRTDB_MD_SendNewValue(deviceName,tagvalue, true);
				//	//TAG_VALUE_LOCAL tag;
				//	//tag.fValue = 1234.5f;
				//	//tag.cTagType = 'R';
				//	//const clock_t begin_time = clock();
				//	//long timeCount = 0;
				//	//for (int j = 0; j < 10; j++){
				//	//	for (int i = 1; i <= 1000; i++){
				//	//		string nodeName = "Simu11_";
				//	//		char tmpBuf[4];
				//	//		for(int k=0;k<4;k++)
				//	//			tmpBuf[k]=0;
				//	//		itoa(i,tmpBuf,10);
				//	//		nodeName.append(tmpBuf);
				//	//		strcpy_s(tag.szTagSource, sizeof(tag.szTagSource), nodeName.data());
				//	//		tag.fValue = (float)i;
				//	//		long r = DRTDB_MD_SendNewValue("Simu11", tag, true);
				//	//	}
				//	//}
				//}
				BOOL bErrorPack = FALSE;
				if (result != 0) {
					bErrorPack = TRUE;
					char tmp_s[16];
					CString ret_str, tagid_str;
					_ltoa_s(result, tmp_s, 10);
					ret_str = tmp_s;
					_ltoa_s(tagvalue.lTagID, tmp_s, 10);
					tagid_str = tmp_s;

					CString tmp_str = "Error Frame, Return code : " + ret_str + ", TagID : " + tagid_str;
					((CKernelMonitorv1Dlg*)m_pDlgPointer)->SendAndStoreMessage(tmp_str);
				}
				((CKernelMonitorv1Dlg*)m_pDlgPointer)->AddPack(PHLength, bErrorPack);	// m_lPackCount m_lDataSize
				iParsePos += PHLength;
			}
		}
    }while (!bExit);
	baseType->~BaseType();
	//DRTDB_Flush();
	if (m_lPos >= iParsePos)
	{
		memcpy(m_pBuffer, m_pBuffer + iParsePos, m_lPos - iParsePos);
		m_lPos = m_lPos - iParsePos;
	}
	return;
}
