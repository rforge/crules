#ifndef KNOWLEDGERULE_H_
#define KNOWLEDGERULE_H_
#include "SetOfConditions.h"

class KnowledgeRule {
public:
	KnowledgeRule(){}
	KnowledgeRule(const SetOfConditions& conditions) : conditions(conditions) {}
	virtual ~KnowledgeRule(){}

	SetOfConditions& getConditions() {
		return conditions;
	}

	void setConditions(const SetOfConditions& conditions) {
		this->conditions = conditions;
	}

private:
	SetOfConditions conditions;
};
#endif /* KNOWLEDGERULE_H_ */
