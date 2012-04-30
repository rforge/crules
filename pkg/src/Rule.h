#ifndef RULE_H
#define	RULE_H
#include "ElementaryCondition.h"
#include "Attribute.h"
#include "Example.h"
#include "DataSet.h"
#include "UsefulFunctions.h"
#include "RulesInductionException.h"
#include <vector>
#include <set>

/**
 * This class represents the decision rule. It contains a vector of lists of elementary conditions
  * (ElementaryCondition class objects). Number of vector element is
  * sttribute index which the condition concerns; lists include conditions
  * for the same attribute. The class also stores the number of decision class
  * which the rule indicates and the confidence degree.
 */
class Rule {
public:
	Rule(): decisionClass(0) {};
    Rule(int noOfAttributes): conditions(noOfAttributes), decisionClass(0) {}
    //Rule(const Rule& orig);
    bool covers(Example&);
    void setDecisionClass(double decisionClass) { this->decisionClass = decisionClass; }
    double getDecisionClass() { return decisionClass; }
    std::vector<std::list<ElementaryCondition > >& getConditions() { return conditions; }
    void addCondition(ElementaryCondition& newCondition);
	void addConditionAndOptimize(ElementaryCondition& newCondition);
    void removeCondition(ElementaryCondition& condition);
    bool operator==(const Rule& secondRule);
    Rule& operator=(const Rule& orig);
    std::string toString();
    std::string toString(DataSet& ds);
    double getConfidenceDegree() { return confidenceDegree; }
    void setConfidenceDegree(double cd) { confidenceDegree = cd; }
    bool containsCondition(ElementaryCondition& condition);
    static Rule parseRule(DataSet& ds, std::string ruleStr);
private:
    std::vector< std::list< ElementaryCondition > > conditions;
    double decisionClass;
    double confidenceDegree;
};

#endif	/* RULE_H */

