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
    while (uncoveredPositives.size() != 0)
    {
        SetOfExamples covered(examples);
        Rule rule;
        rule.setDecisionClass(decClass);
        growRule(rule, covered, uncoveredPositives, rqmGrow);
        if(precision.EvaluateRuleQuality(covered, rule) <= apriori)
            break;
        pruneRule(rule, examples, rqmPrune);
        rule.setConfidenceDegree(rqmPrune.EvaluateRuleQuality(examples, rule));
        ruleSet.push_back(rule);
        covered = getCoveredExamples(rule, examples);
        uncoveredPositives = uncoveredPositives - covered;
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
        if(isEntropy)
            bestCondition = findBestConditionUsingEntropy(decClass, covered, uncoveredPositives, ruleQualityMeasure);
        else
            bestCondition = findBestCondition(decClass, covered, uncoveredPositives, ruleQualityMeasure);
        if (bestCondition.getAttributeIndex() == -1)
            break;
        //checking stop criterion
        //currentPrecision = prec.EvaluateConditionQuality(covered, bestCondition, decClass);
        rer = RuleQualityMeasure::EvaluateCondition(covered, bestCondition, decClass);
        if (rer.n == 0)
        {
            rule.addConditionAndOptimize(bestCondition);
            break;
        }
        coveredCount = rer.p + rer.n;
        if(coveredCount == prevCoveredCount)
            break;
        covered = getCoveredExamples(bestCondition, covered);
        uncoveredPositives = getCoveredExamples(bestCondition, uncoveredPositives);
        prevCoveredCount = coveredCount;
        rule.addConditionAndOptimize(bestCondition);
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
ElementaryCondition SequentialCovering::findBestCondition
(double decClass, SetOfExamples& covered, SetOfExamples& uncoveredPositives, RuleQualityMeasure& rqm)
{
    vector<list<ElementaryCondition> >::iterator itVec;
    list<ElementaryCondition>::iterator it;
    double quality, bestQuality = -numeric_limits<double>::max();
    ElementaryCondition bestCondition;
    list<ElementaryCondition> equallyBestConditions;
    vector<list<ElementaryCondition> > elemCond;
    generateElementaryConditions(elemCond, decClass, covered, uncoveredPositives, false);

    //finding the best el. cond.
    for(itVec = elemCond.begin(); itVec != elemCond.end(); itVec++)
    {
        for (it = itVec->begin(); it != itVec->end(); it++)
        {
            RuleEvaluationResult rer = RuleQualityMeasure::EvaluateCondition(covered, *it, decClass);
            rer.N = N;
            rer.P = P;
            quality = rqm.EvaluateRuleQualityFromResult(rer);
            if (quality > bestQuality)
            {
                bestQuality = quality;
                equallyBestConditions.clear();
                equallyBestConditions.push_back(*it);
            }
            else if (quality == bestQuality)
                equallyBestConditions.push_back(*it);
        }
    }

    //when more than one condition is the best
    if (equallyBestConditions.size() == 1)
        bestCondition = equallyBestConditions.front();
    else if (equallyBestConditions.size() > 1)
        bestCondition = chooseConditionFromEqual(equallyBestConditions, decClass, uncoveredPositives);
    return bestCondition;
}

/**
 * Finds the best elementary condition from all possible ones using negated conditional entropy
 * @param decClass number of positive class
 * @param covered set of examples covered by current rule
 * @param uncoveredPositives set of examples not covered by current set of rules but covered by the current rule
 * @param rqm rule quality measure
 * @return the best elementary condition
 */
ElementaryCondition SequentialCovering::findBestConditionUsingEntropy
(double decClass, SetOfExamples& covered, SetOfExamples& uncoveredPositives, RuleQualityMeasure& rqm)
{
    vector<list<ElementaryCondition> >::iterator itVec;
    list<ElementaryCondition>::iterator it;
    double quality, bestQuality = -numeric_limits<double>::max();
    ElementaryCondition bestCondition;
    bestCondition.setAttributeIndex(-1);
    list<ElementaryCondition> equallyBestConditions;
    vector<list<ElementaryCondition> > elemCond;
    generateElementaryConditions(elemCond, decClass, covered, uncoveredPositives, true);
    //finding the best el. cond.
    for(itVec = elemCond.begin(); itVec != elemCond.end(); itVec++)
    {
        for (it = itVec->begin(); it != itVec->end(); it++)
        {
            quality = rqm.EvaluateConditionQuality(covered, *it, decClass);
            if (quality > bestQuality)
            {
                bestQuality = quality;
                equallyBestConditions.clear();
            }
            if(quality == bestQuality)  //meaning quality >= bestQuality
                equallyBestConditions.push_back(*it);
        }
    }
    //equally best conditions
    if (equallyBestConditions.size() == 1)
        bestCondition = equallyBestConditions.front();
    else if (equallyBestConditions.size() > 1)
        bestCondition = chooseConditionFromEqual(equallyBestConditions, decClass, uncoveredPositives);
    return bestCondition;
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
//        rule.addCondition(*bestCondIt);
//        rer = RuleQualityMeasure::EvaluateRule(uncoveredPositives, rule);
//        rule.removeCondition(*bestCondIt);
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
    double apriori = P / (P + N);
    Precision precision;
    while (1)
    {
        equallyWorstConds.clear();
        tempRule = rule;
        for (itVec = rule.getConditions().begin(); itVec != rule.getConditions().end(); itVec++)
            for (itList = itVec->begin(); itList != itVec->end(); itList++)
            {
                tempRule.removeCondition(*itList);
                currentQuality = ruleQualityMeasure.EvaluateRuleQuality(examples, tempRule);
                if ((currentQuality > bestQuality) && (precision.EvaluateRuleQuality(examples, tempRule) > apriori))
                {
                    bestQuality = currentQuality;
                    equallyWorstConds.clear();
                    equallyWorstConds.push_back(*itList);
                }
                else if((currentQuality == bestQuality) && (precision.EvaluateRuleQuality(examples, tempRule) > apriori))
                    equallyWorstConds.push_back(*itList);
                tempRule.addCondition(*itList);
            }
        if (equallyWorstConds.empty())
            break;
        rule.removeCondition(equallyWorstConds[rand() % equallyWorstConds.size()]);
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

/**
 * Generates all possible elementary conditions for given arguments
 * @param conditions vector of lists of elementary conditions
 * index of list corresponds to the index of attribute in data set
 * @param decClass positive class index
 * @param covered set of examples covered by current rule
 * @param uncoveredPositives set of examples not covered by current set of rules but covered by the current rule
 * @param logical value that indicates whether entropy measure is used
 */
void SequentialCovering::generateElementaryConditions(vector<list<ElementaryCondition> >& conditions, double decClass, SetOfExamples& covered, SetOfExamples& uncoveredPositives, bool entropy)
{
    if (covered.size() == 0)
        return;
    int numberOfAtts = covered[0].getAttributes().size();
    conditions.resize(numberOfAtts);

    for (int i = 0; i < numberOfAtts; i++)
    {
        //pobranie i-tego atrybutu dla pierwszego przyk�adu z listy (w celu poznania jego typu)
        Attribute::AttributeType attributeType = covered.getAttributeType(i);
        switch (attributeType)
        {
        case Attribute::NUMERICAL:
            if (entropy)
                generateECForNumericalAttForEntropy(conditions[i], decClass, covered, uncoveredPositives, i);
            else
                generateECForNumericalAtt(conditions[i], covered, uncoveredPositives, i);
            break;
        case Attribute::NOMINAL:
            generateECForNominalAtt(conditions[i], covered, uncoveredPositives, i);
            break;
        }
    }
}

/**
 * Generates elementary conditions for the numerical attribute
 * @param conds list of generated conditions
 * @param covered set of examples covered by current rule
 * @param uncoveredPositives set of examples not covered by current set of rules but covered by the current rule
 * @param attIndex attribute index
 */
void SequentialCovering::generateECForNumericalAtt(list<ElementaryCondition>& conds, SetOfExamples& covered, SetOfExamples& uncoveredPositives, int attIndex)
{
    //obliczenie minimalnej i maksymalnej wartości w zbiorze
    double min = numeric_limits<double>::max(), max = numeric_limits<double>::min();
    int size = uncoveredPositives.size();
    double value;
    if(size > 0)
        min = max = uncoveredPositives[0].getAttribute(attIndex);
    for(int i = 1; i < size; i++)
    {
        value = uncoveredPositives[i].getAttribute(attIndex);
        if(value > max)
            max = value;
        else if(value < min)
            min = value;
    }

    set<double> distinctValues;
    getDistinctSortedAttributeValues(distinctValues, covered, attIndex);
    set<double>::iterator it;
    double prev, mean;

    it = distinctValues.begin();
    if (it != distinctValues.end())
    {
        prev = *it;
        it++;
    }
    for (; it != distinctValues.end(); it++)
    {
        mean = ((*it) + prev) / 2;
        if(min < mean)
            conds.push_back(ElementaryCondition(attIndex, new LessThanOperator(), mean));
        if(max >= mean)
            conds.push_back(ElementaryCondition(attIndex, new GreaterEqualOperator(), mean));
        prev = *it;
    }
}

/**
 * Generates elementary conditions for the nominal attribute
 * @param conds list of generated conditions
 * @param covered set of examples covered by current rule
 * @param uncoveredPositives set of examples not covered by current set of rules but covered by the current rule
 * @param attIndex attribute index
 */
void SequentialCovering::generateECForNominalAtt(list<ElementaryCondition>& conds, SetOfExamples& covered, SetOfExamples& uncoveredPositives, int attIndex)
{
    set<double> distinctValues;
    getDistinctSortedAttributeValues(distinctValues, covered, attIndex);
    set<double>::iterator it;
    for (it = distinctValues.begin(); it != distinctValues.end(); it++)
    {
        if(existsExampleWithEqualAttValue(attIndex, *it, uncoveredPositives))
            conds.push_back(ElementaryCondition(attIndex, new EqualityOperator(), *it));
    }
}

/**
 * Generates set of distinct values of the attribute
 * @param values set to be filled with distinct values
 * @param examples set of examples
 * @param attIndex attribute index
 */
void SequentialCovering::getDistinctSortedAttributeValues(set<double>& values, SetOfExamples& examples, int attIndex)
{
    int size = examples.size();
    double attValue;
    for (int i = 0; i < size; i++)
    {
        attValue = examples[i].getAttribute(attIndex);
        if (attValue == attValue) //false if NaN
            values.insert(attValue);
    }
}

/**
 * Metoda generuje warunki elementarne dla określonego atrybutu numerycznego w przypadku stosowania metody opartej o miarę entropii warunkowej.
 * @param decClass numer aktualnie rozpatrywanej klasy decyzyjnej.
 * @param conds lista, która powinna zostać wypełniona utworzonymi warunkami elementarnymi.
 * @param covered zbiór przykładów pokrywanych przez aktualnie budowaną regułę.
 * @param uncoveredPositives zbiór przykładów niepokrytych przez dotychczas utworzone reguły ale pokrywany przez obecną regułę.
 * @param attIndex indeks atrybutu w zbiorze danych.
 */

/**
 * Generates elementary conditions for the numerical attribute when entropy measure is used
 * @param conds list of generated conditions
 * @param decClass index of positive class
 * @param covered set of examples covered by current rule
 * @param uncoveredPositives set of examples not covered by current set of rules but covered by the current rule
 * @param attIndex attribute index
 */
void SequentialCovering::generateECForNumericalAttForEntropy(list<ElementaryCondition>& conds, double decClass, SetOfExamples& covered, SetOfExamples& uncoveredPositives, int attIndex)
{
    //obliczenie minimalnej i maksymalnej wartości w zbiorze
    double min = numeric_limits<double>::max(), max = numeric_limits<double>::min();
    int size = uncoveredPositives.size();
    double value;
    if(size > 0)
        min = max = uncoveredPositives[0].getAttribute(attIndex);
    for(int i = 1; i < size; i++)
    {
        value = uncoveredPositives[i].getAttribute(attIndex);
        if(value > max)
            max = value;
        else if(value < min)
            min = value;
    }

    map<double, double> values;
    double attValue;
    for (int i = 0; i < covered.size(); i++)
    {
        attValue = covered[i].getAttribute(attIndex);
        if (attValue == attValue)
            values.insert(pair<double, double>(attValue, covered[i].getDecisionAttribute()));
    }

    map<double, double>::iterator it;
    double prev, mean, prevClass;
    RuleEvaluationResult rer1, rer2;
    it = values.begin();
    if (it != values.end())
    {
        prev = it->first;
        prevClass = it->second;
        it++;
    }
    for (; it != values.end(); it++)
    {
        if (it->second != prevClass)
        {
            mean = (it->first + prev) / 2;
            ElementaryCondition cond1(attIndex, new LessThanOperator(), mean);
            ElementaryCondition cond2(attIndex, new GreaterEqualOperator(), mean);
            rer1 = RuleQualityMeasure::EvaluateCondition(uncoveredPositives, cond1, decClass);
            rer2 = RuleQualityMeasure::EvaluateCondition(uncoveredPositives, cond2, decClass);
            if(rer1.p > rer2.p)
            {
                if(min < mean)
                    conds.push_back(cond1);
            }
            else if (max >= mean)
                conds.push_back(cond2);
            
            prevClass = it->second;
        }
        prev = it->first;
    }
}
