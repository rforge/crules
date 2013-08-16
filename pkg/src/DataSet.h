#ifndef DATASET_H
#define	DATASET_H

#include <list>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <fstream>
#include "Example.h"
#include "RulesInductionException.h"


/**
 * Represents data set. Contains data and information about attributes
 */
class DataSet {
public:
    DataSet();
    DataSet(const DataSet& orig);
    virtual ~DataSet();

    std::vector<Example>& getExamples() { return examples; }
    std::vector<Attribute>& getAttributes() { return attributes; }
    int getDecisionAttributeIndex() { return decisionAttributeIndex; }
    void setDecisionAttributeIndex(int index) { decisionAttributeIndex = index; }
    DataSet& operator=(const DataSet&);
    bool operator==(const DataSet&);
    Example& operator[](int i) { return examples[i]; }
    std::string printData(); //wywaliłbym to
    void addAttribute(std::vector<double> values, Attribute attribute) throw (RulesInductionException);
    void setName(std::string name) { this->name = name; }
    std::string getName() const { return name; }
    void addWeights(std::vector<double> weights) throw (RulesInductionException);
    Attribute& getConditionalAttribute(int index) {return attributes[index < decisionAttributeIndex ? index : index + 1]; }
    Attribute& getDecisionAttribute() {return attributes[decisionAttributeIndex]; }
    int getConditionalAttributeIndex(std::string name);

private:
    std::vector<Example> examples;
    std::vector<Attribute> attributes;
    int decisionAttributeIndex;
    std::string name;
};

#endif	/* DATASET_H */

