#ifndef KNOWLEDGECONDITION_H_
#define KNOWLEDGECONDITION_H_
#include<limits>
#include "Attribute.h"

class KnowledgeCondition {
public:
	KnowledgeCondition() : attributeIndex(-1), from(-std::numeric_limits<double>::max()),
							to(std::numeric_limits<double>::max()), fixed(false), required(false), attributeType(Attribute::NUMERICAL){}

	KnowledgeCondition(int attributeIndex, double from, double to, bool fixed, bool required) : attributeIndex(attributeIndex), from(from),
								to(to), fixed(fixed), required(required), attributeType(Attribute::NUMERICAL){}

	KnowledgeCondition(int attributeIndex, double value, bool fixed, bool required) : attributeIndex(attributeIndex), from(value),
									to(value), fixed(fixed), required(required), attributeType(Attribute::NOMINAL){}

	virtual ~KnowledgeCondition() {}

	int getAttributeIndex() const {
		return attributeIndex;
	}

	void setAttributeIndex(int attributeIndex) {
		this->attributeIndex = attributeIndex;
	}

	double getValue() const {
		return from == to ? from : std::numeric_limits<double>::quiet_NaN();
	}

	void setValue(double value) {
		from = to = value;
	}

	bool isFixed() const {
		return fixed;
	}

	void setFixed(bool fixed) {
		this->fixed = fixed;
	}

	double getFrom() const {
		return from;
	}

	void setFrom(double from) {
		this->from = from;
	}

	bool isRequired() const {
		return required;
	}

	void setRequired(bool required) {
		this->required = required;
	}

	double getTo() const {
		return to;
	}

	void setTo(double to) {
		this->to = to;
	}

	Attribute::AttributeType getAttributeType() const {
			return attributeType;
		}

	void setAttributeType(Attribute::AttributeType attributeType) {
		this->attributeType = attributeType;
	}

private:
	int attributeIndex;
	double from;
	double to;
	bool fixed;
	bool required;
	Attribute::AttributeType attributeType;
};

#endif /* KNOWLEDGECONDITION_H_ */
