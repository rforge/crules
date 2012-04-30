#ifndef SETOFEXAMPLES_H
#define	SETOFEXAMPLES_H

#include <list>
#include <algorithm>
#include <iterator>
#include <vector>
#include <set>
#include <iostream>
#include "Attribute.h"
#include "Example.h"
#include "DataSet.h"
#include "RulesInductionException.h"

/**
 * Represents set of examples and provides methods for operating on it.
 * It doesn't contain examples, but their indices in data set (object of DataSet class).
 * That is why SetOfExamples object has to be associated with DataSet object.
 */
class SetOfExamples {
public:
    SetOfExamples(DataSet& ds, bool init = false);
    SetOfExamples(const SetOfExamples& orig);
    SetOfExamples& operator=(const SetOfExamples&);
    SetOfExamples operator-(SetOfExamples&);
    int size() { return examples.size(); }
    Example& operator[](int i);

    //list<Example>& getExamples() ;
    //vector<double> getDecisionAttributes() ;
    std::vector<double> getDistinctClasses();
    int getClassesCount();
    int getAllClassesCount();
    bool operator==(const SetOfExamples&);
    std::string toString();
    SetOfExamples getExamplesForDecAtt(double);
    Attribute::AttributeType getAttributeType(int);
    DataSet& getDataSet() { return *dataset; }
    void addExample(SetOfExamples& second, int index);
    void removeExample(int index) { examples.erase(examples.begin() + index); }
    void clear() { examples.clear(); }
    void shuffle();
    std::vector<SetOfExamples> createStratifiedFolds(unsigned int nfolds, bool everyClassInFold) throw(RulesInductionException);
    double getSumOfWeights();
private:
    SetOfExamples(){};
    std::vector<int> examples;	//indices in data set
    DataSet* dataset;
};

#endif	/* SETOFEXAMPLES_H */

