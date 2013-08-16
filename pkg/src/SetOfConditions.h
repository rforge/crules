#ifndef SETOFCONDITIONS_H_
#define SETOFCONDITIONS_H_
#include <list>
#include <limits>
#include "KnowledgeCondition.h"

class SetOfConditions {
public:
	SetOfConditions() : decisionClass(std::numeric_limits<double>::quiet_NaN()), expandable(true), rulesAtLeast(0), forbidden(false) {}
	SetOfConditions(const std::list<KnowledgeCondition>& conditions, double decisionClass, bool expandable, int rulesAtLeast, bool forbidden) :
		conditions(conditions), decisionClass(decisionClass), expandable(expandable), rulesAtLeast(rulesAtLeast), forbidden(forbidden) {}
	SetOfConditions(double decisionClass, bool expandable, int rulesAtLeast, bool forbidden) :
			decisionClass(decisionClass), expandable(expandable), rulesAtLeast(rulesAtLeast), forbidden(forbidden) {}
	SetOfConditions(const SetOfConditions& orig) :SetOfConditions(orig.conditions, orig.decisionClass, orig.expandable, orig.rulesAtLeast, orig.forbidden) {}
	virtual ~SetOfConditions() {}

	std::list<KnowledgeCondition>& getConditions() {
		return conditions;
	}

	void setConditions(const std::list<KnowledgeCondition>& conditions) {
		this->conditions = conditions;
	}

	double getDecisionClass() const {
		return decisionClass;
	}

	void setDecisionClass(double decisionClass) {
		this->decisionClass = decisionClass;
	}

	bool isExpandable() const {
		return expandable;
	}

	void setExpandable(bool expandable) {
		this->expandable = expandable;
	}

	bool isForbidden() const {
		return forbidden;
	}

	void setForbidden(bool forbidden) {
		this->forbidden = forbidden;
	}

	int getRulesAtLeast() const {
		return rulesAtLeast;
	}

	void setRulesAtLeast(int rulesAtLeast) {
		this->rulesAtLeast = rulesAtLeast;
	}

	std::list<KnowledgeCondition> getConditionsForAttribute(int attributeIndex);

private:
	std::list<KnowledgeCondition> conditions;
	double decisionClass;
	bool expandable;
	int rulesAtLeast;
	bool forbidden;
};
#endif /* SETOFCONDITIONS_H_ */
