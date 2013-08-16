#include "SetOfConditions.h"

std::list<KnowledgeCondition> SetOfConditions::getConditionsForAttribute(int attributeIndex)
{
	std::list<KnowledgeCondition> result;

	for(std::list<KnowledgeCondition>::iterator it = conditions.begin(); it != conditions.end(); it++)
		if(it->getAttributeIndex() == attributeIndex)
			result.push_back(*it);

	return result;
}
