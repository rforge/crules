#include "RuleClassifier.h"

using namespace std;

/**
 * Performs classification of an example
 * @param example example to be classified
 * @return predicted class value
 */
double RuleClassifier::classifyExample(Example& example)
{
	list<Rule> coveringRules(getCoveringRules(example));
	if(coveringRules.size() == 1)
		return coveringRules.front().getDecisionClass();
	if(coveringRules.size() > 1)
		return resolveConflict(coveringRules);
	//if no rule covers the example
	return numeric_limits<double>::quiet_NaN();
}

/*
Confidence degrees of the rules covering the test example are summed up for each decision class
and then the class with maximal confidence degree is picked.
*/
/**
* Resolves conflict when the example is covered by more than one rule.
* Uses voting strategy, in which the voting power of each rule is its confidence degree.
  * @param coveringRules list of rules covering an example.
  * @return predicted class value
 */
double RuleClassifier::resolveConflict(list<Rule>& coveringRules)
{//voting scheme:
	map<double, double> sumsOfConfidenceDegress;	//one item per class
	map<double, double>::iterator itMap;
	list<Rule>::iterator it;
	double decisionClass, confidenceDegree;
	for(it = coveringRules.begin(); it != coveringRules.end(); it++)
	{
		decisionClass = it->getDecisionClass();
		confidenceDegree = it->getConfidenceDegree();
		itMap = sumsOfConfidenceDegress.find(decisionClass);
		if(itMap == sumsOfConfidenceDegress.end())
			sumsOfConfidenceDegress[decisionClass] = confidenceDegree;
		else	//addition of confidence degree
			itMap->second += confidenceDegree;
	}
	confidenceDegree = -numeric_limits<double>::max();
	decisionClass = numeric_limits<double>::quiet_NaN();
	for (itMap = sumsOfConfidenceDegress.begin(); itMap != sumsOfConfidenceDegress.end(); itMap++)
		if((itMap->second) > confidenceDegree)
		{
			confidenceDegree = itMap->second;
			decisionClass = itMap->first;
		}
	return decisionClass;
}

/**
 * Performs classification of a set of examples
 * @param examples set of examples to be classified
 * @return vector of predicted class values
 */
vector<double> RuleClassifier::classifyExamples(SetOfExamples& examples)
{
    vector<double> predictions(examples.size(), numeric_limits<double>::quiet_NaN());
    for(int i = 0; i < examples.size(); i++)
        predictions[i] = classifyExample(examples[i]);
    return predictions;
}

/**
 * Metoda generująca macierz pomyłek dla zadanego zbioru testowego.
 * @param testSet testowy zbiór przykładów.
 * @param preds wektor przewidywanych wartości klas decyzyjnych dla poszczególnych przykładów.
 * Jeśli wektor ten jest pusty, wówczas dokonywana jest klasyfikacja i zostaje on uzupełniony.
 * @return obiekt będący macierzą pomyłek.
 */
ConfusionMatrix RuleClassifier::generateConfusionMatrix(SetOfExamples& testSet, vector<double>& preds)
{
	ConfusionMatrix confusionMatrix(testSet.getAllClassesCount());	//actual class in rows, predicted in cols
	unsigned int numOfExamples = testSet.size();
	double actualClass, predictedClass;
        if(preds.size() != numOfExamples)   //if preds is empty, fill it with predictions
        {
            preds.clear();
            preds.resize(numOfExamples);
            for(unsigned int i = 0; i < numOfExamples; i++)
            {
                actualClass = testSet[i].getDecisionAttribute();
                predictedClass = classifyExample(testSet[i]);
                if(predictedClass == predictedClass)
                    confusionMatrix[actualClass][predictedClass]++;
                else
                    confusionMatrix.getUnclassified()[actualClass]++;
                preds[i] = predictedClass;
            }
        }
        else    //using preds instead of classifying
        {
            for(unsigned int i = 0; i < numOfExamples; i++)
            {
                actualClass = testSet[i].getDecisionAttribute();
                predictedClass = preds[i];
                if(predictedClass == predictedClass)
                    confusionMatrix[actualClass][predictedClass]++;
                else
                    confusionMatrix.getUnclassified()[actualClass]++;
            }
        }

	return confusionMatrix;
}

/**
 * Generates confusion matrix for given test set using weights.
 * @param testSet test set of examples
 * @param preds vector of predicted class values for each example
 * If this vector is empty, classification is performed.
 * @return confusion matrix
 */
ConfusionMatrix RuleClassifier::generateConfusionMatrixWithWeights(SetOfExamples& testSet, vector<double>& preds)
{
	ConfusionMatrix confusionMatrix(testSet.getAllClassesCount());	//actual class in rows, predicted in cols
	unsigned int numOfExamples = testSet.size();
	double actualClass, predictedClass;
        if(preds.size() != numOfExamples)   //if preds is empty, fill it with predictions
        {
            preds.clear();
            preds.resize(numOfExamples);
            for(unsigned int i = 0; i < numOfExamples; i++)
            {
                actualClass = testSet[i].getDecisionAttribute();
                predictedClass = classifyExample(testSet[i]);
                if(predictedClass == predictedClass)
                    confusionMatrix[actualClass][predictedClass] += testSet[i].getWeight();
                else
                    confusionMatrix.getUnclassified()[actualClass] += testSet[i].getWeight();
                preds[i] = predictedClass;
            }
        }
        else    //using preds instead of classifying
        {
            for(unsigned int i = 0; i < numOfExamples; i++)
            {
                actualClass = testSet[i].getDecisionAttribute();
                predictedClass = preds[i];
                if(predictedClass == predictedClass)
                    confusionMatrix[actualClass][predictedClass] += testSet[i].getWeight();
                else
                    confusionMatrix.getUnclassified()[actualClass] += testSet[i].getWeight();
            }
        }

	return confusionMatrix;
}

/**
 * Evaluates classification accuracy
 * @param cm confusion matrix
 * @return evaluation of classification accuracy
 */
double RuleClassifier::evaluateAccuracy(ConfusionMatrix& cm)
{
	//return (double)cm.getNumberOfCorrectlyClassifiedExamples() / cm.getNumberOfExamples();
    return cm.getSumOfCorrectlyClassifiedExamples() / cm.getSumOfExamples();
}

vector<double> RuleClassifier::evaluateClassesAccuracy(ConfusionMatrix& cm)
{
    vector<double> classesAcc(cm.getNumberOfClasses());
    for(unsigned int i = 0; i < classesAcc.size(); i++)
    {
        double weightOfEx = cm.getSumOfExamples(i);
        if(weightOfEx > 0)
            classesAcc[i] = cm.getSumOfTruePositives(i) / weightOfEx;
        else
            classesAcc[i] = numeric_limits<double>::quiet_NaN();
    }
    return classesAcc;
}

double RuleClassifier::evaluateAvgAccuracy(vector<double> accs)
{
    int notEmptyClassesCount = 0;
    double sumOfAccs = 0;
    for(unsigned int i = 0; i < accs.size(); i++)
        if(accs[i] == accs[i])
        {
            sumOfAccs += accs[i];
            notEmptyClassesCount++;
        }
    return (double)sumOfAccs / notEmptyClassesCount;
}

/**
 * Evaluates average classifiaction acccuracy of decision classes
 * @param cm confusion matrix
 * @return evaluation of average classifiaction acccuracy of decision classes
 */
double RuleClassifier::evaluateAvgAccuracy(ConfusionMatrix& cm)
{
    return evaluateAvgAccuracy(evaluateClassesAccuracy(cm));
}

/**
 * Evaluates coverage of set of examples by rule classfier
 * @param predictions vector of predicted class values
 * @return evaluation of coverage
 */
double RuleClassifier::getCoverage(vector<double>& predictions)
{
    int nans = 0;
    int size = predictions.size();
    for (int i = 0; i < size; i++)
        if (predictions[i] != predictions[i])
            nans++;
    return 1.0 - (double) nans / size;
}

double RuleClassifier::getCoverage(vector<double>& predictions, SetOfExamples& examples)
{
    double nans = 0;
    int size = predictions.size();
    for (int i = 0; i < size; i++)
        if (predictions[i] != predictions[i])
            nans += examples[i].getWeight();
    return 1.0 - nans / examples.getSumOfWeights();
}
/**
 * Adds rule to classifier
 * @param newRule rule to be added to classifier
 */
void RuleClassifier::addRule(Rule& newRule)
{
	rules.push_back(newRule);
}

/**
 * Adds list of rules to classfier
 * @param newRules list of rule to be added to classfier
 */
void RuleClassifier::addRules(list<Rule>& newRules)
{
    rules.insert(rules.end(), newRules.begin(), newRules.end());
}

/**
 * Returns list of rules which cover the example
 * @param example
 * @return list of rules which cover the example
 */
list<Rule> RuleClassifier::getCoveringRules(Example& example)
{
	list<Rule> coveringRules;
	list<Rule>::iterator it;
	for(it = rules.begin(); it != rules.end(); it++)
	{
		if(it->covers(example))
			coveringRules.push_back(*it);
	}
	return coveringRules;
}

string RuleClassifier::toString()
{
	ostringstream oss;
	list<Rule>::iterator it;
	for(it = rules.begin(); it != rules.end(); it++)
	{
		oss << it->toString() << endl;
	}
	return oss.str();
}

string RuleClassifier::toString(DataSet& ds)
{
    ostringstream oss;
    list<Rule>::iterator it;
    for (it = rules.begin(); it != rules.end(); it++)
        oss << it->toString(ds) << endl;
    return oss.str();
}

vector<string> RuleClassifier::toVectorOfStrings(DataSet& ds)
{
    vector<string> strings;
    list<Rule>::iterator it;
    for (it = rules.begin(); it != rules.end(); it++)
        strings.push_back(it->toString());
    return strings;
}

/**
 * Computes the most important statistics of the rule set
 * @param examples set of examples which the statistics will be computed on
 * @return object of the RuleSetStats class which contains values of computed statistics
 */
RuleSetStats RuleClassifier::getRuleSetStats(SetOfExamples& examples)
{
    RuleSetStats stats;
    Coverage coverage;
    Precision precision;
    Pvalue pvalue;
    RuleEvaluationResult rer;
    int nconds = 0;
    list<Rule>::iterator it;
    list<ElementaryCondition>::iterator itCond;
    vector<list<ElementaryCondition> >::iterator itVec;
    stats.warning = false;
    for (it = rules.begin(); it != rules.end(); it++)
    {
        nconds = 0;
        for (itVec = it->getConditions().begin(); itVec != it->getConditions().end(); itVec++)
        	if(itVec->size() > 0)
        		nconds++;
        rer  = RuleQualityMeasure::EvaluateRule(examples, *it);
        stats.condCounts.push_back(nconds);
        stats.precs.push_back(precision.EvaluateRuleQualityFromResult(rer));
        stats.covs.push_back(coverage.EvaluateRuleQualityFromResult(rer));
        stats.pvalues.push_back(pvalue.ComputePvalue(rer, stats.warning));
    }
    return stats;
}
