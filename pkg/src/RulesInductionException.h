#ifndef RULESINDUCTIONEXCEPTION_H
#define	RULESINDUCTIONEXCEPTION_H
#include <string>

/**
 * Exception class used in program
 */

class RulesInductionException
{
public:
    RulesInductionException() {}
    RulesInductionException(std::string message){ this->message = message; }
    virtual ~RulesInductionException() {}
    void setMessage(std::string message) { this->message = message; }
    std::string getMessage() const { return message; }
private:
    std::string message;
};
#endif	/* RULESINDUCTIONEXCEPTION_H */

