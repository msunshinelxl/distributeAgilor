#pragma once
class BaseGetValueEntity
{
public:
	BaseGetValueEntity(void);
	~BaseGetValueEntity(void);
	vector<int> intArray;
	vector<char*>strArray;
	vector<float>floatArray;
	vector<bool>boolArray;
	vector<int>timeArray;
	int totleSize;
	void BaseGetValueEntity::praseTagval(TAGVAL);
	void BaseGetValueEntity::pushData(char *dist);
	void BaseGetValueEntity::init();
	bool strArrayIsUsed;
	char type;
};

