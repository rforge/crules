#ifndef RULECLASSIFIER_H
#define	RULECLASSIFIER_H

#include "Attribute.h"
#include "Example.h"
#include "ElementaryCondition.h"
#include "SetOfExamples.h"
#include "Rule.h"
#include "RuleQualityMeasure.h"
#include "Operator.h"
#include "ConfusionMatrix.h"
#include "SequentialCovering.h"
#include <map>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include <limits>
#include <algorithm>


class RuleSetStats;

/**
 * Represents rule classifier. Contains list of rules and provides methods for classification
 * (of an example of a set of examples) and computing some statistics (eg. classification accuracy)
 */
class RuleClassifier
{
public:
    RuleClassifier() {}
    RuleClassifier(std::list<Rule>& rules): rules(rules) {}
    double classifyExample(Example& example);
    std::vector<double> classifyExamples(SetOfExamples& examples);
    ConfusionMatrix generateConfusionMatrix(SetOfExamples& testSet, std::vector<double>& preds);
    ConfusionMatrix generateConfusionMatrixWithWeights(SetOfExamples& testSet, std::vector<double>& preds);
    double evaluateAccuracy(ConfusionMatrix& cm);
    std::vector<double> evaluateClassesAccuracy(ConfusionMatrix& cm);
    double evaluateAvgAccuracy(std::vector<double> accs);
    double evaluateAvgAccuracy(ConfusionMatrix& cm);
    //double getCoverage(ConfusionMatrix& cm) { return 1 - (double)cm.getNumberOfUncoveredExamples()/cm.getNumberOfExamples();}
    double getCoverage(ConfusionMatrix& cm) { return 1 - (double)cm.getSumOfUncoveredExamples()/cm.getSumOfExamples();}
    double getCoverage(std::vector<double>& predictions);
    double getCoverage(std::vector<double>& predictions, SetOfExamples& examples);
    void removeRules();
    void addRule(Rule& newRule);
    void addRules(std::list<Rule>& newRules);
    std::list<Rule> getCoveringRules(Example& example);
    RuleSetStats getRuleSetStats(SetOfExamples& examples);
    std::string toString();
    std::string toString(DataSet& ds);
    std::vector<std::string> toVectorOfStrings(DataSet& ds);
    void setRules(std::list<Rule>& rules) { removeRules(); this->rules = rules; }
    std::list<Rule>& getRules() { return rules; }
private:
    double resolveConflict(std::list<Rule>&);
    std::list<Rule> rules;
};

/**
 * RuleSetStats
 * Contains values of some rule statistics
 */
class RuleSetStats {
public:
    RuleSetStats() {}
    std::vector<int> condCounts;    /**< number of elementary conditions in rule*/
    std::vector<double> precs;         /**< rule precisions*/
    std::vector<double> covs;          /**< rule coverages*/
    std::vector<double> pvalues;          /**< p-values*/
    bool warning;
};

#endif	/* RULECLASSIFIER_H */
