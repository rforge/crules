#include "SetOfExamples.h"

using namespace std;

/**
 * Constructor
 * @param ds DataSet object whichc set of examples will be associated with
 * @param init indicates whether set of examples should be initiated with proper values
 */
SetOfExamples::SetOfExamples(DataSet& ds, bool init) : dataset(&ds)
{
    if (!init) return;
    unsigned int size = dataset->getExamples().size();
    examples.resize(size);
    for (unsigned int i = 0; i < size; i++)
        examples[i] = i;
}

SetOfExamples::SetOfExamples(const SetOfExamples& orig)
{
    examples = orig.examples;
    dataset = orig.dataset;
}

/**
 * Returns reference to the example from the data set
 * @param i index of the example
 * @return reference to the example from the data set
 */
Example& SetOfExamples::operator[](int i)
{
    //if(i < examples.size())	//exception is thrown by vector
    return (*dataset)[examples[i]];
    //else
    //	throw RulesInductionException("Index is out of range");
}

bool SetOfExamples::operator==(const SetOfExamples& toCompare)
{
    if (dataset != toCompare.dataset)
        return false;
    return examples == toCompare.examples;
}

SetOfExamples& SetOfExamples::operator =(const SetOfExamples& orig)
{
    if (&orig == this)
        return *this;
    examples = orig.examples;
    dataset = orig.dataset;
    return *this;
}

/**
 * Operator of the set difference
 * @param second set of examples
 * @return set difference
 */
SetOfExamples SetOfExamples::operator-(SetOfExamples& second)
{
    SetOfExamples result;
    result.dataset = dataset;
    sort(examples.begin(), examples.end());
    sort(second.examples.begin(), second.examples.end());
    set_difference(examples.begin(), examples.end(), second.examples.begin(), second.examples.end(), back_inserter(result.examples));
    return result;
}

string SetOfExamples::toString()
{
    ostringstream oss;
    for (unsigned int i = 0; i < examples.size(); i++)
        oss << (*dataset)[examples[i]].toString() << endl;
    return oss.str();
}

/**
 * Returns set of examples from given class
 * @param decAttVal index of class
 * @return set of examples from given class
 */
SetOfExamples SetOfExamples::getExamplesForDecAtt(double decAttVal)
{
    SetOfExamples result;
    vector<int>::iterator it;
    result.dataset = dataset;
    for (it = examples.begin(); it != examples.end(); it++)
    {
        if (((*dataset)[*it].getDecisionAttribute()) == decAttVal)
            result.examples.push_back(*it);
    }
    return result;
}

/**
 * Returns attribute type for given attribute index
 * @param index attribute index
 * @return attribute type
 */
Attribute::AttributeType SetOfExamples::getAttributeType(int index)
{
    return dataset->getConditionalAttribute(index).getType();
}

/**
 * Adds the example from the other set
 * @param second other set of examples
 * @param index index of the example in the other set
 */
void SetOfExamples::addExample(SetOfExamples& second, int index)
{
    examples.push_back(second.examples[index]);
}

/**
 * Returns vector of decision class indices of the examples
 */
vector<double> SetOfExamples::getDistinctClasses()
{
    set<double> classes;
    vector<int>::iterator it;
    for (it = examples.begin(); it != examples.end(); it++)
        classes.insert((*dataset)[*it].getDecisionAttribute());
    vector<double> result;
    set<double>::iterator itSet;
    for (itSet = classes.begin(); itSet != classes.end(); itSet++)
        result.push_back(*itSet);
    return result;
}

/**
 * Returns number of decision classes in the set of examples
 */
int SetOfExamples::getClassesCount()
{
    return getDistinctClasses().size();
}

/**
 * Returns number of decision classes in the entire data set
 */
int SetOfExamples::getAllClassesCount()
{
    int decAttIndex = dataset->getDecisionAttributeIndex();
    return (dataset->getAttributes()[decAttIndex].getLevels().size());
}

/**
 * Shuffles the set of examples
 */
void SetOfExamples::shuffle()
{
    random_shuffle(examples.begin(), examples.end(), [](ptrdiff_t i) { return rand()%i;});
}

/**
 * Generates "folds" for the purpose of cross validation
 * Distribution of the decision attribute is kept in each fold (stratified sampling)
 * @param nfolds number of folds
 * @param everyClassInFold indicates whether there should be examples from all decision classes in every fold
 * @return vector of nfold set of examples
 */
vector<SetOfExamples> SetOfExamples::createStratifiedFolds(unsigned int nfolds, bool everyClassInFold) throw (RulesInductionException)
{
    if (nfolds > examples.size() || nfolds <= 1)
        throw RulesInductionException("Number of folds cannot be less than two or more than number of examples");
    vector<double> classes = getDistinctClasses();
    vector<SetOfExamples> folds(nfolds, SetOfExamples(*dataset));
    int exNr = 0;
    unsigned int classSize;
    for (unsigned int i = 0; i < classes.size(); i++)
    {
        SetOfExamples exOfClass(getExamplesForDecAtt(classes[i]));
        exOfClass.shuffle();
        classSize = exOfClass.size();
        if(!everyClassInFold || classSize >= nfolds)
            for (unsigned int j = 0; j < classSize; j++)
                folds[exNr++ % nfolds].addExample(exOfClass, j);
        else
            for(unsigned int j = 0; j < nfolds; j++)
                folds[j].addExample(exOfClass, j % classSize);

    }
    return folds;
}

double SetOfExamples::getSumOfWeights()
{
    double sumOfWeights = 0;
    vector<int>::iterator it;
    for (it = examples.begin(); it != examples.end(); it++)
    {
        sumOfWeights += (*dataset)[*it].getWeight();
    }
    return sumOfWeights;
}
