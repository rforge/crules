#ifndef SEQUENTIALCOVERING_H
#define	SEQUENTIALCOVERING_H

#include "Attribute.h"
#include "Example.h"
#include "ElementaryCondition.h"
#include "SetOfExamples.h"
#include "Rule.h"
#include "RuleQualityMeasure.h"
#include "Operator.h"
#include <assert.h>
#include <cstdlib>
#include <list>
#include <vector>
#include <set>
#include <map>
#include <limits>

/**
 * Class represents a sequential covering strategy for induction of decision rules.
 * Performs the induction of decision rules based on given training set and using
 * specific rule quality measures for the phases of rule growth and pruning.
 */
class SequentialCovering
{
public:
	std::list<Rule> generateRules(SetOfExamples& examples, RuleQualityMeasure& rqmGrow, RuleQualityMeasure& rqmPrune);
	std::list<Rule> generateRulesForClass(SetOfExamples&, RuleQualityMeasure& rqmGrow, RuleQualityMeasure& rqmPrune, double decClass);
	void growRule(Rule& rule, SetOfExamples& covered, SetOfExamples uncoveredPositives, RuleQualityMeasure& ruleQualityMeasure);
	void pruneRule(Rule& rule, SetOfExamples& examples, RuleQualityMeasure& ruleQualityMeasure);
	SetOfExamples getCoveredExamples(Rule& rule, SetOfExamples& examples);
    SetOfExamples getCoveredExamples(ElementaryCondition& cond, SetOfExamples& examples);
private:
	bool existsCoveredExample(Rule& rule,SetOfExamples& examples);
	bool existsCoveredExample(ElementaryCondition& condition,SetOfExamples& examples);
	bool existsExampleWithEqualAttValue(int attIndex, double attValue, SetOfExamples& examples);
	ElementaryCondition findBestCondition(double decClass, SetOfExamples& covered, SetOfExamples& uncoveredPositives, RuleQualityMeasure& rqm, bool isRqmEntropy);
	void findBestConditionForNominalAttribute(double decClass, SetOfExamples& covered, SetOfExamples& uncoveredPositives,
			RuleQualityMeasure& rqm, bool isRqmEntropy, int attributeIndex, std::list<ElementaryCondition>& equallyBestConditions, double& bestQuality);
	void findBestConditionForNumericalAttribute(double decClass, SetOfExamples& covered, SetOfExamples& uncoveredPositives,
			RuleQualityMeasure& rqm, bool isRqmEntropy, int attributeIndex, std::list<ElementaryCondition>& equallyBestConditions, double& bestQuality);
	ElementaryCondition chooseConditionFromEqual(std::list<ElementaryCondition>& equallyBestConditions, double decClass, SetOfExamples& uncoveredPositives);
	int getNumberOfValuesLessOrGreater(std::multiset<double>& values, double value, bool takeLess);

	double P; /**< Number of all positive examples*/
    double N; /**< Number of all negative examples*/
};

#endif	/* SEQUENTIALCOVERING_H */
