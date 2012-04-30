/* 
 * File:   ArffReader.h
 * Author: wojtek
 *
 * Created on December 23, 2011, 10:20 PM
 */

#ifndef USEFULFUNCTIONS_H
#define	USEFULFUNCTIONS_H

#include<vector>
#include<string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include "DataSet.h"

class UsefulFunctions {
public:
    static DataSet* createDataSetFromFile(std::string filename);
    static std::vector<std::string> splitString(std::string str, std::string delimeters);
};

#endif	/* USEFULFUNCTIONS_H */

