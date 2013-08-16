#ifndef KNOWLEDGE_H
#define KNOWLEDGE_H
#include <vector>
#include <list>
#include "KnowledgeRule.h"
#include "KnowledgeCondition.h"
#include "SetOfConditions.h"

class Knowledge {
public:
	Knowledge(int numberOfClasses, bool generateRulesForOtherClasses = false, bool useSpecifiedOnly = false) :
		numberOfClasses(numberOfClasses), generateRulesForOtherClasses(generateRulesForOtherClasses), useSpecifiedOnly(useSpecifiedOnly),
		allowedRules(numberOfClasses), forbiddenRules(numberOfClasses),
		allowedConditions(numberOfClasses), forbiddenConditions(numberOfClasses){}

	int getNumberOfClasses() {
		return numberOfClasses;
	}

	std::vector<SetOfConditions>& getAllowedConditions() {
		return allowedConditions;
	}

	std::vector<std::list<KnowledgeRule> >& getAllowedRules() {
		return allowedRules;
	}

	std::vector<SetOfConditions>& getForbiddenConditions() {
		return forbiddenConditions;
	}

	std::vector<std::list<KnowledgeRule> >& getForbiddenRules() {
		return forbiddenRules;
	}

	bool isGenerateRulesForOtherClasses() const {
		return generateRulesForOtherClasses;
	}

	bool isUseSpecifiedOnly() const {
		return useSpecifiedOnly;
	}

private:
	int numberOfClasses;
	bool generateRulesForOtherClasses;
	bool useSpecifiedOnly;
	std::vector<std::list<KnowledgeRule> > allowedRules;
	std::vector<std::list<KnowledgeRule> > forbiddenRules;
	std::vector<SetOfConditions> allowedConditions;
	std::vector<SetOfConditions> forbiddenConditions;
};

#endif /* KNOWLEDGE_H */
