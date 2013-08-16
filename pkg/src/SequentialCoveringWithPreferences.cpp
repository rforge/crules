#include "SequentialCoveringWithPreferences.h"

using namespace std;

/**
 * Generates list of rules for every decision class
 * @param examples training set on which the induction is based
 * @param rqmGrow rule qualisty measure used in growth phase
 * @param rqmPrune rule qualisty measure used in pruning phase
 * @return ist of rules for every decision class
 */
list<Rule> SequentialCoveringWithPreferences::generateRules(SetOfExamples& examples, RuleQualityMeasure& rqmGrow, RuleQualityMeasure& rqmPrune)
{
    list<Rule> ruleSet;
    vector<double> classes = examples.getDistinctClasses();
    vector<double>::iterator it;
    for(it = classes.begin(); it != classes.end(); it++)
    {
    	if(!knowledge->isGenerateRulesForOtherClasses() && knowledge->getAllowedRules()[*it].size() == 0 && knowledge->getAllowedConditions()[*it].getConditions().size() == 0
    			&& knowledge->getForbiddenRules()[*it].size() == 0 && knowledge->getForbiddenConditions()[*it].getConditions().size() == 0)
    		continue;

        list<Rule> rulesForClass(generateRulesForClass(examples, rqmGrow, rqmPrune, *it));
        ruleSet.insert(ruleSet.end(), rulesForClass.begin(), rulesForClass.end());
    }
    return ruleSet;
}

/**
 * Generates list of rules for given decision class
 * @param examples training set on which the induction is based
 * @param rqmGrow rule qualisty measure used in growth phase
 * @param rqmPrune rule qualisty measure used in pruning phase
 * @param decClass decision class value
 * @return ist of rules for the decision class
 */
list<Rule> SequentialCoveringWithPreferences::generateRulesForClass(SetOfExamples& examples, RuleQualityMeasure& rqmGrow, RuleQualityMeasure& rqmPrune, double decClass)
{
    list<Rule> ruleSet;
    SetOfExamples uncoveredPositives(examples.getExamplesForDecAtt(decClass));
    P = uncoveredPositives.getSumOfWeights();
    N = examples.getSumOfWeights() - P;
    double apriori = P / (P + N);
    Precision precision;
    vector<list<ElementaryCondition> >::iterator itVec;

    SetOfConditions& allowedConditions = knowledge->getAllowedConditions()[decClass];

    bool useSpecifiedOnly = allowedConditions.getConditions().size() > 0;
    int rulesFromSpecifiedConditionsCount = 0;

    //insert specified rules to ruleset and compute uncoveredPositives
    Rule* tempRule;
    for(list<KnowledgeRule>::iterator it = knowledge->getAllowedRules()[decClass].begin(); it != knowledge->getAllowedRules()[decClass].end(); it++)
    {
    	tempRule = getRuleFromKnowledgeRule(*it);

    	SetOfExamples covered(getCoveredExamples(*tempRule, examples));
    	uncoveredPositives = uncoveredPositives - covered;
    	tempRule->setConfidenceDegree(rqmPrune.EvaluateRuleQuality(examples, *tempRule));
		ruleSet.push_back(*tempRule);

    	delete tempRule;
    }

    //try to improve specified rules
    list<KnowledgeRule>::iterator itKnowRule = knowledge->getAllowedRules()[decClass].begin();
    for(list<Rule>::iterator itRule = ruleSet.begin(); itRule != ruleSet.end(); itRule++, itKnowRule++)
	{
    	if(!itKnowRule->getConditions().isExpandable())
    		continue;	//if not expandable, don't try to improve it

    	SetOfExamples covered(examples);
    	Rule rule(*itRule);

    	//temporarily add conditions from rule to "allowed conditions" so their "fixed" and "required" properties are respected
    	allowedConditions.getConditions().insert(allowedConditions.getConditions().end(), itKnowRule->getConditions().getConditions().begin(),
    			itKnowRule->getConditions().getConditions().end());

    	growRule(rule, covered, uncoveredPositives, rqmGrow, useSpecifiedOnly, &(*itKnowRule));
    	pruneRule(rule, examples, rqmPrune);

        covered = getCoveredExamples(rule, examples);

        if(precision.EvaluateRuleQuality(covered, *itRule) <= apriori && rulesFromSpecifiedConditionsCount >= allowedConditions.getRulesAtLeast())
        {
        	//if adding new rule from the specified conditions will give worse precision than apriori
        	//then if not "useSpecifiedOnly", start using also other conditions
        	if(useSpecifiedOnly && !knowledge->isUseSpecifiedOnly())
        		useSpecifiedOnly = false;
        }
        else
        {
			uncoveredPositives = uncoveredPositives - covered;
			rule.setConfidenceDegree(rqmPrune.EvaluateRuleQuality(examples, rule));
			*itRule = rule;
			if(useSpecifiedOnly)
				rulesFromSpecifiedConditionsCount++;
        }

        //remove temporarily added conditions
        list<KnowledgeCondition>::iterator remBegin = allowedConditions.getConditions().end();
        for(unsigned int i = 0; i < itKnowRule->getConditions().getConditions().size(); i++) remBegin--;
        allowedConditions.getConditions().erase(remBegin, allowedConditions.getConditions().end());
	}

    //create new rules
    if(knowledge->getAllowedRules()[decClass].size() == 0 || !knowledge->isUseSpecifiedOnly())
    {
		while (uncoveredPositives.size() != 0)
		{
			SetOfExamples covered(examples);
			Rule rule;
			rule.setDecisionClass(decClass);
			growRule(rule, covered, uncoveredPositives, rqmGrow, useSpecifiedOnly);
			pruneRule(rule, examples, rqmPrune);
			covered = getCoveredExamples(rule, examples);

			if(precision.EvaluateRuleQuality(covered, rule) <= apriori && rulesFromSpecifiedConditionsCount >= allowedConditions.getRulesAtLeast())
			{
				//if adding new rule from the specified conditions will give worse precision than apriori
				//then if not "useSpecifiedOnly", start using also other conditions
				if(useSpecifiedOnly && !knowledge->isUseSpecifiedOnly())
					useSpecifiedOnly = false;
				else
					break;
			}
			else
			{
				uncoveredPositives = uncoveredPositives - covered;
				rule.setConfidenceDegree(rqmPrune.EvaluateRuleQuality(examples, rule));
				ruleSet.push_back(rule);
				if(useSpecifiedOnly)
					rulesFromSpecifiedConditionsCount++;
			}
			//cout << rule.toString(examples.getDataSet()) << endl;
		}
    }
    return ruleSet;
}

/**
 * Growth phase of a rule induction
 * @param rule rule to be built
 * @param covered set of examples covered by current rule
 * @param uncoveredPositives set of examples not covered by current set of rules
 * @param ruleQualityMeasure rule quality measure
 */
void SequentialCoveringWithPreferences::growRule(Rule& rule, SetOfExamples& covered, SetOfExamples uncoveredPositives, RuleQualityMeasure& ruleQualityMeasure, bool useSpecifiedOnly, KnowledgeRule* knowRule)
{
    Precision prec;
    ElementaryCondition bestCondition;
    double decClass = rule.getDecisionClass();
    RuleEvaluationResult rer;
    double coveredCount = 0, prevCoveredCount = 0;
    bool isEntropy = typeid (ruleQualityMeasure) == typeid (NegConditionalEntropy);

    while (1)
    {
        bestCondition = findBestCondition(rule, decClass, covered, uncoveredPositives, ruleQualityMeasure, isEntropy,  useSpecifiedOnly, knowRule);

        //if it wasn't possible to find best condition from specified, then if conditions are "expandable"
        //and "useSpecifiedOnly" (global, not this method parameter) is not true, try to use other conditions
        if (bestCondition.getAttributeIndex() == -1 && knowledge->getAllowedConditions()[decClass].isExpandable() && !knowledge->isUseSpecifiedOnly() && useSpecifiedOnly)
        	bestCondition = findBestCondition(rule, decClass, covered, uncoveredPositives, ruleQualityMeasure, isEntropy, false, knowRule);

        if(bestCondition.getAttributeIndex() == -1)
            break;
        //cout << "Best condition:" << bestCondition.toString(covered.getDataSet()) << endl;

        //checking stop criterion
        rer = RuleQualityMeasure::EvaluateCondition(covered, bestCondition, decClass);
        if (rer.n == 0)
        {
            rule.addCondition(bestCondition);
        	//rule.addConditionAndOptimize(bestCondition);
            //cout << "Added condition: " << bestCondition.toString(covered.getDataSet()) << endl;
            break;
        }
        coveredCount = rer.p + rer.n;
        if(coveredCount == prevCoveredCount)
            break;

        covered = getCoveredExamples(bestCondition, covered);
        uncoveredPositives = getCoveredExamples(bestCondition, uncoveredPositives);
        prevCoveredCount = coveredCount;
        rule.addCondition(bestCondition);
        //rule.addConditionAndOptimize(bestCondition);
        //cout << "Added condition: " << bestCondition.toString(covered.getDataSet()) << endl;
    }
}


/**
 * Finds the best elementary condition from all possible ones
 * @param decClass number of positive class
 * @param covered set of examples covered by current rule
 * @param uncoveredPositives set of examples not covered by current set of rules but covered by the current rule
 * @param rqm rule quality measure
 * @return the best elementary condition
 */
ElementaryCondition SequentialCoveringWithPreferences::findBestCondition
(Rule& rule, double decClass, SetOfExamples& covered, SetOfExamples& uncoveredPositives, RuleQualityMeasure& rqm, bool isRqmEntropy, bool useSpecifiedOnly, KnowledgeRule* knowRule)
{
    ElementaryCondition bestCondition;
    list<ElementaryCondition> equallyBestConditions;
    double bestQuality = -numeric_limits<double>::max();

    int size = covered.size();

    if (size == 0)
        return bestCondition;
    int numberOfAtts = covered[0].getAttributes().size();

    for (int i = 0; i < numberOfAtts; i++)
    {
        Attribute::AttributeType attributeType = covered.getAttributeType(i);
        switch (attributeType)
        {
			case Attribute::NUMERICAL:
				//check if there is a specified rule with a required and fixed condition for this attribute, if so skip this attribute
				if(knowRule != NULL)
				{
					list<KnowledgeCondition> knowConds = knowRule->getConditions().getConditionsForAttribute(i);
					for(list<KnowledgeCondition>::iterator itCond = knowConds.begin(); itCond != knowConds.end(); itCond++)
						if(itCond->isFixed() && itCond->isRequired())
							continue;
				}
				findBestConditionForNumericalAttribute(rule, decClass, covered, uncoveredPositives, rqm, isRqmEntropy, i, equallyBestConditions, bestQuality, useSpecifiedOnly);
				break;
			case Attribute::NOMINAL:
				findBestConditionForNominalAttribute(rule, decClass, covered, uncoveredPositives, rqm, isRqmEntropy, i, equallyBestConditions, bestQuality, useSpecifiedOnly);
				break;
        }
    }

    //when more than one condition is the best
    if (equallyBestConditions.size() == 1)
        bestCondition = equallyBestConditions.front();
    else if (equallyBestConditions.size() > 1)
        bestCondition = chooseConditionFromEqual(equallyBestConditions, decClass, uncoveredPositives);

    return bestCondition;
}

void SequentialCoveringWithPreferences::findBestConditionForNumericalAttribute
(Rule& rule, double decClass, SetOfExamples& covered, SetOfExamples& uncoveredPositives,
		RuleQualityMeasure& rqm, bool isRqmEntropy, int attributeIndex, list<ElementaryCondition>& equallyBestConditions, double& bestQuality, bool useSpecifiedOnly)
{
	double mean, quality, ltQuality, currWeight;
	ElementaryCondition bestCondition;
	bool shouldSkip = false;
	int entrLT_p = 0, entrGE_p = 0, size = covered.size();
	RuleEvaluationResult rer_ge(P, 0, N, 0);
	RuleEvaluationResult rer_lt(P, 0, N, 0);
	//<value of an attribute, number of example>
	//plus: <value of an attribute from specified conditions, index of the condition>
	multimap<double, int> values;
	double attValue, prevVal = numeric_limits<double>::max(), prevClass = -1, currClass;

	for (int j = 0; j < size; j++)
	{
		attValue = covered[j].getAttribute(attributeIndex);
		if (attValue != attValue) //false if NaN
			continue;

		values.insert(pair<double, int>(attValue, j));

		if(covered[j].getDecisionAttribute() == decClass)
			rer_ge.p += covered[j].getWeight();
		else
			rer_ge.n +=covered[j].getWeight();
	}

	int sizeUnc = uncoveredPositives.size();
	multiset<double> uncPosValues;

	for(int j = 0; j < sizeUnc; j++)
			uncPosValues.insert(uncoveredPositives[j].getAttribute(attributeIndex));

	double min = *(uncPosValues.begin());
	double max = *(uncPosValues.rbegin());

	//specified conditions for this attribute:
	list<KnowledgeCondition> kConditions;
	int kCondIndex = -1;
	for(list<KnowledgeCondition>::iterator it = knowledge->getAllowedConditions()[decClass].getConditions().begin();
			it != knowledge->getAllowedConditions()[decClass].getConditions().end(); it++)
	{
		if(it->getAttributeIndex() == attributeIndex)
		{
			kConditions.push_back(*it);
			values.insert(pair<double, int>(it->getFrom(), kCondIndex));
			values.insert(pair<double, int>(it->getTo(), kCondIndex));
			kCondIndex--;
		}
	}

	multimap<double, int>::iterator val = values.begin();
	prevVal = val->first;
	bool isGreaterEqualAllowed = true;
	bool isLessThanAllowed = true;
	bool additionalValue = false;

	for(;val != values.end();
		currClass == decClass ?	(rer_lt.p += currWeight, rer_ge.p -= currWeight)
							  : (rer_lt.n += currWeight, rer_ge.n -= currWeight), val++)
	{
		additionalValue = val->second < 0;	//the value was added from specified conditions

		if(!additionalValue)
		{
			currClass = covered[val->second].getDecisionAttribute();
			currWeight = covered[val->second].getWeight();

			shouldSkip = currClass == prevClass || prevVal == val->first;

			mean = (prevVal + val->first) / 2;
			prevVal = val->first;
			prevClass = currClass;

			if(shouldSkip)
				continue;
		}
		else
		{
			mean = val->first;
			currWeight = 0;
			currClass = decClass;
		}

		isGreaterEqualAllowed = true;
		isLessThanAllowed = true;

		if(useSpecifiedOnly)
		{
			isGreaterEqualAllowed = isNumericConditionSpecified(mean, true, kConditions);
			isLessThanAllowed = isNumericConditionSpecified(mean, false, kConditions);

			if(!isGreaterEqualAllowed && !isLessThanAllowed)
				continue;
		}

		isGreaterEqualAllowed = isGreaterEqualAllowed && !isNumericConditionForbidden(rule, decClass, attributeIndex, mean, true);
		isLessThanAllowed = isLessThanAllowed && !isNumericConditionForbidden(rule, decClass, attributeIndex, mean, false);

		if(!isGreaterEqualAllowed && !isLessThanAllowed)
			continue;

		quality =  -numeric_limits<double>::max();
		ltQuality = -numeric_limits<double>::max();

		if(!isRqmEntropy)
		{
			if(mean <= max && isGreaterEqualAllowed)
				quality = rqm.EvaluateRuleQualityFromResult(rer_ge);
			if(mean > min && isLessThanAllowed)
				ltQuality = rqm.EvaluateRuleQualityFromResult(rer_lt);
		}
		else
		{
			entrLT_p = getNumberOfValuesLessOrGreater(uncPosValues, mean, true);
			entrGE_p = uncPosValues.size() - entrLT_p;

			if(mean <= max && entrGE_p > entrLT_p && isGreaterEqualAllowed)
				quality = NegConditionalEntropy::ComputeQualityForTwoGroups(rer_lt.p, rer_lt.n, rer_ge.p, rer_ge.n);
			else if(mean > min /*&& entrGE_p <= entrLT_p*/ && isLessThanAllowed)
				ltQuality = NegConditionalEntropy::ComputeQualityForTwoGroups(rer_lt.p, rer_lt.n, rer_ge.p, rer_ge.n);
			else
				continue;
		}

		if (quality < bestQuality && ltQuality < bestQuality) continue;

		if(quality > bestQuality || ltQuality > bestQuality)
		{
			bestQuality = quality > ltQuality ? quality : ltQuality;
			equallyBestConditions.clear();
		}

		if(quality >= ltQuality)
			equallyBestConditions.push_back(ElementaryCondition(attributeIndex, new GreaterEqualOperator(), mean));

		if(quality <= ltQuality)
			equallyBestConditions.push_back(ElementaryCondition(attributeIndex, new LessThanOperator(), mean));

		//cout << "Value: " << mean << "\tltQuality: " << ltQuality << "\tgtQuality: " << quality << endl;
	}
}

bool SequentialCoveringWithPreferences::isNumericConditionSpecified(double value, bool greaterEqual, list<KnowledgeCondition>& conditions, bool andRequired)
{
	bool result = false;

	for(list<KnowledgeCondition>::iterator it = conditions.begin(); it != conditions.end(); it++)
	{
		if(it->isFixed())
		{
			if((greaterEqual && value == it->getFrom()) || (!greaterEqual && value == it->getTo()))
			{
				result = andRequired ? it->isRequired() : true;
				if(result)
					break;
			}
		}
		else
		{//check if interception is not empty && condition is equal or more restrictive
			if((greaterEqual && value >= it->getFrom() && value < it->getTo())
				|| (!greaterEqual && value > it->getFrom() && value <= it->getTo()))
			{
				result = andRequired ? it->isRequired() : true;
				if(result)
					break;
			}
		}
	}

	return result;
}

bool SequentialCoveringWithPreferences::isNumericConditionForbidden(Rule& rule, double decClass, int attributeIndex, double value, bool greaterEqual)
{
	list<KnowledgeCondition> conditions = knowledge->getForbiddenConditions()[decClass].getConditionsForAttribute(attributeIndex);
	bool isForbidden = false;

	for(list<KnowledgeCondition>::iterator it = conditions.begin(); it != conditions.end(); it++)
		if(isConitionsInterceptionNotEmpty(decClass, attributeIndex, value, greaterEqual, *it))
		{
			isForbidden = true;
			break;
		}

	if(!isForbidden)
		isForbidden = isConditionForbiddenInRule(rule, decClass, attributeIndex, value, greaterEqual, false);

	return isForbidden;
}

bool SequentialCoveringWithPreferences::isConitionsInterceptionNotEmpty(double decClass, int attributeIndex, double value, bool greaterEqual, KnowledgeCondition& cond)
{
	bool result = false;
	if(cond.isFixed())
	{
		if((greaterEqual && value == cond.getFrom()) || (!greaterEqual && value == cond.getTo()))
		{
			result = true;
		}
	}
	else
	{
		if((greaterEqual && value < cond.getTo()) || (!greaterEqual && value > cond.getFrom()))
		{
			result = true;
		}
	}
	return result;
}

void SequentialCoveringWithPreferences::findBestConditionForNominalAttribute
(Rule& rule, double decClass, SetOfExamples& covered, SetOfExamples& uncoveredPositives,
		RuleQualityMeasure& rqm, bool isRqmEntropy, int attributeIndex, list<ElementaryCondition>& equallyBestConditions, double& bestQuality, bool useSpecifiedOnly)
{
	map<double, RuleEvaluationResult> values;
	int size = covered.size();
	double attValue, p = 0, n = 0, quality;

	for (int j = 0; j < size; j++)
	{
		attValue = covered[j].getAttribute(attributeIndex);
		if (attValue != attValue) continue; //true if NaN

		if(covered[j].getDecisionAttribute() == decClass)
			values[attValue].p += covered[j].getWeight();
		else
			values[attValue].n += covered[j].getWeight();
	}

	for(auto val = values.begin(); val != values.end(); val++)
	{
		p += val->second.p;
		n += val->second.n;
	}

	if(useSpecifiedOnly)
	{
		for(list<KnowledgeCondition>::iterator kCond = knowledge->getAllowedConditions()[decClass].getConditions().begin();
				kCond != knowledge->getAllowedConditions()[decClass].getConditions().end(); kCond++)
		{
			if(kCond->getAttributeIndex() == attributeIndex && (isNominalConditionForbidden(rule, decClass, attributeIndex, kCond->getValue()) ||
				!existsExampleWithEqualAttValue(attributeIndex, kCond->getValue(), uncoveredPositives)))
						continue;

			RuleEvaluationResult rer = values[kCond->getValue()];

			if(!isRqmEntropy)
			{
				rer.P = P; rer.N = N;
				quality = rqm.EvaluateRuleQualityFromResult(rer);
			}
			else
			{
				quality = NegConditionalEntropy::ComputeQualityForTwoGroups(rer.p, rer.n,
																			p - rer.p, n - rer.n);
			}

			if (quality < bestQuality) continue;

			if(quality > bestQuality)
			{
				bestQuality = quality;
				equallyBestConditions.clear();
			}

			equallyBestConditions.push_back(ElementaryCondition(attributeIndex, new EqualityOperator(), kCond->getValue()));
		}
	}
	else
	{
		for(auto val = values.begin(); val != values.end(); val++)
		{
			if(isNominalConditionForbidden(rule, decClass, attributeIndex, val->first) ||
					!existsExampleWithEqualAttValue(attributeIndex, val->first, uncoveredPositives))
				continue;

			if(!isRqmEntropy)
			{
				val->second.P = P; val->second.N = N;
				quality = rqm.EvaluateRuleQualityFromResult(val->second);
			}
			else
			{
				quality = NegConditionalEntropy::ComputeQualityForTwoGroups(val->second.p, val->second.n,
																			p - val->second.p, n - val->second.n);
			}

			if (quality < bestQuality) continue;

			if(quality > bestQuality)
			{
				bestQuality = quality;
				equallyBestConditions.clear();
			}

			equallyBestConditions.push_back(ElementaryCondition(attributeIndex, new EqualityOperator(), val->first));
		}
	}
}

bool SequentialCoveringWithPreferences::isNominalConditionForbidden(Rule& rule, double decClass, int attributeIndex, double value)
{
	bool isForbidden = false;
	for(list<KnowledgeCondition>::iterator kCond = knowledge->getForbiddenConditions()[decClass].getConditions().begin();
			kCond != knowledge->getForbiddenConditions()[decClass].getConditions().end(); kCond++)
	{
		if(kCond->getAttributeIndex() == attributeIndex && kCond->getValue() == value)
		{
			isForbidden = true;
			break;
		}
	}

	if(!isForbidden)
		isForbidden = isConditionForbiddenInRule(rule, decClass, attributeIndex, value, false, true);

	return isForbidden;
}

int SequentialCoveringWithPreferences::getNumberOfValuesLessOrGreater(multiset<double>& values, double value, bool takeLess)
{
	int cnt = 0;
	for(auto it = values.begin(); it != values.end() && *it < value; it++, cnt++);

	return takeLess ? cnt : values.size() - cnt;
}

/**
 * Chooses one elementary condition from conditions with the same value of rule quality measure
 * @param equallyBestConditions list of the equally best conditions
 * @param decClass positive class value
 * @param uncoveredPositives set of examples not covered by current set of rules but covered by the current rule
 * @return chosen condition
 */
ElementaryCondition SequentialCoveringWithPreferences::chooseConditionFromEqual(list<ElementaryCondition>& equallyBestConditions, double decClass, SetOfExamples& uncoveredPositives)
{
    vector<ElementaryCondition> equallyCoveringConds;
    list<ElementaryCondition>::iterator bestCondIt;
    double pMax = 0;
    RuleEvaluationResult rer;
    for (bestCondIt = equallyBestConditions.begin(); bestCondIt != equallyBestConditions.end(); bestCondIt++)
    {
        rer = RuleQualityMeasure::EvaluateCondition(uncoveredPositives, *bestCondIt, decClass);
        if (rer.p > pMax)
        {
            equallyCoveringConds.clear();
            equallyCoveringConds.push_back(*bestCondIt);
            pMax = rer.p;
        }
        else if (rer.p == pMax)
            equallyCoveringConds.push_back(*bestCondIt);
    }
    return equallyCoveringConds[rand() % equallyCoveringConds.size()]; //works also for one condition
}

/**
 * Pruning the rule
 * @param rule rule to be pruned
 * @param examples training set
 * @param ruleQualityMeasure rule quality measure
 */
void SequentialCoveringWithPreferences::pruneRule(Rule& rule, SetOfExamples& examples, RuleQualityMeasure& ruleQualityMeasure)
{
    vector<list<ElementaryCondition> >::iterator itVec;
    list<ElementaryCondition>::iterator itList;
    double bestQuality = ruleQualityMeasure.EvaluateRuleQuality(examples, rule);
    double currentQuality;
    vector<ElementaryCondition> equallyWorstConds;
    Rule tempRule;
    ElementaryCondition* conditionToRemove;
    unsigned int numberOfAtts = rule.getConditions().size();
    bool isRequired = false;
    while (1)
    {
        equallyWorstConds.clear();
        tempRule = rule;

        for (unsigned int i = 0; i < numberOfAtts; i++)
        	for (itList = rule.getConditions()[i].begin(); itList != rule.getConditions()[i].end(); itList++)
			{
                Attribute::AttributeType attributeType = examples.getAttributeType(i);
                switch (attributeType)
                {
        			case Attribute::NUMERICAL:
        				isRequired = isNumericConditionRequired(*itList, rule);
        				break;
        			case Attribute::NOMINAL:
        				isRequired = isNominalConditionRequired(*itList, rule);
        				break;
                }

                if(isRequired)	//can't remove condition if it is required
                	continue;

				tempRule.removeCondition(*itList);
				currentQuality = ruleQualityMeasure.EvaluateRuleQuality(examples, tempRule);
				if (currentQuality > bestQuality)
				{
					bestQuality = currentQuality;
					equallyWorstConds.clear();
					equallyWorstConds.push_back(*itList);
				}
				else if((currentQuality == bestQuality) || (currentQuality != currentQuality && bestQuality != bestQuality))	//equal or both are NaN
					equallyWorstConds.push_back(*itList);
				tempRule.addCondition(*itList);
			}

        if (equallyWorstConds.empty())
            break;

        conditionToRemove = &equallyWorstConds[rand() % equallyWorstConds.size()];
        rule.removeCondition(*conditionToRemove);
        //cout << "Removed condition: " << conditionToRemove->toString(examples.getDataSet()) << "\tBestQuality: " << bestQuality << endl;
    }
}

bool SequentialCoveringWithPreferences::isNominalConditionRequired(ElementaryCondition& condition, Rule& rule)
{
	double decClass = rule.getDecisionClass();
	int attributeIndex = condition.getAttributeIndex();
	double value = condition.getAttributeValue();

	bool result = false;
	for(list<KnowledgeCondition>::iterator kCond = knowledge->getAllowedConditions()[decClass].getConditions().begin();
			kCond != knowledge->getAllowedConditions()[decClass].getConditions().end(); kCond++)
	{
		if(kCond->isRequired() && kCond->getAttributeIndex() == attributeIndex && kCond->getValue() == value)
		{
			result = true;
			break;
		}
	}

	return result;
}

bool SequentialCoveringWithPreferences::isNumericConditionRequired(ElementaryCondition& condition, Rule& rule)
{
	int attributeIndex = condition.getAttributeIndex();
	double value = condition.getAttributeValue();

	list<KnowledgeCondition> conditions = knowledge->getAllowedConditions()[rule.getDecisionClass()].getConditionsForAttribute(attributeIndex);


	bool result = false;
	bool greaterEqual = typeid(*condition.getOperator()) == typeid(GreaterEqualOperator);

	if(isNumericConditionSpecified(value, greaterEqual, conditions, true))
	{
		result = true;
		//checking if other required condition with the same "direction" exists for this attribute; if yes, the main condition can be removed
		if(rule.getConditions()[attributeIndex].size() > 1)
		{
			for(list<ElementaryCondition>::iterator it = rule.getConditions()[attributeIndex].begin(); it != rule.getConditions()[attributeIndex].end(); it++)
				if(*it != condition && 	//not the same condition
					(typeid(*it->getOperator()) == typeid(*condition.getOperator())) && //the same operator
					isNumericConditionSpecified(it->getAttributeValue(), greaterEqual, conditions, true))	//is specified and required
				{
					result = false;
					break;
				}
		}
	}

	return result;
}

/**
 * Returns set of examples covered by the rule
 * @param rule decision rule
 * @param examples set of examples
 * @return set of examples covered by the rule
 */
SetOfExamples SequentialCoveringWithPreferences::getCoveredExamples(Rule& rule, SetOfExamples& examples)
{
    SetOfExamples covered(examples.getDataSet());
    for (int i = 0; i < examples.size(); i++)
    {
        if (rule.covers(examples[i]))
            covered.addExample(examples, i);
    }
    return covered;
}

/**
 * Returns set of examples covered by the condition
 * @param cond condition
 * @param examples set of examples
 * @return set of examples covered by the condition
 */
SetOfExamples SequentialCoveringWithPreferences::getCoveredExamples(ElementaryCondition& cond, SetOfExamples& examples)
{
    SetOfExamples covered(examples.getDataSet());
    int attIndex = cond.getAttributeIndex();
    for (int i = 0; i < examples.size(); i++)
    {
        if (cond.isSatisfied(examples[i].getAttribute(attIndex)))
            covered.addExample(examples, i);
    }
    return covered;
}

/**
 * Checks if the set contains at least one instance covered by the rule.
 * @param rule decision rule
 * @param examples set of examples
 * @return true or false
 */
bool SequentialCoveringWithPreferences::existsCoveredExample(Rule& rule, SetOfExamples& examples)
{
    for (int i = 0; i < examples.size(); i++)
    {
        if (rule.covers(examples[i]))
            return true;
    }
    return false;
}

/**
 * Checks if the set contains at least one instance covered by the elementary condition
 * @param condition elementary condition
 * @param examples set of examples
 * @return true or false
 */
bool SequentialCoveringWithPreferences::existsCoveredExample(ElementaryCondition& condition,SetOfExamples& examples)
{
    int attIndex = condition.getAttributeIndex();
    for (int i = 0; i < examples.size(); i++)
    {
        if(condition.isSatisfied(examples[i].getAttribute(attIndex)))
            return true;
    }
    return false;
}

/**
 * Check if the set of examples contains at least one example with certain value of the attribute
 * @param attIndex attribute index
 * @param attValue attribute value
 * @param examples set of examples
 * @return true or false
 */
bool SequentialCoveringWithPreferences::existsExampleWithEqualAttValue(int attIndex, double attValue, SetOfExamples& examples)
{
    int size = examples.size();
    for (int i = 0; i < size; i++)
        if(attValue == examples[i].getAttribute(attIndex))
            return true;
    return false;
}


Rule* SequentialCoveringWithPreferences::getRuleFromKnowledgeRule(KnowledgeRule& kRule)
{
	Rule* rule = new Rule();
	SetOfConditions& kConds = kRule.getConditions();

	rule->setDecisionClass(kConds.getDecisionClass());

	for(list<KnowledgeCondition>::iterator it = kRule.getConditions().getConditions().begin();
			it != kRule.getConditions().getConditions().end(); it++)
	{
		if(it->getValue() == it->getValue())
		{
			ElementaryCondition newCondition(it->getAttributeIndex(), new EqualityOperator(), it->getValue());
			rule->addCondition(newCondition);
		}
		else
		{
			if(it->getFrom() > -numeric_limits<double>::max())
			{
				ElementaryCondition newCondition(it->getAttributeIndex(), new GreaterEqualOperator(), it->getFrom());
				rule->addCondition(newCondition);
			}

			if(it->getTo() < numeric_limits<double>::max())
			{
				ElementaryCondition newCondition(it->getAttributeIndex(), new LessThanOperator(), it->getTo());
				rule->addCondition(newCondition);
			}
		}
	}
	return rule;
}






bool SequentialCoveringWithPreferences::isConditionForbiddenInRule(Rule& rule, double decClass, int attributeIndex, double value, bool greaterEqual, bool nominal)
{
	bool isForbidden = false;

	//for each forbidden rule
	for(list<KnowledgeRule>::iterator itForbRule = knowledge->getForbiddenRules()[decClass].begin(); !isForbidden && itForbRule != knowledge->getForbiddenRules()[decClass].end(); itForbRule++)
	{
		//check if current condition is present in forbidden rule
		list<KnowledgeCondition>::iterator itForbCond;
		for(itForbCond = itForbRule->getConditions().getConditions().begin(); itForbCond != itForbRule->getConditions().getConditions().end(); itForbCond++)
		{
			if(itForbCond->getAttributeIndex() == attributeIndex)
			{
				if(nominal)
				{
					if(itForbCond->getValue() == value)
						break;
				}
				else
				{
					if(isConitionsInterceptionNotEmpty(decClass, attributeIndex, value, greaterEqual, *itForbCond))
						break;
				}
			}
		}
		//if current condition is present in "itForbRule" forbidden rule, check if all other conditions from this forbidden rule are in a current rule
		if(itForbCond != itForbRule->getConditions().getConditions().end())
		{
			list<KnowledgeCondition>::iterator itForbCondPrime;
			for(itForbCondPrime = itForbRule->getConditions().getConditions().begin(); itForbCondPrime != itForbRule->getConditions().getConditions().end(); itForbCondPrime++)
			{
				if(itForbCondPrime == itForbCond)
					continue;

				//check if "itCondPrime" condition from forbidden rule is present in current rule
				bool isConditionPresent = false;

				if(itForbCondPrime->getValue() == itForbCondPrime->getValue())	//if nominal attribute
				{
					for(list<ElementaryCondition>::iterator itElCond = rule.getConditions()[itForbCondPrime->getAttributeIndex()].begin(); itElCond != rule.getConditions()[itForbCondPrime->getAttributeIndex()].end(); itElCond++)
					{
						if(itForbCondPrime->getValue() == itElCond->getAttributeValue())
						{
							isConditionPresent = true;
							break;
						}
					}
				}
				else if(rule.getConditions().size() > itForbCondPrime->getAttributeIndex())
				{
					for(list<ElementaryCondition>::iterator itElCond = rule.getConditions()[itForbCondPrime->getAttributeIndex()].begin(); itElCond != rule.getConditions()[itForbCondPrime->getAttributeIndex()].end(); itElCond++)
					{
						if(isConitionsInterceptionNotEmpty(decClass, itElCond->getAttributeIndex(), itElCond->getAttributeValue(), typeid(*itElCond->getOperator()) == typeid(GreaterEqualOperator), *itForbCondPrime))
						{
							isConditionPresent = true;
							break;
						}
					}
				}

				//if not, adding condition to the rule will not create forbidden conjunction according to "itRule"
				if(!isConditionPresent)
					break;
			}

			if(itForbCondPrime == itForbRule->getConditions().getConditions().end())
				isForbidden = true;
		}
	}

	return isForbidden;
}
