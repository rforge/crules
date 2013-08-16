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
Rcpp::List RInterface::generateRules(Rcpp::List params)
{
    try
    {
    	srand ( Rcpp::as<double>(params["seed"]) *  numeric_limits<unsigned int>::max());
        //creating data set
        DataSet* ds = createDataSet(params);
        Knowledge* know = createKnowledgeObject(params, ds);
        SetOfExamples examples(*ds, true);
        //choosing rule quality measure
        RuleQualityMeasure* rqmGrowPtr = createRuleQualityMeasure(Rcpp::as<string>(params["qsplit"]), (SEXP)params["qsplitfun"]);
        RuleQualityMeasure* rqmPrunePtr = createRuleQualityMeasure(Rcpp::as<string>(params["q"]), (SEXP)params["qfun"]);
        //generating rules

        list<Rule> rules;
        if(know == NULL)
        {
        	SequentialCovering sc;
        	rules = sc.generateRules(examples, *rqmGrowPtr, *rqmPrunePtr);
        }
        else
        {
        	SequentialCoveringWithPreferences scwp(know);
        	rules = scwp.generateRules(examples, *rqmGrowPtr, *rqmPrunePtr);
        }
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
Rcpp::List RInterface::predict(Rcpp::List params)
{
    try
    {
    	srand ( Rcpp::as<double>(params["seed"]) *  numeric_limits<unsigned int>::max());
        DataSet* ds = createDataSet(params);
        SetOfExamples examples(*ds, true);
        RuleClassifier ruleClassifier(deserializeRules(Rcpp::as<vector<string> >(params["rules"]),
        												Rcpp::as<vector<double> >(params["confidenceDegrees"]), *ds));

        vector<double> predictions = ruleClassifier.classifyExamples(examples);
        double acc = numeric_limits<double>::quiet_NaN();
        double bac = numeric_limits<double>::quiet_NaN();
        ConfusionMatrix cm(0);
        vector<double> classesAccuracies;
        if (Rcpp::as<vector<double> >(params["y"]).size() > 0)
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
DataSet* RInterface::createDataSet(Rcpp::List& params)
{
    DataSet* ds = new DataSet();

    vector<double> y = Rcpp::as<vector<double> >(params["y"]);
        ds->setDecisionAttributeIndex(0);
        Attribute att;
        att.setName(Rcpp::as<string>(params["yname"]));
        att.setType(Attribute::NOMINAL);
        att.setLevels(Rcpp::as<vector<string> >(params["ylevels"]));

        //auto decrement = [](double v) { return --v; };

        transform(y.begin(), y.end(), y.begin(), decrement);    //we may consider using string instead of class indices to prevent changed order of classes
        ds->addAttribute(y, att);

    Rcpp::DataFrame dfx((SEXP)params["x"]);
    Rcpp::StringVector xtypes((SEXP)params["xtypes"]);
    Rcpp::List xlevels((SEXP)params["xlevels"]);
    const vector<string>& xnames = Rcpp::as<vector<string> >(params["xnames"]);
    const vector<double>& weights = Rcpp::as<vector<double> >(params["weights"]);

    Attribute::AttributeType type;
    for (int i = 0; i < xtypes.size(); i++)
    {
        Attribute att;
        vector<double> xval = Rcpp::as<vector<double> >(dfx[xnames[i]]);
        if (xtypes[i] == "numeric" || xtypes[i] == "integer")
            type = Attribute::NUMERICAL;
        else
        {
            type = Attribute::NOMINAL;
            att.setLevels(Rcpp::as<vector<string> >(xlevels[xnames[i]]));
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
RuleClassifier RInterface::deserializeRules(vector<string> _serialRules, vector<double> confidenceDegrees, DataSet& ds)
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
RuleQualityMeasure* RInterface::createRuleQualityMeasure(string name, SEXP customRqm)
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
*/
Rcpp::List RInterface::crossValidation(Rcpp::List params)
{
    try
    {
    	srand ( Rcpp::as<double>(params["seed"]) *  numeric_limits<unsigned int>::max());
		//creating data set
		DataSet* ds = createDataSet(params);
		SetOfExamples examples(*ds, true);
		//choosing rule quality measure
		RuleQualityMeasure* rqmGrowPtr = createRuleQualityMeasure(Rcpp::as<string>(params["qsplit"]), (SEXP)params["qsplitfun"]);
		RuleQualityMeasure* rqmPrunePtr = createRuleQualityMeasure(Rcpp::as<string>(params["q"]), (SEXP)params["qfun"]);
		int runs = Rcpp::as<int>(params["runs"]);
		int nfolds = Rcpp::as<int>(params["folds"]);
		bool everyClassInFold = Rcpp::as<bool>(params["everyClassInFold"]);
		bool useWeightsInPrediction = Rcpp::as<bool>(params["useWeightsInPrediction"]);

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

double RInterface::decrement(double value)
{
	return --value;
}

Knowledge* RInterface::createKnowledgeObject(Rcpp::List& params, DataSet* dataSet)
{
	if(Rf_isNull((SEXP)params["knowledge"]))
		return NULL;

	const char* ALLOWED_CONDITIONS = "allowedConditions";
	const char* FORBIDDEN_CONDITIONS = "forbiddenConditions";
	const char* ALLOWED_RULES = "allowedRules";
	const char* FORBIDDEN_RULES = "forbiddenRules";
	SetOfConditions* conditions;

	int attrCount = dataSet->getAttributes().size() - 1;
	Rcpp::S4 rKnow((SEXP)params["knowledge"]);
	Knowledge* know = new Knowledge(attrCount, rKnow.slot("generateRulesForOtherClasses"), rKnow.slot("useSpecifiedOnly"));

	Rcpp::List requirements(rKnow.slot("requirements"));

	//for each class
	unsigned int decAttrLevelsCount = dataSet->getDecisionAttribute().getLevels().size();
	for(unsigned int classIndex = 0; classIndex < decAttrLevelsCount; classIndex++)
	{
		std::string className = dataSet->getDecisionAttribute().getStringValue(classIndex);
		if(!requirements.containsElementNamed(className.c_str()))
			continue;

		Rcpp::List currentReqs((SEXP)requirements[className]);

		if(currentReqs.containsElementNamed(ALLOWED_CONDITIONS))
		{
			Rcpp::S4 rConditions((SEXP)currentReqs[ALLOWED_CONDITIONS]);

			conditions = getSetOfConditionsFromRConditions(rConditions, dataSet, classIndex);
			know->getAllowedConditions()[classIndex] = *conditions;
			delete conditions;
		}
		if(currentReqs.containsElementNamed(FORBIDDEN_CONDITIONS))
		{
			Rcpp::S4 rConditions((SEXP)currentReqs[FORBIDDEN_CONDITIONS]);

			conditions = getSetOfConditionsFromRConditions(rConditions, dataSet, classIndex);
			know->getForbiddenConditions()[classIndex] = *conditions;
			delete conditions;
		}
		if(currentReqs.containsElementNamed(ALLOWED_RULES))
		{
			Rcpp::List rRules((SEXP)currentReqs[ALLOWED_RULES]);
			fillListOfRulesWithRRules(know->getAllowedRules()[classIndex], rRules, dataSet, classIndex);
		}
		if(currentReqs.containsElementNamed(FORBIDDEN_RULES))
		{
			Rcpp::List rRules((SEXP)currentReqs[FORBIDDEN_RULES]);
			fillListOfRulesWithRRules(know->getForbiddenRules()[classIndex], rRules, dataSet, classIndex);
		}
	}
	return know;
}

SetOfConditions* RInterface::getSetOfConditionsFromRConditions(Rcpp::S4& rConditions, DataSet* dataSet, int classIndex)
{
	SetOfConditions* conditions = new SetOfConditions(classIndex, Rcpp::as<bool>(rConditions.slot("expandable")),
										Rcpp::as<int>(rConditions.slot("rulesAtLeast")), Rcpp::as<bool>(rConditions.slot("forbidden")));

	Rcpp::List rCondsList(rConditions.slot("setOfConditions"));

	for(Rcpp::List::iterator it = rCondsList.begin(); it != rCondsList.end(); it++)
	{
		Rcpp::S4 rCond((SEXP)*it);
		int attributeIndex = dataSet->getConditionalAttributeIndex(rCond.slot("attribute"));
		KnowledgeCondition cond(attributeIndex, -1, rCond.slot("fixed"), rCond.slot("required"));

		if(dataSet->getConditionalAttribute(attributeIndex).getType() == Attribute::NOMINAL)
			cond.setValue(dataSet->getConditionalAttribute(attributeIndex).getDoubleValue(Rcpp::as<string>(rCond.slot("value"))));
		else
		{
			cond.setFrom(Rcpp::as<double>(rCond.slot("from")));
			cond.setTo(Rcpp::as<double>(rCond.slot("to")));
		}

		conditions->getConditions().push_back(cond);
	}
	return conditions;
}

void RInterface::fillListOfRulesWithRRules(std::list<KnowledgeRule>& rules, Rcpp::List& rRules, DataSet* dataSet, int classIndex)
{
	SetOfConditions* conditions;
	for(Rcpp::List::iterator it = rRules.begin(); it != rRules.end(); it++)
	{
		Rcpp::S4 rRule((SEXP)*it);
		Rcpp::S4 rConditions((SEXP)rRule.slot("conditions"));

		conditions = getSetOfConditionsFromRConditions(rConditions, dataSet, classIndex);
		rules.push_back(KnowledgeRule(*conditions));
		delete conditions;
	}
}
