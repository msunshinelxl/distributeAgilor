#include "stdafx.h"
#include "BaseGetValueEntity.h"


BaseGetValueEntity::BaseGetValueEntity(void)
{
	strArrayIsUsed=false;
	totleSize=0;
	type=0;
}


BaseGetValueEntity::~BaseGetValueEntity(void)
{
}
void BaseGetValueEntity::init(){
	if(strArrayIsUsed){
		for(vector<char *>::iterator it = strArray.begin(); it != strArray.end(); ++it) {
			free((*it));
		}
	}
	intArray.clear();
	floatArray.clear();
	boolArray.clear();
	strArray.clear();
	timeArray.clear();
	totleSize=0;
	type=0;
}
void BaseGetValueEntity::pushData(char *dist){
	int st=0;
	vector<int>::iterator itTime=timeArray.begin();
	BaseType tmp;
	
	switch(type){
	case'L':
		{
			
			for(vector<int>::iterator it = intArray.begin(); it != intArray.end()&&itTime!=timeArray.end(); ++it,++itTime){
				intToByte4J((*itTime),(byte *)(dist+st));
				st+=sizeof(int);
				intToByte4J((*it),(byte *)(dist+st));
				st+=sizeof(int);
			}
		}
		break;
	case 'F':
		{
			for(vector<float>::iterator it = floatArray.begin(); it != floatArray.end()&&itTime!=timeArray.end(); ++it,++itTime){
				intToByte4J((*itTime),(byte *)(dist+st));
				st+=sizeof(int);
				tmp.val.fv=(*it);
				for(int i=0;i<4;i++){
					dist[st+i]=tmp.val.tmp[i];
				}
				st+=sizeof(float);
			}
		}
		break;
	case 'S':
		{
			for(vector<char*>::iterator it = strArray.begin(); it != strArray.end()&&itTime!=timeArray.end(); ++it,++itTime){
				intToByte4J((*itTime),(byte *)(dist+st));
				st+=sizeof(int);
				int tmpLength=strlen((*it));
				intToByte4J(tmpLength,(byte *)(dist+st));
				st+=4;
				memcpy(dist+st,(*it),tmpLength);
				st+=tmpLength;
			}
		}
		break;
	case 'B':
		{
			for(vector<bool>::iterator it = boolArray.begin(); it != boolArray.end()&&itTime!=timeArray.end(); ++it,++itTime){
				intToByte4J((*itTime),(byte *)(dist+st));
				st+=sizeof(int);				
				*(dist+st)=(*it);
				st+=sizeof(bool);
			}
		}
		break;
	}
}
void BaseGetValueEntity::praseTagval(TAGVAL tv){
	//char *tmpTime=(char *)calloc(20,sizeof(char));
	//Agtm_Long2DateString(tv.Timestamp,tmpTime);
	//int j=0;
	//int i=0;
	//for(i=0;i+j<20;i++){
	//	if(tmpTime[i+j]=='-'||tmpTime[i+j]==':'||tmpTime[i+j]==' ')
	//		j++;	
	//	tmpTime[i]=tmpTime[i+j];
	//}
	//tmpTime[i]=0;
	timeArray.insert(timeArray.end(),tv.Timestamp);
	totleSize+=sizeof(int);
	switch(tv.TagType){
	case 'L':
		{
			intArray.insert(intArray.end(),tv.lval);
			totleSize+=sizeof(int);
			type='L';
		}
		break;
	case 'S':
		{
			char *tmpStr=(char *)calloc(strlen(tv.sval)+1,sizeof(char));
			strcpy(tmpStr,tv.sval);
			strArray.insert(strArray.end(),tmpStr);
			strArrayIsUsed=true;
			totleSize+=strlen(tmpStr)+4;
			type='S';
		}
		break;
	case'R':
		{
			floatArray.insert(floatArray.end(),tv.rval);
			totleSize+=sizeof(float);
			type='F';
		}
		break;
	case 'B':
		{
			boolArray.insert(boolArray.end(),tv.bval);
			totleSize+=1;
			type='B';
		}
		break;
	}
}

