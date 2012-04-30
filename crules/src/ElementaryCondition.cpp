#include "ElementaryCondition.h"
using namespace std;


ElementaryCondition::ElementaryCondition(int attributeIndex, RelationalOperator* _operator, double attributeValue)
{
    this->attributeIndex = attributeIndex;
    this->_operator = _operator;
    this->attributeValue = attributeValue;
}

ElementaryCondition::ElementaryCondition(const ElementaryCondition& orig)
{
    attributeIndex = orig.attributeIndex;
    attributeValue = orig.attributeValue;
	if(orig._operator != NULL)
		_operator = orig._operator->clone();
	else
		_operator = NULL;
}

ElementaryCondition::~ElementaryCondition()
{
	delete _operator;
}

ElementaryCondition& ElementaryCondition::operator =(const ElementaryCondition& orig)
{
	if(*this == orig)
		return *this;
    delete _operator;
    attributeIndex = orig.attributeIndex;
    attributeValue = orig.attributeValue;
	if(orig._operator != NULL)
		_operator = orig._operator->clone();
	else
		_operator = NULL;
	return *this;
}

bool ElementaryCondition::operator ==(const ElementaryCondition& toCompare) const
{
    if (toCompare._operator == NULL || _operator == NULL) return false;
    if (
            (attributeIndex == toCompare.attributeIndex) &&
            (attributeValue == toCompare.attributeValue) &&
            (typeid (*_operator) == typeid (*(toCompare._operator)))
            )
        return true;
    else
        return false;
}


string ElementaryCondition::toString()
{
	ostringstream str;
    str << "[" << attributeIndex << "] ";
    str << _operator->toString();
    str << " " << attributeValue ;
	return str.str();
}

string ElementaryCondition::toString(DataSet& ds)
{
    ostringstream str;
    str << ds.getConditionalAttribute(attributeIndex).getName() << " ";
    str << _operator->toString();
    str << " " << ds.getConditionalAttribute(attributeIndex).getStringValue(attributeValue);
	return str.str();
}


void ElementaryCondition::print()
{
	cout << this->toString();
}
