#include "DataSet.h"
using namespace std;

DataSet::DataSet() : decisionAttributeIndex(-1)
{
}

DataSet::DataSet(const DataSet& orig)
{
    copy(orig.examples.begin(), orig.examples.end(), back_inserter(examples));
}

DataSet::~DataSet()
{
}

void DataSet::addAttribute(vector<double> values, Attribute attribute) throw(RulesInductionException)
{
    int index = attributes.size();
    vector<Example>::iterator it;
    vector<double>::iterator itVal;
    if (!examples.empty())
    {
        if (examples.size() != values.size())
            throw RulesInductionException("Inconsistent input data");

        if (index == decisionAttributeIndex)
            for (it = examples.begin(), itVal = values.begin(); it != examples.end(); it++, itVal++)
                it->setDecisionAttribute(*itVal);
        else
            for (it = examples.begin(), itVal = values.begin(); it != examples.end(); it++, itVal++)
                it->addAttribute(*itVal);
    }
    else
    {
        for (itVal = values.begin(); itVal != values.end(); itVal++)
        {
            Example example;
            if (index == decisionAttributeIndex)
                example.setDecisionAttribute(*itVal);
            else
                example.addAttribute(*itVal);
            examples.push_back(example);
        }
    }
    attributes.push_back(attribute);
}

void DataSet::addWeights(vector<double> weights) throw(RulesInductionException)
{
    vector<Example>::iterator it;
    vector<double>::iterator itVal;
    if (!examples.empty())
    {
        if (examples.size() != weights.size())
            throw RulesInductionException("Inconsistent input data");

        for (it = examples.begin(), itVal = weights.begin(); it != examples.end(); it++, itVal++)
            it->setWeight(*itVal);
    }
    else
    {
        for (itVal = weights.begin(); itVal != weights.end(); itVal++)
        {
            examples.push_back(Example());
            examples.back().setWeight(*itVal);
        }
    }
}


string DataSet::printData()
{
    ostringstream oss;
    vector<Example>::iterator lit;
    vector<Attribute>::iterator vit;

    for (vit = attributes.begin(); vit != attributes.end(); vit++)
    {
        oss << vit->getName() << '\t';
    }
    oss << endl;
    for (vit = attributes.begin(); vit != attributes.end(); vit++)
    {
        oss << "------\t";
    }
    oss << endl;

    for (lit = examples.begin(); lit != examples.end(); lit++)
    {
        oss << lit->toString() << endl;
    }

    return oss.str();
}

int DataSet::getConditionalAttributeIndex(std::string name)
{
	unsigned int i = 0;
	while(i < attributes.size() && attributes[i].getName().compare(name) != 0)
		i++;

	if(i >= decisionAttributeIndex)
		i--;

	if(i == attributes.size())
		return -1;
	return i;
}

