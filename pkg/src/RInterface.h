#ifndef RINTERFACE_H
#define	RINTERFACE_H

#include "Attribute.h"
#include "Example.h"
#include "ElementaryCondition.h"
#include "SetOfExamples.h"
#include "Rule.h"
#include "RuleQualityMeasure.h"
#include "Operator.h"
#include "ConfusionMatrix.h"
#include "SequentialCovering.h"
#include "RuleClassifier.h"
#include "RulesInductionException.h"
#include <map>
#include <Rcpp.h>
#include <limits>
#include <ctime>
#include <cstdlib>

/**
 * RInterface.
 * This class contains methods invoked from R, where it's instantiated.
 * RInterface class uses Rcpp library.
 */
class RInterface {
public:
    Rcpp::List generateRules(std::vector<double> y, std::string yname, std::vector<std::string> ylevels,
            SEXP x, std::vector<std::string> xtypes, std::vector<std::string> xnames, Rcpp::List xlevels,
            std::string rqmPrune, std::string rqmGrow, SEXP rqmPruneCustom, SEXP rqmGrowCustom,
            std::vector<double> weights, double fseed);

    Rcpp::List predict(std::vector<double> y, std::string yname, std::vector<std::string> ylevels,
            SEXP x, std::vector<std::string> xtypes,
            std::vector<std::string> xnames, Rcpp::List xlevels, std::vector<std::string> _serialRules,std::vector<double> confidenceDegrees,
            std::vector<double> weights, double fseed);

    Rcpp::List crossValidation(std::vector<double> y, std::string yname, std::vector<std::string> ylevels,
            SEXP x, std::vector<std::string> xtypes, std::vector<std::string> xnames, Rcpp::List xlevels,
            std::string rqmPrune, std::string rqmGrow, int nfolds, int runs, bool everyClassInFold, SEXP rqmPruneCustom,
            SEXP rqmGrowCustom, std::vector<double> weights, bool useWeightsInPrediction, double fseed);

private:
    DataSet* createDataSet(std::vector<double>& y, std::string& yname, std::vector<std::string>& ylevels,
            SEXP& x, std::vector<std::string>& xtypes,
            std::vector<std::string> xnames, Rcpp::List& xlevels, std::vector<double>& weights);
    Rcpp::List serializeRules(RuleClassifier& rules, SetOfExamples& examples);
    RuleClassifier deserializeRules(std::vector<std::string> _serialRules, std::vector<double>& confidenceDegrees, DataSet& ds);
    double resolveConflict(std::list<Rule*>&);
    RuleQualityMeasure* createRuleQualityMeasure(std::string name, SEXP& customRqm);
};

RCPP_MODULE(crules_mod) {
    using namespace Rcpp;
    class_<RInterface > ("RInterface")
            .constructor()
            .method("generateRules", &RInterface::generateRules, "Generate decision rules using sequential covering")
            .method("predict", &RInterface::predict, "Predict class values")
            .method("crossValidation", &RInterface::crossValidation, "Perform n runs of k-fold cross-validation")
            ;

}

/**
 * Represents custom measure defined by user in R.
 */
class CustomMeasure: public RuleQualityMeasure
{
public:
    CustomMeasure(Rcpp::Function& _rFunction): rFunction(_rFunction) {}
    CustomMeasure(SEXP rFunctionPtr): rFunction(rFunctionPtr) {}
    double EvaluateRuleQualityFromResult(RuleEvaluationResult r) {
        return Rcpp::as<double>(rFunction(Rcpp::Named("P", r.P), Rcpp::Named("p", r.p), Rcpp::Named("N", r.N), Rcpp::Named("n", r.n)));
    }
private:
    Rcpp::Function rFunction;
};
#endif	/* RINTERFACE_H */

