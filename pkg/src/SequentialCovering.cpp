#include "SequentialCovering.h"

using namespace std;

/**
 * Generates list of rules for every decision class
 * @param examples training set on which the induction is based
 * @param rqmGrow rule qualisty measure used in growth phase
 * @param rqmPrune rule qualisty measure used in pruning phase
 * @return ist of rules for every decision class
 */
list<Rule> SequentialCovering::generateRules(SetOfExamples& examples, RuleQualityMeasure& rqmGrow, RuleQualityMeasure& rqmPrune)
{
    list<Rule> ruleSet;
    vector<double> classes = examples.getDistinctClasses();
    vector<double>::iterator it;
    for(it = classes.begin(); it != classes.end(); it++)
    {
    	//KNOW: if(knowledge[*it].size() == 0 && knowledge.generateRulesForOtherClasses)
    	//KNOW:	continue;
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
list<Rule> SequentialCovering::generateRulesForClass(SetOfExamples& examples, RuleQualityMeasure& rqmGrow, RuleQualityMeasure& rqmPrune, double decClass)
{
    list<Rule> ruleSet;
    SetOfExamples uncoveredPositives(examples.getExamplesForDecAtt(decClass));
    //P = uncoveredPositives.size();
    P = uncoveredPositives.getSumOfWeights();
    //N = examples.size() - P;
    N = examples.getSumOfWeights() - P;
    double apriori = P / (P + N);
    Precision precision;
    vector<list<ElementaryCondition> >::iterator itVec;

//	KNOW: foreach(rule in knowledge[decClass].rules)
//	{
//		SetOfExamples covered(examples);
//		growSpecifiedRule(rule, covered, uncoveredPositives, rqmGrow, knowlegde);
//		pruneSPecifiedRule(rules, examples, rqmPrune, knowledge);
//        uncoveredPositives = uncoveredPositives - covered;
//        rule.setConfidenceDegree(rqmPrune.EvaluateRuleQuality(examples, rule));
//        ruleSet.push_back(rule);
//	}

//    if(!useSpecifiedOnly)
//    {

    while (uncoveredPositives.size() != 0)
    {
        SetOfExamples covered(examples);
        Rule rule;
        rule.setDecisionClass(decClass);
        growRule(rule, covered, uncoveredPositives, rqmGrow);
        pruneRule(rule, examples, rqmPrune);
        //cout << rule.toString((covered.getDataSet())) << endl;

        covered = getCoveredExamples(rule, examples);

        if(precision.EvaluateRuleQuality(covered, rule) <= apriori)
        {
        	//cout << "precision < apriori" << endl;
        	//cout << "apriori:  " << apriori << "\tcurrent: " << precision.EvaluateRuleQuality(covered, rule) << endl;
            break;
        }

        uncoveredPositives = uncoveredPositives - covered;
        rule.setConfidenceDegree(rqmPrune.EvaluateRuleQuality(examples, rule));
        ruleSet.push_back(rule);


        //cout << rule.toString(examples.getDataSet()) << endl;
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
void SequentialCovering::growRule(Rule& rule, SetOfExamples& covered, SetOfExamples uncoveredPositives, RuleQualityMeasure& ruleQualityMeasure)
{
    Precision prec;
    ElementaryCondition bestCondition;
    double decClass = rule.getDecisionClass();
    RuleEvaluationResult rer;
    double coveredCount = 0, prevCoveredCount = 0;
    bool isEntropy = typeid (ruleQualityMeasure) == typeid (NegConditionalEntropy);

    while (1)
    {
        bestCondition = findBestCondition(decClass, covered, uncoveredPositives, ruleQualityMeasure, isEntropy);
        if (bestCondition.getAttributeIndex() == -1)
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

//KNOW:
//void CheckCondition(checkedCondition, Rule currentRule, forbiddenRules, allowedConditions, forbiddenConditions)
//{
//
//}


/**
 * Finds the best elementary condition from all possible ones
 * @param decClass number of positive class
 * @param covered set of examples covered by current rule
 * @param uncoveredPositives set of examples not covered by current set of rules but covered by the current rule
 * @param rqm rule quality measure
 * @return the best elementary condition
 */
ElementaryCondition SequentialCovering::findBestCondition
(double decClass, SetOfExamples& covered, SetOfExamples& uncoveredPositives, RuleQualityMeasure& rqm, bool isRqmEntropy)
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
				findBestConditionForNumericalAttribute(decClass, covered, uncoveredPositives, rqm, isRqmEntropy, i, equallyBestConditions, bestQuality);
				break;
			case Attribute::NOMINAL:
				findBestConditionForNominalAttribute(decClass, covered, uncoveredPositives, rqm, isRqmEntropy, i, equallyBestConditions, bestQuality);
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

void SequentialCovering::findBestConditionForNumericalAttribute
(double decClass, SetOfExamples& covered, SetOfExamples& uncoveredPositives,
		RuleQualityMeasure& rqm, bool isRqmEntropy, int attributeIndex, list<ElementaryCondition>& equallyBestConditions, double& bestQuality)
{
	double mean, quality, ltQuality, currWeight;
	ElementaryCondition bestCondition;
	bool shouldSkip = false;
	int entrLT_p = 0, entrGE_p = 0, size = covered.size();
	RuleEvaluationResult rer_ge(P, 0, N, 0);
	RuleEvaluationResult rer_lt(P, 0, N, 0);
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

	//cout << "Size " << values.size() << "\tMin: " << min << "\tMax: " << max << endl;

	multimap<double, int>::iterator val = values.begin();
	prevVal = val->first;

	for(;val != values.end();
		currClass == decClass ?	(rer_lt.p += currWeight, rer_ge.p -= currWeight)
							  : (rer_lt.n += currWeight, rer_ge.n -= currWeight), val++)
	{
		currClass = covered[val->second].getDecisionAttribute();
		currWeight = covered[val->second].getWeight();

		shouldSkip = currClass == prevClass || prevVal == val->first;

		mean = (prevVal + val->first) / 2;
		prevVal = val->first;
		prevClass = currClass;

		if(shouldSkip)
			continue;

		quality =  -numeric_limits<double>::max();
		ltQuality = -numeric_limits<double>::max();

		if(!isRqmEntropy)
		{
			if(mean <= max)
				quality = rqm.EvaluateRuleQualityFromResult(rer_ge);
			if(mean > min)
				ltQuality = rqm.EvaluateRuleQualityFromResult(rer_lt);
		}
		else
		{
			entrLT_p = getNumberOfValuesLessOrGreater(uncPosValues, mean, true);
			entrGE_p = uncPosValues.size() - entrLT_p;

			if(mean <= max && entrGE_p > entrLT_p)
				quality = NegConditionalEntropy::ComputeQualityForTwoGroups(rer_lt.p, rer_lt.n, rer_ge.p, rer_ge.n);
			else if(mean > min /*&& entrGE_p <= entrLT_p*/)
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

void SequentialCovering::findBestConditionForNominalAttribute
(double decClass, SetOfExamples& covered, SetOfExamples& uncoveredPositives,
		RuleQualityMeasure& rqm, bool isRqmEntropy, int attributeIndex, list<ElementaryCondition>& equallyBestConditions, double& bestQuality)
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

	for(auto val = values.begin(); val != values.end(); val++)
	{
		if(!existsExampleWithEqualAttValue(attributeIndex, val->first, uncoveredPositives))
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

int SequentialCovering::getNumberOfValuesLessOrGreater(multiset<double>& values, double value, bool takeLess)
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
ElementaryCondition SequentialCovering::chooseConditionFromEqual(list<ElementaryCondition>& equallyBestConditions, double decClass, SetOfExamples& uncoveredPositives)
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
void SequentialCovering::pruneRule(Rule& rule, SetOfExamples& examples, RuleQualityMeasure& ruleQualityMeasure)
{
    vector<list<ElementaryCondition> >::iterator itVec;
    list<ElementaryCondition>::iterator itList;
    double bestQuality = ruleQualityMeasure.EvaluateRuleQuality(examples, rule);
    double currentQuality;
    vector<ElementaryCondition> equallyWorstConds;
    Rule tempRule;
    ElementaryCondition* conditionToRemove;
    while (1)
    {
        equallyWorstConds.clear();
        tempRule = rule;
        for (itVec = rule.getConditions().begin(); itVec != rule.getConditions().end(); itVec++)
            for (itList = itVec->begin(); itList != itVec->end(); itList++)
            {
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

/**
 * Returns set of examples covered by the rule
 * @param rule decision rule
 * @param examples set of examples
 * @return set of examples covered by the rule
 */
SetOfExamples SequentialCovering::getCoveredExamples(Rule& rule, SetOfExamples& examples)
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
SetOfExamples SequentialCovering::getCoveredExamples(ElementaryCondition& cond, SetOfExamples& examples)
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
bool SequentialCovering::existsCoveredExample(Rule& rule, SetOfExamples& examples)
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
bool SequentialCovering::existsCoveredExample(ElementaryCondition& condition,SetOfExamples& examples)
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
bool SequentialCovering::existsExampleWithEqualAttValue(int attIndex, double attValue, SetOfExamples& examples)
{
    int size = examples.size();
    for (int i = 0; i < size; i++)
        if(attValue == examples[i].getAttribute(attIndex))
            return true;
    return false;
}
