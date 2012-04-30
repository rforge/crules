#include "Attribute.h"
using namespace std;

Attribute::~Attribute()
{
}

/**
 * The method returns the index number representing the attribute value decision-making in the vector of its unique value.
 * This allows you to encode the string, which is a number, so that the handling of data is more efficient.
 * @param string newval, being the nominal value of the attribute.
 * @return index number representing the nominal value of an attribute.
 */
double Attribute::getDoubleValue(string newVal)
{
    if (newVal.empty() || newVal == "?" || newVal == "NA") //w przypadku, gdy warto�� jest nieznana
        return numeric_limits<double>::quiet_NaN();
    if (type == NOMINAL)
    {
        for (unsigned int i = 0; i < levels.size(); i++)
        {
            if (newVal == levels[i])
                return i;
        }
        throw RulesInductionException("Could not find value \"" + newVal
                                      + "\" for attribute \"" + name + "\"");
        //        levels.push_back(newVal);
        //        return levels.size() - 1;
    }
    //    if (type == NUMERICAL)
    //    {
    double numValue;
    stringstream myStream(newVal);
    if (myStream >> numValue)
        return numValue;
    throw RulesInductionException("Attribute \"" + name + "\" is of numerical type");

        //        double numValue = atof(newVal.c_str());
        //        return numValue;
        //    }

}

/**
 * This method returns the attribute value corresponding to the nominal specified key.
 * @param key the key (index) value.
 * @return a string containing the actual value of the nominal attribute.
 */
string Attribute::getStringValue(double key)
{
    if (type == NOMINAL)
        return levels[key];
//    else if (type == NUMERICAL)
//    {
        ostringstream oss;
        oss << key;
        return oss.str();
//    }
}

