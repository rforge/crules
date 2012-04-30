#ifndef EXAMPLE_H
#define	EXAMPLE_H
#include "Attribute.h"
#include <vector>
#include <algorithm>


/**
 * Represents single example (object) in dataset. Vector of attribute values.
 */
class Example {
public:
    Example();
    Example(int);
    Example(const Example& orig);
    virtual ~Example();
    void addAttribute(double);
    double getAttribute(int index);
    double operator[](int index);
    bool operator==(const Example&) const;
    bool operator!=(const Example& toCompare){return !operator==(toCompare);}
    std::string toString();
    friend std::ostream& operator<<(std::ostream& out, const Example& example);

    void setDecisionAttribute(double decisionAttribute){
        this->decisionAttribute = decisionAttribute;
    }
    double getDecisionAttribute() const { return decisionAttribute; }
    void setAttributes(std::vector<double>& attributes) { this->attributes = attributes; }
    std::vector<double>& getAttributes() { return attributes; }
    void setWeight(double weight) { this->weight = weight; }
    double getWeight() const { return weight; }

private:
    std::vector<double> attributes;
    double decisionAttribute;
    double weight;
};

#endif	/* EXAMPLE_H */

