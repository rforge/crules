#include "RInterface.h"
#include "RuleClassifier.h"

using namespace std;
/**
 * Performs rules induction. Takes arguments from R and returns valus to it.
 * @param y vector of decision attribute values (or indices in case of nominal attribute)
 * @param yname decision attribute's name
 * @param ylevels unique values of decision attribute
 * @param x values (or indices in case of nominal attributes) of conditional attributes
 * @param xtypes vector of conditional attributes' types
 * @param xnames vector of conditional attributes' names
 * @param xlevels 2-dim table of unique values of conditional attributes
 * @param rqmPrune name of rule quality measure to be used in pruning phase
 * @param rqmGrow  name of rule quality measure to be used in growing phase
 * @return representation of generated rules and their statistics
 */
Rcpp::List RInterface::generateRules(vector<double> y, string yname, vector<string> ylevels,
                                                 SEXP x, vector<string> xtypes,
                                                 vector<string> xnames, SEXP xlevels,
                                                 string rqmPrune, string rqmGrow,
                                                 SEXP rqmPruneCustom, SEXP rqmGrowCustom, vector<double> weights)
{
    try
    {
        //creating data set
        DataSet* ds = createDataSet(y, yname, ylevels, x, xtypes, xnames, xlevels, weights);
        SetOfExamples examples(*ds, true);
        //choosing rule quality measure
        RuleQualityMeasure* rqmGrowPtr = createRuleQualityMeasure(rqmGrow, rqmGrowCustom);
        RuleQualityMeasure* rqmPrunePtr = createRuleQualityMeasure(rqmPrune, rqmPruneCustom);
        //generating rules
        SequentialCovering sc;
        list<Rule> rules = sc.generateRules(examples, *rqmGrowPtr, *rqmPrunePtr);
        RuleClassifier ruleClassifier(rules);

        Rcpp::List result = serializeRules(ruleClassifier, examples);
        delete ds;
        delete rqmGrowPtr;
        delete rqmPrunePtr;
        return result;
    }
    catch (RulesInductionException& ex)
    {
        cout << "Exception occured: " << ex.getMessage() << endl;
        return Rcpp::List();
    }
    return 0;
}

/**
 * Performs prediction of class values for examples from test set
 * @param y vector of decision attribute values (or indices in case of nominal attribute); may be empty
 * @param yname decision attribute's name
 * @param ylevels unique values of decision attribute
 * @param x values (or indices in case of nominal attributes) of conditional attributes
 * @param xtypes vector of conditional attributes' types
 * @param xnames vector of conditional attributes' names
 * @param xlevels 2-dim table of unique values of conditional attributes
 * @param _serialRules vector of strings representing rules
 * @param confidenceDegrees vector of confidence degree for each rule
 * @param weights vector of case weights
 * @return predicted class values and statistics of classification
 */
Rcpp::List RInterface::predict(vector<double> y, string yname, vector<string> ylevels,
                                           SEXP x, vector<string> xtypes,
                                           vector<string> xnames, SEXP xlevels, vector<string> _serialRules,
                               vector<double> confidenceDegrees, vector<double> weights)
{
    try
    {
        DataSet* ds = createDataSet(y, yname, ylevels, x, xtypes, xnames, xlevels, weights);
        SetOfExamples examples(*ds, true);
        RuleClassifier ruleClassifier(deserializeRules(_serialRules, confidenceDegrees, *ds));
        vector<double> predictions = ruleClassifier.classifyExamples(examples);
        double acc = numeric_limits<double>::quiet_NaN();
        double bac = numeric_limits<double>::quiet_NaN();
        ConfusionMatrix cm(0);
        vector<double> classesAccuracies;
        if (y.size() > 0)
        {
            cm = ruleClassifier.generateConfusionMatrixWithWeights(examples, predictions);
            acc = ruleClassifier.evaluateAccuracy(cm);
            classesAccuracies = ruleClassifier.evaluateClassesAccuracy(cm);
            bac = ruleClassifier.evaluateAvgAccuracy(classesAccuracies);
        }
        double cov = ruleClassifier.getCoverage(predictions, examples);

        Rcpp::List result = Rcpp::List::create(Rcpp::Named("acc", acc), Rcpp::Named("bac", bac),
                                               Rcpp::Named("cov", cov), Rcpp::Named("predictions", predictions),
                                               Rcpp::Named("confusionMatrix", cm.getMatrix()),
                                               Rcpp::Named("classesAccuracies", classesAccuracies),
                                               Rcpp::Named("unclassified", cm.getUnclassified()));
        delete ds;
        return result;
    }
    catch (RulesInductionException& ex)
    {
        cout << "Exception occured: " << ex.getMessage() << endl;
        return Rcpp::List();
    }
    return 0;
}



/**
 * Creates DataSet object from data passed from R
* @param y vector of decision attribute values (or indices in case of nominal attribute); may be empty
 * @param yname decision attribute's name
 * @param ylevels unique values of decision attribute
 * @param x values (or indices in case of nominal attributes) of conditional attributes
 * @param xtypes vector of conditional attributes' types
 * @param xnames vector of conditional attributes' names
 * @param xlevels 2-dim table of unique values of conditional attributes
 * @return pointer to created DataSet object
 */
DataSet* RInterface::createDataSet(vector<double>& y, string& yname, vector<string>& ylevels,
                                               SEXP& x, vector<string>& xtypes,
                                               vector<string> xnames, SEXP& xlevels, vector<double>& weights)
{
    DataSet* ds = new DataSet();
    //if (y.size() > 0)
    //{
        ds->setDecisionAttributeIndex(0);
        Attribute att;
        att.setName(yname);
        att.setType(Attribute::NOMINAL);
        att.setLevels(ylevels);
        transform(y.begin(), y.end(), y.begin(), decrement);    //we may consider using string instead of class indices to prevent changed order of classes
        ds->addAttribute(y, att);
    //}

    Rcpp::DataFrame dfx(x);
    Rcpp::DataFrame dfxlevels(xlevels);
    Attribute::AttributeType type;
    for (unsigned int i = 0; i < xtypes.size(); i++)
    {
        Attribute att;
        vector<double> xval = Rcpp::as<vector<double> >(dfx[xnames[i]]);
        if (xtypes[i] == "numeric" || xtypes[i] == "integer")
            type = Attribute::NUMERICAL;
        else
        {
            type = Attribute::NOMINAL;
            Rcpp::StringVector sv((SEXP)dfxlevels[xnames[i]]);
            att.setLevels(Rcpp::as<vector<string> >(sv));
            transform(xval.begin(), xval.end(), xval.begin(), decrement);
        }

        att.setName(xnames[i]);
        att.setType(type);
        ds->addAttribute(xval, att);
    }
    if(weights.size() > 0)
        ds->addWeights(weights);
    return ds;
}


/**
 * "Serializes" RuleClassifier object to object proper R object
 * @param rules RuleClassifier object with inner representation of rules
 * @param examples set of examples used for computing rule statistics
 * @return list whith representation of rules that may be used in R
 */
Rcpp::List RInterface::serializeRules(RuleClassifier& rules, SetOfExamples& examples)
{
    vector<string> serialRules;
    vector<double> confidenceDegrees;
    list<Rule>::iterator itRulePtr;
    for (itRulePtr = rules.getRules().begin(); itRulePtr != rules.getRules().end(); itRulePtr++)
    {
        serialRules.push_back(itRulePtr->toString(examples.getDataSet()));
        confidenceDegrees.push_back(itRulePtr->getConfidenceDegree());
    }
    RuleSetStats stats = rules.getRuleSetStats(examples);

    if(stats.warning)
        cout << "warning: p-value has been calculated for weighted examples." << endl;

    return Rcpp::List::create(Rcpp::Named("Rules", serialRules),
                              Rcpp::Named("ConfidenceDegrees", confidenceDegrees),
                              Rcpp::Named("NumbersOfConditions", stats.condCounts),
                              Rcpp::Named("RulesPrecisions", stats.precs),
                              Rcpp::Named("RulesCoverages", stats.covs),
                              Rcpp::Named("Pvalues", stats.pvalues));
}

/**
 * "Deserializes" rules from R representation to RuleClassifier object
 * @param _serialRules R representation of rules
 * @param confidenceDegrees vector of confidence degrees for each rule
 * @return RuleClassifier object with inner representation of rules
 */
RuleClassifier RInterface::deserializeRules(vector<string> _serialRules, vector<double>& confidenceDegrees, DataSet& ds)
{
    RuleClassifier ruleClassifier;

    for (unsigned int i = 0; i < _serialRules.size(); i++)
    {
        Rule rule(Rule::parseRule(ds, _serialRules[i]));
        rule.setConfidenceDegree(confidenceDegrees[i]);
        ruleClassifier.addRule(rule);
    }
    return ruleClassifier;
}

/**
 * Creates proper RuleQualityMeasure object based on its name
 * @param name name of measure
 * @param customRqm function passed from R to be used as rqm
 * @return pointer to created RuleQualityMeasure object
 */
RuleQualityMeasure* RInterface::createRuleQualityMeasure(string name, SEXP& customRqm)
{
    if (name == "g2") return new TwoMeasure();
    if (name == "lift") return new Lift();
    if (name == "ls") return new LogicalSufficiency();
    if (name == "rss") return new RSS();
    //if (name == "ms") return new MutualSupport();
    if (name == "corr") return new Correlation();
    if (name == "s") return new SBayesConfirmation();
    if (name == "c1") return new C1();
    if (name == "c2") return new C2();
    if (name == "entropy") return new NegConditionalEntropy();
    if (name == "cn2") return new Cn2();
    if (name == "gain") return new Gain();
    if (name.empty()) return new CustomMeasure(customRqm);
    return NULL;
}

/**
 * Performs certains number (given by parameter "runs") of runs of k-fold cross validation
* @param y vector of decision attribute values (or indices in case of nominal attribute)
 * @param yname decision attribute's name
 * @param ylevels unique values of decision attribute
 * @param x values (or indices in case of nominal attributes) of conditional attributes
 * @param xtypes vector of conditional attributes' types
 * @param xnames vector of conditional attributes' names
 * @param xlevels 2-dim table of unique values of conditional attributes
 * @param rqmPrune name of rule quality measure to be used in pruning phase
 * @param rqmGrow  name of rule quality measure to be used in growing phase
 * @param nfolds number of folds in each run of cross-validation
 * @param runs number of cross-validation runs
 * @param everyClassInFold indicates if each fold should contain examples from every class
 * @param rqmPruneCustom function passed from R to be used as custom rule qualisty measure during pruning phase
 * @param rqmGrowCustom function passed from R to be used as custom rule qualisty measure during growing phase
 * @param weights vector of case weights
 * @param useWeightsInPrediction indicates if weights should be used in prediction
 * @return generated rules and statistics for every experiment
 */
Rcpp::List RInterface::crossValidation(vector<double> y, string yname, vector<string> ylevels,
                                                 SEXP x, vector<string> xtypes,
                                                 vector<string> xnames, SEXP xlevels,
                                                 string rqmPrune, string rqmGrow,
                                                 int nfolds, int runs, bool everyClassInFold, SEXP rqmPruneCustom, SEXP rqmGrowCustom,
                                                 vector<double> weights, bool useWeightsInPrediction)
{
    try
    {
        ////utworzenie zbioru danych
        DataSet* ds = createDataSet(y, yname, ylevels, x, xtypes, xnames, xlevels, weights);
        SetOfExamples examples(*ds, true);
        //wybór miar jakości reguły
        RuleQualityMeasure* rqmGrowPtr = createRuleQualityMeasure(rqmGrow, rqmGrowCustom);
        RuleQualityMeasure* rqmPrunePtr = createRuleQualityMeasure(rqmPrune, rqmPruneCustom);
        //wygenerowanie reguł
        SequentialCovering sc;
        Rcpp::List result;
        for (int i = 0; i < runs; i++)
        {
            Rcpp::List runResult;
            vector<SetOfExamples> folds = examples.createStratifiedFolds(nfolds, everyClassInFold);
            for (int j = 0; j < nfolds; j++)
            {
                //induction
                Rcpp::List foldResult;
                SetOfExamples testSet = folds[j];
                SetOfExamples trainSet = examples - testSet;
                list<Rule> rules = sc.generateRules(trainSet, *rqmGrowPtr, *rqmPrunePtr);
                RuleClassifier ruleClassifier(rules);
                foldResult.push_back(serializeRules(ruleClassifier, trainSet));
                //prediction
                vector<double> predictions = ruleClassifier.classifyExamples(testSet);
                ConfusionMatrix cm(0);
                if (useWeightsInPrediction)
                    cm = ruleClassifier.generateConfusionMatrixWithWeights(testSet, predictions);
                else
                    cm = ruleClassifier.generateConfusionMatrix(testSet, predictions);
                double acc = ruleClassifier.evaluateAccuracy(cm);
                vector<double> classesAccuracies = ruleClassifier.evaluateClassesAccuracy(cm);
                double bac = ruleClassifier.evaluateAvgAccuracy(classesAccuracies);
                double cov = ruleClassifier.getCoverage(cm);
                foldResult.push_back(Rcpp::List::create(Rcpp::Named("acc", acc), Rcpp::Named("bac", bac),
                                                        Rcpp::Named("cov", cov), Rcpp::Named("predictions", predictions),
                                                        Rcpp::Named("confusionMatrix", cm.getMatrix()),
                                                        Rcpp::Named("classesAccuracies", classesAccuracies),
                                                        Rcpp::Named("unclassified", cm.getUnclassified())));
                runResult.push_back(foldResult);
            }
            result.push_back(runResult);
        }
        delete ds;
        delete rqmGrowPtr;
        delete rqmPrunePtr;
        return result;
    }
    catch (RulesInductionException& ex)
    {
        cout << "Exception occured: " << ex.getMessage() << endl;
        return Rcpp::List();
    }
    return 0;
}
