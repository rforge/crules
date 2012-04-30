#ifndef CONFUSIONMATRIX_H
#define	CONFUSIONMATRIX_H

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <numeric>


/**
 * Represents confusion matrix and provides useful methods to operate on it
 */
class ConfusionMatrix
{
public:
	ConfusionMatrix(int numberOfClasses);
	std::vector<double>& operator[](int i){ return matrix[i]; }
	int getNumberOfClasses() { return numberOfClasses; }
	void setNumberOfClasses(int numberOfClasses);
	std::string toString();
	void setUnclassified(std::vector<double>& unclassified) { this->unclassified = unclassified; }
	std::vector<double>& getUnclassified() { return unclassified; }
	//with weights:
	double getSumOfExamples();
	double getSumOfExamples(int classNumber);
	double getSumOfCorrectlyClassifiedExamples();
	double getSumOfTruePositives(int classNumber);
	double getSumOfTrueNegatives(int classNumber);
	double getSumOfFalsePositives(int classNumber);
	double getSumOfFalseNegatives(int classNumber);
        double getSumOfUncoveredExamples();
        std::vector<std::vector<double> > getMatrix() const { return matrix; }
private:   
	int numberOfClasses;
	std::vector<std::vector<double> > matrix;	//in rows: actual class, in columns: predicted
	std::vector<double> unclassified;   //indices correspond to number of class
};

#endif	/* CONFUSIONMATRIX_H */
