#include "Example.h"
using namespace std;

Example::Example() {
    decisionAttribute = -1;
    weight = 1;
}

Example::Example(int noOfAttributes) : attributes(noOfAttributes) {
    decisionAttribute = -1;
    weight = 1;
}

Example::Example(const Example& orig) {
    *this = orig;
}

Example::~Example() {
}

string Example::toString()
{
	ostringstream oss;
	vector<double>::const_iterator it;
    for(it = attributes.begin(); it != attributes.end(); it++)
    {
        oss << *it << '\t';
    }
    oss << "|  " << decisionAttribute;
	return oss.str();
}

ostream& operator<<(ostream& out, const Example& example)
{
    vector<double>::const_iterator it;
    for(it = example.attributes.begin(); it != example.attributes.end(); it++)
    {
        out << *it << '\t';
    }
    out << "|  " << example.decisionAttribute;
    return out;
}

void Example::addAttribute(double value) {
    attributes.push_back(value);
}


double Example::getAttribute(int index) {
    return attributes[index];
}

double Example::operator [](int index) {
    return attributes[index];
}

bool Example::operator ==(const Example& toCompare) const{

	if(decisionAttribute != toCompare.decisionAttribute)
		return false;
    return equal(attributes.begin(), attributes.end(), toCompare.attributes.begin()); 
}



