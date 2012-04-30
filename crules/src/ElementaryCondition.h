#ifndef ELEMENTARYCONDITION_H
#define	ELEMENTARYCONDITION_H
#include "Attribute.h"
#include "SetOfExamples.h"
#include "Operator.h"
#include "DataSet.h"
#include <map>
#include <typeinfo>


/**
 * Represents elementary condition (in a rule). Contains index of the attribute it concerns, relational operator and
 * value of the attribute.
 */
class ElementaryCondition
{
public:
    ElementaryCondition(): attributeIndex(-1), _operator(0), attributeValue(0) {};
    ElementaryCondition(int attributeIndex, RelationalOperator* _operator, double attributeValue);
    ElementaryCondition(const ElementaryCondition&);
    ~ElementaryCondition();
    /**
      * Main method of the class. Checks if the value satisfies the condition.
      * @param value the value that is tested.
      * @return true - if the condition is true, false - otherwise.
     */
    bool isSatisfied(double value){ return (*_operator)(value, attributeValue); }
    ElementaryCondition& operator=(const ElementaryCondition&);
    bool operator==(const ElementaryCondition&)  const;
    void setAttributeIndex(int attributeIndex) { this->attributeIndex = attributeIndex; }
    int getAttributeIndex() const { return attributeIndex; }
    void setOperator(RelationalOperator* _operator) { this->_operator = _operator; }
    RelationalOperator* getOperator() const { return _operator; }
    void setAttributeValue(double attributeValue) { this->attributeValue = attributeValue; }
    double getAttributeValue() const { return attributeValue; }
    void print();
    std::string toString();
    std::string toString(DataSet&);
private:
    int attributeIndex;
    RelationalOperator* _operator;
    double attributeValue;
};

#endif	/* ELEMENTARYCONDITION_H */

