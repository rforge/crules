#ifndef RULEQUALITYMEASURE_H
#define	RULEQUALITYMEASURE_H
#include "Rule.h"
#include "SetOfExamples.h"
#include "Attribute.h"
#include <cmath>

/**
 * Contains result of the evaluation of the rule
 */
class RuleEvaluationResult {
public:

    RuleEvaluationResult() : P(0), p(0), N(0), n(0) {
    }

    RuleEvaluationResult(double P, double p, double N, double n) : P(P), p(p), N(N), n(n) {
    }
    double P;  /**< positives*/
    double p;  /**< true positives*/
    double N;  /**< negatives*/
    double n;  /**< false negatives*/
};

/**
 * Base class for rule quality measures
 */
class RuleQualityMeasure {
public:
    virtual ~RuleQualityMeasure() {};
    static RuleEvaluationResult EvaluateRule(SetOfExamples&, Rule&);
    static RuleEvaluationResult EvaluateCondition(SetOfExamples& ds, ElementaryCondition& cond, double decClass);
    /**
     * Evaluates rule quality based on RuleEvaluationResult object
     * @param r RuleEvaluationResult object
     * @return evaluation of rule quality
     */
    virtual double EvaluateRuleQualityFromResult(RuleEvaluationResult r){return std::numeric_limits<double>::quiet_NaN();}
    /**
     * Evaluates quality of the rule on set of examples
     * @param ds set of examples
     * @param rule evaluated rule
     * @return value of rule quality
     */
    virtual double EvaluateRuleQuality(SetOfExamples& ds, Rule& rule) {
        return EvaluateRuleQualityFromResult(EvaluateRule(ds, rule));
    }
    /**
     * Evaluates quality of the elementary condition on set of examples
     * @param ds set of examples
     * @param cond evaluated condition
     * @param decClass positive class
     * @return value of condition quality
     */
    virtual double EvaluateConditionQuality(SetOfExamples& ds, ElementaryCondition& cond, double decClass) {
        return EvaluateRuleQualityFromResult(EvaluateCondition(ds, cond, decClass));
    }
    //virtual double EvaluateRuleQuality(SetOfExamples&, ElementaryCondition, double);
    static double Log2(double n) { return log(n) / log(2.0); } //for windows c++ compiler
};

class Precision : public RuleQualityMeasure {
public:
    double EvaluateRuleQualityFromResult(RuleEvaluationResult r) {
        return r.p / (r.p + r.n);
    }
};

class Coverage : public RuleQualityMeasure {
public:
    double EvaluateRuleQualityFromResult(RuleEvaluationResult r) {
        return r.p / r.P;
    }
};


class RSS : public RuleQualityMeasure {
public:
    double EvaluateRuleQualityFromResult(RuleEvaluationResult r) {
        return r.p / r.P - r.n / r.N;
    }
};

class TwoMeasure : public RuleQualityMeasure {
public:
    double EvaluateRuleQualityFromResult(RuleEvaluationResult r) {
        return r.p / (r.p + r.n + 2);
    }
};

class Lift : public RuleQualityMeasure {
public:
    double EvaluateRuleQualityFromResult(RuleEvaluationResult r) {
        return (r.p + 1)*(r.P + r.N) / ((r.p + r.n + 2) * r.P);
    }
};

class LogicalSufficiency : public RuleQualityMeasure {
public:
    double EvaluateRuleQualityFromResult(RuleEvaluationResult r) {
        return (r.p * r.N) / (r.n * r.P);
    }
};

class MutualSupport : public RuleQualityMeasure {
public:
    double EvaluateRuleQualityFromResult(RuleEvaluationResult r) {
        return r.p / (r.n + r. P);
    }
};

class Correlation : public RuleQualityMeasure {
public:
    double EvaluateRuleQualityFromResult(RuleEvaluationResult r) {
        return (r.p * r.N - r.P * r.n) / sqrt(r.P * r.N * (r.p + r.n)*(r.P - r.p + r.N - r.n));
    }
};

class SBayesConfirmation : public RuleQualityMeasure {
public:
    double EvaluateRuleQualityFromResult(RuleEvaluationResult r) {
        return r.p / (r.p + r.n) - (r.P - r.p) / (r.P - r.p + r.N - r.n);
    }
};

class C2 : public RuleQualityMeasure {
public:
    double EvaluateRuleQualityFromResult(RuleEvaluationResult r) {
        double first, second;
        first = ((r.P + r.N)*(r.p / (r.p + r.n)) - r.P) / r.N;
        second = (1 + r.p / r.P) / 2;
        return first * second;
    }
};

class CohenMeasure : public RuleQualityMeasure {
public:
    double EvaluateRuleQualityFromResult(RuleEvaluationResult r) {
        double numerator, denominator;
        numerator = (r.P + r.N)*(r.p / (r.p + r.n)) - r.P;
        denominator = ((r.P + r.N) / 2) * (1 + (r.p / (r.p + r.n)) / (r.p / r.P)) - r.P;
        return numerator / denominator;
    }
};

class C1 : public RuleQualityMeasure {
public:
    double EvaluateRuleQualityFromResult(RuleEvaluationResult r) {
        double first, second;
        first = ((r.P + r.N)*(r.p / (r.p + r.n)) - r.P) / r.N;
        second = (2 + cohen(r)) / 3;
        return first * second;
    }
private:
    double cohen(RuleEvaluationResult r)
    {
        double numerator, denominator;
        numerator = (r.P + r.N)*(r.p / (r.p + r.n)) - r.P;
        denominator = ((r.P + r.N) / 2) * (1 + (r.p / (r.p + r.n)) / (r.p / r.P)) - r.P;
        return numerator / denominator;
    }
};

class NegConditionalEntropy : public RuleQualityMeasure {
public:
    double EvaluateRuleQuality(SetOfExamples& ds, Rule& rule);
    double EvaluateConditionQuality(SetOfExamples& ds, ElementaryCondition& cond, double decClass);
    double Entropy(SetOfExamples& examples);
};

class Cn2 : public RuleQualityMeasure {
public:
    double EvaluateRuleQualityFromResult(RuleEvaluationResult r) {
        double result = 2 * (r.p * log(r.p / ((r.p + r.n) * r.P / (r.P + r.N)))
                + r.n * log((r.n + 1) / ((r.p + r.n) * r.N / (r.P + r.N))));
        if((r.p / (r.p + r.n)) < (r.P / (r.P + r.N)))
                return -result;
        return result;
    }
};

class Gain : public RuleQualityMeasure {
public:
    double EvaluateRuleQualityFromResult(RuleEvaluationResult r) {
        double result = info(r.P, r.N) - info_pn(r);
        if((r.p / (r.p + r.n)) < (r.P / (r.P + r.N)))
                return -result;
        return result;
    }
private:
    double info(double p, double n)
    {
        double sum = p + n;
        return - (p / sum * Log2(p / sum) + n / sum * Log2(n / sum));
    }
    double info_pn(RuleEvaluationResult r)
    {
        return (r.p + r.n) / (r.P + r.N) * info(r.p, r.n)
                + (r.P - r.p + r.N - r.n) / (r.P + r.N) * info(r.P - r.p, r.N - r.n);
    }
};

class Pvalue : public RuleQualityMeasure {
public:
    double ComputePvalue(RuleEvaluationResult r, bool& warning) {
        double suma = 0;
        int upto = !(r.P - r.p < r.n) ? r.n : r.P - r.p;
        for(int k = 0; k < upto; k++)
            suma += prob(r.p + k, r.n - k, r.P, r.N);
        if(r.P > (int)r.P || r.p > (int)r.p || r.N > (int)r.N || r.n > (int)r.n)
            warning = true;
        return suma;
    }
private:
    double prob(double p, double n, double P, double N)
    {
        return exp(
                lgamma(p + n + 1) + lgamma(P + N - p - n + 1) + lgamma(P + 1) + lgamma(N + 1)
                - lgamma(n + 1) - lgamma(p + 1) - lgamma(N - n + 1) - lgamma(P - p + 1) - lgamma(P + N + 1)
                );
    }
};
#endif	/* RULEQUALITYMEASURE_H */

/*
 Mianownik przy gain ratio:
    double temp(RuleEvaluationResult r)
    {
        double p1 = (r.p + r.n) / (r.P + r.N);
        double p2 = (r.P - r.p + r.N - r.n) / (r.P + r.N);
        return - (p1 * Log2(p1) + p2 * Log2(p2));
    }
 */
