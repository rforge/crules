#include "Rule.h"

using namespace std;

/**
 * Adds new elementary condition to the rule.
 * @param newCondition elementary condition to be added to the rule
 */
void Rule::addCondition(ElementaryCondition& newCondition) {
	unsigned int attIndex = newCondition.getAttributeIndex();
	if (attIndex + 1 > conditions.size())
		conditions.resize(attIndex + 1);
	conditions[attIndex].push_back(newCondition);
}

/**
 * Adds new elementary condition to the rule and removes redundant conditions.
 * @param newCondition elementary condition to be added to the rule
 */
void Rule::addConditionAndOptimize(ElementaryCondition& newCondition) {
	list<ElementaryCondition>::iterator it;
	unsigned int attIndex = newCondition.getAttributeIndex();
	if (attIndex + 1 > conditions.size()) {
		conditions.resize(attIndex + 1);
		conditions[attIndex].push_back(newCondition);
	} else {
		if (typeid(*(newCondition.getOperator())) == typeid(EqualityOperator)) {
			conditions[attIndex].push_back(newCondition);
			return;
		}
		for (it = conditions[attIndex].begin();
				it != conditions[attIndex].end(); it++) {
			if (typeid (*(it->getOperator()))
					== typeid (*(newCondition.getOperator()))) {
				if (it->getOperator()->operator()(it->getAttributeValue(),
						newCondition.getAttributeValue()))
					return;
				else if (it->getOperator()->operator()(
						newCondition.getAttributeValue(),
						it->getAttributeValue())) { //usuwamy it na rzecz newCondition
					conditions[attIndex].remove(*it);
					conditions[attIndex].push_back(newCondition);
					return;
				}
			}
		}
		conditions[attIndex].push_back(newCondition);
	}
}

/**
 * Removes elementary condition from the rule
 * @param condition elementary condition to be removed from the rule
 */
void Rule::removeCondition(ElementaryCondition& condition) {
	int attIndex = condition.getAttributeIndex();
	list<ElementaryCondition>::iterator it;
	for (it = conditions[attIndex].begin(); it != conditions[attIndex].end();
			it++) {
		if (*it == condition) {
			conditions[attIndex].erase(it);
			break;
		}
	}
}

Rule& Rule::operator =(const Rule& orig) {
	if (*this == orig)
		return *this;
	conditions = orig.conditions;
	setDecisionClass(orig.decisionClass);
	setConfidenceDegree(orig.confidenceDegree);
	return *this;
}

bool Rule::operator ==(const Rule& secondRule) {
	if (confidenceDegree != secondRule.confidenceDegree)
		return false;
	if (decisionClass != secondRule.decisionClass)
		return false;
	return conditions == secondRule.conditions;
}

/**
 * Tests if the rule covers an example
 * @param example example
 * @return true - if the rule covers an example; fałsz - otherwise
 */
bool Rule::covers(Example& example) {
	list<ElementaryCondition>::iterator itCond;
	vector<list<ElementaryCondition> >::iterator itVec;
	for (itVec = conditions.begin(); itVec != conditions.end(); itVec++) {
		for (itCond = itVec->begin(); itCond != itVec->end(); itCond++) {
			if (!(itCond->isSatisfied(
					example.getAttribute(itCond->getAttributeIndex()))))
				return false;
		}
	}
	return true;
}

bool Rule::containsCondition(ElementaryCondition& condition) {
	vector<list<ElementaryCondition> >::iterator itVec;
	list<ElementaryCondition>::iterator itList;
	for (itVec = conditions.begin(); itVec != conditions.end(); itVec++)
		for (itList = itVec->begin(); itList != itVec->end(); itList++)
			if (condition == *itList)
				return true;
	return false;
}

string Rule::toString() {
	string str;
	str.append("IF ");
	list<ElementaryCondition>::iterator itCond;
	for (unsigned int i = 0; i < conditions.size(); i++) {
		for (itCond = conditions[i].begin(); itCond != conditions[i].end();
				itCond++) {
			str.append(itCond->toString());
			str.append(" AND ");
		}
	}
	if (str.size() > 3)
		str.resize(str.size() - 4);
	str.append("THEN ");
	ostringstream oss;
	oss << decisionClass;
	str.append(oss.str());
	return str;
}

string Rule::toString(DataSet& ds) {
	ostringstream oss;
	string str;
	str.append("IF ");
	list<ElementaryCondition>::iterator itCond;
	for (unsigned int i = 0; i < conditions.size(); i++) {
		if (ds.getConditionalAttribute(i).getType() == Attribute::NUMERICAL
				&& conditions[i].size() > 1) {
			ElementaryCondition* condMin, *condMax;
			condMin = condMax = &(conditions[i].front());
			itCond = conditions[i].begin();
			for (itCond++; itCond != conditions[i].end(); itCond++) {
				if (itCond->getAttributeValue() > condMax->getAttributeValue())
					condMax = &(*itCond);
				else if (itCond->getAttributeValue()
						< condMin->getAttributeValue())
					condMin = &(*itCond);
			}
			str.append(ds.getConditionalAttribute(i).getName() + " in [ ");
			oss << condMin->getAttributeValue() << " ; "
					<< condMax->getAttributeValue() << " )";
			str.append(oss.str());
			oss.str("");
			str.append(" AND ");
		} else
			for (itCond = conditions[i].begin(); itCond != conditions[i].end();
					itCond++) {
				str.append(itCond->toString(ds));
				str.append(" AND ");
			}
	}
	if (str.size() > 3)
		str.resize(str.size() - 4);
	str.append("THEN ");

	oss << ds.getDecisionAttribute().getStringValue(decisionClass);
	str.append(oss.str());
	return str;
}

Rule Rule::parseRule(DataSet& ds, string ruleStr) {
	unsigned int attCount = ds.getAttributes().size() - 1;
	unsigned int attIndex;
	string op;
	RelationalOperator* opPtr;
	Rule rule(attCount);
	vector<string> strings = UsefulFunctions::splitString(ruleStr, " \t[;)");
	double val;
	for (unsigned int i = 0; i < strings.size() && strings[i] != "THEN"; i++) {
		i++;
		for (attIndex = 0; attIndex < attCount; attIndex++)
			if (strings[i] == ds.getConditionalAttribute(attIndex).getName())
				break;
		if (attIndex == attCount
				&& strings[i]
						!= ds.getConditionalAttribute(attIndex - 1).getName())
			throw RulesInductionException("Wrong attribute name");
		i++;
		op = strings[i];

		if (op == "in") {
			val = ds.getConditionalAttribute(attIndex).getDoubleValue(
					strings[++i]);   //may throw an exception
			ElementaryCondition cond(attIndex, new GreaterEqualOperator(), val);
			rule.addCondition(cond);
			op = "<";
		}

		i++;
		val = ds.getConditionalAttribute(attIndex).getDoubleValue(strings[i]); ///may throw an exception

		if (op == "=")
			opPtr = new EqualityOperator();
		else if (op == "<")
			opPtr = new LessThanOperator();
		else if (op == ">=")
			opPtr = new GreaterEqualOperator();
		else
			throw RulesInductionException("Wrong operator symbol");

		ElementaryCondition cond(attIndex, opPtr, val);
		rule.addCondition(cond);
	}
	rule.setDecisionClass(
			ds.getDecisionAttribute().getDoubleValue(strings.back()));
	return rule;
}
