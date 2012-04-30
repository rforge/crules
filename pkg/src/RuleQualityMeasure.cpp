#include "RuleQualityMeasure.h"

using namespace std;

/**
 * Generates RuleEvaluationResult object for the rule based on the set of examples
 * @param ds set of examples
 * @param rule evaluated rule
 * @return RuleEvaluationResult object
 */
RuleEvaluationResult RuleQualityMeasure::EvaluateRule(SetOfExamples& dataset, Rule& rule)
{
    RuleEvaluationResult result;
    int size = dataset.size();
    for (int i = 0; i < size; i++)
    {
        if (dataset[i].getDecisionAttribute() == rule.getDecisionClass()) //if positive
        {
            result.P += dataset[i].getWeight();
            if (rule.covers(dataset[i]))
                result.p += dataset[i].getWeight();
        }
        else //if negative
        {
            result.N += dataset[i].getWeight();
            if (rule.covers(dataset[i]))
                result.n += dataset[i].getWeight();
        }
    }
    return result;
}

/**
 * Generates RuleEvaluationResult object for the elementary condition based on the set of examples
 * @param ds set of examples
 * @param rule evaluated condition
 * @param decClass positive class value
 * @return RuleEvaluationResult object
 */
RuleEvaluationResult RuleQualityMeasure::EvaluateCondition(SetOfExamples& dataset, ElementaryCondition& cond, double decClass)
{
    RuleEvaluationResult result;
    int size = dataset.size();
    int attIndex = cond.getAttributeIndex();
    for (int i = 0; i < size; i++)
    {
        if (dataset[i].getDecisionAttribute() == decClass) //if positive
        {
            result.P += dataset[i].getWeight();
            if (cond.isSatisfied(dataset[i].getAttribute(attIndex)))
                result.p += dataset[i].getWeight();
        }
        else //if negative
        {
            result.N += dataset[i].getWeight();
            if (cond.isSatisfied(dataset[i].getAttribute(attIndex)))
                result.n += dataset[i].getWeight();
        }
    }
    return result;
}

/**
 * Evaluates entropy for the set of examples
 * @param examples set of examples
 * @return value of entropy
 */
double NegConditionalEntropy::Entropy(SetOfExamples& examples)
{
	set<double> classes;
	//int size = examples.size();
        double size = examples.getSumOfWeights();
	for(int i = 0; i < size; i++)
		classes.insert(examples[i].getDecisionAttribute());

    double sum = 0, p;
    set<double>::iterator it;
    for(it = classes.begin(); it != classes.end(); it++)
    {
        //p = (double)examples.getExamplesForDecAtt(*it).size() / size;
        p = examples.getExamplesForDecAtt(*it).getSumOfWeights() / size;
        if(p != 0)
            sum += p * Log2(p);
        //sum += p * log2(p);
    }
    return -sum;
}

/**
 * Computes negated value of conditional entropy for the rule and the set of examples
 * @param ds set of examples
 * @param rule rule
 * @return negated value of conditional entropy
 */
double NegConditionalEntropy::EvaluateRuleQuality(SetOfExamples& ds, Rule& rule)
{
    SetOfExamples s1(ds.getDataSet());
    SetOfExamples s2(ds.getDataSet());
    int size = ds.size();
    for(int i = 0; i < size; i++)
    {
        if(rule.covers(ds[i]))
            s1.addExample(ds, i);
        else
            s2.addExample(ds, i);
    }
    double sumOfWeights = ds.getSumOfWeights();
    //double result = ((double)s1.size() / (double)size) * Entropy(s1) + ((double)s2.size() / (double)size) * Entropy(s2);
    double result = (s1.getSumOfWeights() / sumOfWeights) * Entropy(s1) + (s2.getSumOfWeights() / sumOfWeights) * Entropy(s2);
    return -result;
}

/**
 * Computes negated value of conditional entropy for the elementary condition and the set of examples
 * @param ds set of examples
 * @param rule rule
 * @param decClass positive class value
 * @return negated value of conditional entropy
 */
double NegConditionalEntropy::EvaluateConditionQuality(SetOfExamples& ds, ElementaryCondition& cond, double decClass)
{
    SetOfExamples s1(ds.getDataSet());
    SetOfExamples s2(ds.getDataSet());
    int attIndex = cond.getAttributeIndex();
    int size = ds.size();
    for(int i = 0; i < size; i++)
    {
        if(cond.isSatisfied(ds[i].getAttribute(attIndex)))
            s1.addExample(ds, i);
        else
            s2.addExample(ds, i);
    }
    double sumOfWeights = ds.getSumOfWeights();
    //double result = ((double)s1.size() / (double)size) * Entropy(s1) + ((double)s2.size() / (double)size) * Entropy(s2);
    double result = (s1.getSumOfWeights() / sumOfWeights) * Entropy(s1) + (s2.getSumOfWeights() / sumOfWeights) * Entropy(s2);
    return -result;
}
