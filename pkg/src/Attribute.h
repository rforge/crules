#ifndef ATTRIBUTE_H
#define	ATTRIBUTE_H
#include "RulesInductionException.h"
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <limits>


/**
 * Contains information about attribute (which characterizes data in dataset).
 * This is information such as the attribute name, type, and a set of values​​, in the case of nominal attributes.
 * class contains methods for changing the nominal value of an attribute to the number (index) and vice versa.
 */
class Attribute
{

public:
    /**Enum type used to indicate type of attribute
     */
    enum AttributeType
    {
        NUMERICAL, /**< numerical attribute*/
        NOMINAL     /**< nominal attribute*/
    };

    Attribute() {};
    Attribute(std::string _name, AttributeType _type ): name(_name), type(_type) {};
    virtual ~Attribute();
    std::string getName() const { return name; }
    void setName(std::string _name) { name = _name; }
    AttributeType getType() const { return type; }
    void setType(AttributeType at) { type = at; }
    std::vector<std::string>& getLevels() { return levels; }
    void setLevels(std::vector<std::string> levels) { this->levels = levels; }
    double getDoubleValue(std::string);	//returns index for given value
    double getDoubleValue(double val) { return val; }	//does nothing
    std::string getStringValue(double); //returns value for given index
private:
    std::string name;
    AttributeType type;
    std::vector<std::string> levels;
};

#endif	/* ATTRIBUTE_H */

