#include "ConfusionMatrix.h"
using namespace std;

ConfusionMatrix::ConfusionMatrix(int numberOfClasses): numberOfClasses(numberOfClasses)
{
    setNumberOfClasses(numberOfClasses);
}

void ConfusionMatrix::setNumberOfClasses(int numberOfClasses)
{
	matrix.resize(numberOfClasses);
	for(int i = 0; i < numberOfClasses; i++)
		matrix[i].resize(numberOfClasses, 0);
        unclassified.resize(numberOfClasses, 0);
}

string ConfusionMatrix::toString()
{
	numberOfClasses = matrix.size();
	ostringstream oss;
	for(int i = 0; i < numberOfClasses; i++)
	{
		for(int j = 0; j < numberOfClasses; j++)
			oss << matrix[i][j] << '\t';
		oss << endl;
	}
	return oss.str();
}


/**
 * Returns sum of weights of all examples
 * @return sum of weights
 */
double ConfusionMatrix::getSumOfExamples()
{
	double weights = 0;
	int size = matrix.size();
	for(int i = 0; i < size; i++)
            weights += accumulate(matrix[i].begin(), matrix[i].end(), 0);
	return weights + getSumOfUncoveredExamples();
}

/**
 * @param classNumber number of decision class
 * @return sum of weights of examples from given class
 */
double ConfusionMatrix::getSumOfExamples(int classNumber)
{
    double weights = 0;
    int size = matrix.size();
    for(int i = 0; i < size; i++)
        weights += matrix[classNumber][i];
    return weights + unclassified[classNumber];
}

/**
 * @return sum of weights of correctly classified examples
 */
double ConfusionMatrix::getSumOfCorrectlyClassifiedExamples()
{
	int weights = 0;
	int size = matrix.size();
	for(int i = 0; i < size; i++)
		weights += matrix[i][i];
	return weights;
}

double ConfusionMatrix::getSumOfTruePositives(int classNumber)
{
	return matrix[classNumber][classNumber];
}

double ConfusionMatrix::getSumOfTrueNegatives(int classNumber)
{
	int weights = 0;
	int size = matrix.size();
	for(int i = 0; i < size; i++)
		for(int j = 0; j < size; j++)
			if((i != classNumber) && (j != classNumber))
				weights += matrix[i][j];
	return weights;
}

double ConfusionMatrix::getSumOfFalsePositives(int classNumber)
{
	int weights = 0;
	int size = matrix.size();
	for(int i = 0; i < size; i++)
		if(i != classNumber)
			weights += matrix[i][classNumber];
	return weights;
}

double ConfusionMatrix::getSumOfFalseNegatives(int classNumber)
{
	int weights = 0;
	int size = matrix.size();
	for(int i = 0; i < size; i++)
		if(i != classNumber)
			weights += matrix[classNumber][i];
	return weights + unclassified[classNumber];   //not sure about this
}

/**
 * @return weights of examples not classified to any of the classes
 */
double ConfusionMatrix::getSumOfUncoveredExamples()
{
    return accumulate(unclassified.begin(), unclassified.end(), 0);
}
