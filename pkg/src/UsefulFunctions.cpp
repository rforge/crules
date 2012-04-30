
#include "UsefulFunctions.h"
#include "DataSet.h"

using namespace std;

DataSet* UsefulFunctions::createDataSetFromFile(string filename)
{
    DataSet* ds = new DataSet();
    vector<Attribute> attributes;
    ifstream file(filename.c_str());
    string line;
    string delimeters = " \t\r";
    while (file.good())
    {
        getline(file, line);
        if(line.size() == 0 || line[0] == '%') continue;
        vector<string> words = splitString(line, delimeters);
        std::transform(words[0].begin(), words[0].end(), words[0].begin(), ::toupper);
        if(words[0] == "@RELATION")
        {
            ds->setName(words[1]);
            break;
        }
    }
    while (file.good())
    {
        getline(file, line);
        if(line.size() == 0 || line[0] == '%') continue;
        vector<string> words = splitString(line, delimeters);
        std::transform(words[0].begin(), words[0].end(), words[0].begin(), ::toupper);
        if(words[0] == "@ATTRIBUTE")
        {
            Attribute att;
            att.setName(words[1]);
            if(words[2][0] == '{')
            {
                string delims = "{}, \"\'\r";
                vector<string> templvls = splitString(words[2], delims);
                vector<string> levels;
                for(unsigned int i = 0; i < templvls.size(); i++)
                    if(templvls[i].empty()) continue;
                    else if (templvls[i][0] == '%') break;
                    else levels.push_back(templvls[i]);

                att.setType(Attribute::NOMINAL);
                att.setLevels(levels);
            }
            else
            {
//                string upper = words[2];
//                std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
//                if(upper == "NUMERIC")
                    att.setType(Attribute::NUMERICAL);
            }
            attributes.push_back(att);
        }
        else if(words[0] == "@DATA") break;
    }
    vector<vector<double> > data;
    data.resize(attributes.size());
    delimeters = ", \"'\t\r";
    while (file.good())
    {
        getline(file, line);
        if(line.size() == 0 || line[0] == '%') continue;
        vector<string> words = splitString(line, delimeters);
        for(unsigned int i = 0; i < attributes.size(); i++)
        {
            data[i].push_back(attributes[i].getDoubleValue(words[i]));
        }
    }

	unsigned int i = 0;
	for(; i < attributes.size(); i++)
		if(attributes[i].getName() == "class" || attributes[i].getName() == "'class'") break;
	ds->setDecisionAttributeIndex(i);
	for(i = 0; i < attributes.size(); i++)
		ds->addAttribute(data[i], attributes[i]);
    return ds;

}


vector<string> UsefulFunctions::splitString(string str, string delimeters)
{
    vector<string> strings;
    char* c_str;
    c_str = new char[str.size()+1];
    strcpy(c_str, str.c_str());
    char* pch;
    pch = strtok(c_str, delimeters.c_str());
    while (pch != NULL)
    {
        strings.push_back(pch);
        pch = strtok(NULL, delimeters.c_str());
    }
    delete []c_str;
    return strings;
}
