#ifndef OPERATOR_H
#define	OPERATOR_H
#include "Attribute.h"
#include <string>

/*
For these operators, we rely on the fact that the double can be NaN. In this case,
 operators >, ==, etc. always return false, which in this case is correct.
*/

/**
 * Abstract class, which is the interface implemented by relational operator classes
 * which take two doubles as arguments.
 */
class RelationalOperator {
public:
    virtual ~RelationalOperator() {};
    virtual bool operator()(double, double) = 0;
    virtual std::string toString() const = 0;
    virtual RelationalOperator* clone() = 0;
};

/**
 * Represents equality operator
 */
class EqualityOperator : public RelationalOperator
{
public:
    EqualityOperator(){};
    bool operator()(double att1, double att2) { return att1 == att2; }
    std::string toString() const { return "="; }
    RelationalOperator* clone() { return new EqualityOperator(); }
};

class LessThanOperator : public RelationalOperator
{
public:
    LessThanOperator(){};
    bool operator()(double att1, double att2) { return att1 < att2; }
    std::string toString() const { return "<"; }
    RelationalOperator* clone() { return new LessThanOperator(); }
};

class GreaterThanOperator : public RelationalOperator
{
public:
    GreaterThanOperator(){};
    bool operator()(double att1, double att2) { return att1 > att2; }
    std::string toString() const { return ">"; }
    RelationalOperator* clone() { return new GreaterThanOperator(); }
};

class LessEqualOperator : public RelationalOperator
{
public:
    LessEqualOperator(){};
    bool operator()(double att1, double att2) { return att1 <= att2; }
    std::string toString() const { return "=<"; }
    RelationalOperator* clone() { return new LessEqualOperator(); }
};

class GreaterEqualOperator : public RelationalOperator
{
public:
    GreaterEqualOperator(){};
    bool operator()(double att1, double att2) { return att1 >= att2; }
    std::string toString() const { return ">="; }
    RelationalOperator* clone() { return new GreaterEqualOperator(); }
};

class InequalityOperator : public RelationalOperator
{
public:
    InequalityOperator(){};
    bool operator()(double att1, double att2) { return att1 != att2; }
    std::string toString() const { return "!="; }
    RelationalOperator* clone() { return new InequalityOperator(); }
};




#endif	/* OPERATOR_H */

