#ifndef SEQUENTIALCOVERINGPREFERENCES_H
#define	SEQUENTIALCOVERINGPREFERENCES_H

#include "Attribute.h"
#include "Example.h"
#include "ElementaryCondition.h"
#include "SetOfExamples.h"
#include "Rule.h"
#include "RuleQualityMeasure.h"
#include "Operator.h"
#include "Knowledge.h"
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
class SequentialCoveringWithPreferences
{
public:
	SequentialCoveringWithPreferences(Knowledge* knowledge) : knowledge(knowledge), P(0), N(0) {}
	virtual ~SequentialCoveringWithPreferences()
	{
		delete knowledge;
	}
	std::list<Rule> generateRules(SetOfExamples& examples, RuleQualityMeasure& rqmGrow, RuleQualityMeasure& rqmPrune);
	std::list<Rule> generateRulesForClass(SetOfExamples&, RuleQualityMeasure& rqmGrow, RuleQualityMeasure& rqmPrune, double decClass);
	void growRule(Rule& rule, SetOfExamples& covered, SetOfExamples uncoveredPositives, RuleQualityMeasure& ruleQualityMeasure, bool useSpecifiedOnly, KnowledgeRule* knowRule = NULL);
	void pruneRule(Rule& rule, SetOfExamples& examples, RuleQualityMeasure& ruleQualityMeasure);
	SetOfExamples getCoveredExamples(Rule& rule, SetOfExamples& examples);
    SetOfExamples getCoveredExamples(ElementaryCondition& cond, SetOfExamples& examples);
private:
	bool existsCoveredExample(Rule& rule,SetOfExamples& examples);
	bool existsCoveredExample(ElementaryCondition& condition,SetOfExamples& examples);
	bool existsExampleWithEqualAttValue(int attIndex, double attValue, SetOfExamples& examples);
	ElementaryCondition findBestCondition(Rule& rule, double decClass, SetOfExamples& covered, SetOfExamples& uncoveredPositives, RuleQualityMeasure& rqm, bool isRqmEntropy, bool useSpecifiedOnly, KnowledgeRule* knowRule = NULL);
	void findBestConditionForNominalAttribute(Rule& rule, double decClass, SetOfExamples& covered, SetOfExamples& uncoveredPositives,
			RuleQualityMeasure& rqm, bool isRqmEntropy, int attributeIndex, std::list<ElementaryCondition>& equallyBestConditions, double& bestQuality, bool useSpecifiedOnly);
	void findBestConditionForNumericalAttribute(Rule& rule, double decClass, SetOfExamples& covered, SetOfExamples& uncoveredPositives,
			RuleQualityMeasure& rqm, bool isRqmEntropy, int attributeIndex, std::list<ElementaryCondition>& equallyBestConditions, double& bestQuality, bool useSpecifiedOnly);
	ElementaryCondition chooseConditionFromEqual(std::list<ElementaryCondition>& equallyBestConditions, double decClass, SetOfExamples& uncoveredPositives);
	int getNumberOfValuesLessOrGreater(std::multiset<double>& values, double value, bool takeLess);

	bool isNominalConditionForbidden(Rule& rule, double decClass, int attributeIndex, double value);
	bool isNominalConditionRequired(ElementaryCondition& condition, Rule& rule);

	bool isNumericConditionSpecified(double value, bool greaterEqual, std::list<KnowledgeCondition>& conditions, bool andRequired = false);
	bool isNumericConditionForbidden(Rule& rule, double decClass, int attributeIndex, double value, bool greaterEqual);
	bool isNumericConditionRequired(ElementaryCondition& condition, Rule& rule);
	bool isConitionsInterceptionNotEmpty(double decClass, int attributeIndex, double value, bool greaterEqual, KnowledgeCondition& cond);

	bool isConditionForbiddenInRule(Rule& rule, double decClass, int attributeIndex, double value, bool greaterEqual, bool nominal);

	Rule* getRuleFromKnowledgeRule(KnowledgeRule& kRule, bool fixedAndRequiredOnly = false);

	Knowledge* knowledge;
	double P; /**< Number of all positive examples*/
    double N; /**< Number of all negative examples*/
};

#endif	/* SEQUENTIALCOVERINGPREFERENCES_H */
