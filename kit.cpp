//
//  kit.cpp
//  Neo4j-cpp-driver
//
//  Created by skyblue on 2017/7/9.
//  Copyright © 2017年 skyblue. All rights reserved.
//

#include "kit.hpp"

#include <sstream>

namespace neo4jDriver
{
    //Neo4j工具包
    
    std::string Kit::getStatusCode(std::string httpHeader)
    {
        size_t begin = httpHeader.find_first_of(" ");
        std::string temp = httpHeader.substr(begin+1, httpHeader.length());
        size_t end =  temp.find_first_of(" ");
        std::string statusCode = temp.substr(0, end);
        return statusCode;
    };
    
    std::string Kit::getWhereString(std::string fieldName, Json::Value &properties, std::string idFieldName)
    {
        return Kit::append(fieldName, properties, " AND ", idFieldName);
    };
    
    std::string Kit::getWhereString(std::string fieldName, std::string propertiesNamePrefix, Json::Value &properties, std::string idFieldName)
    {
        return Kit::append(fieldName, propertiesNamePrefix, properties, " AND ", idFieldName);
    };
    
    std::string Kit::getSetString(std::string fieldName, Json::Value &properties)
    {
        return Kit::append(fieldName, properties, ",");
    };
    
    std::string Kit::getLabelString(const std::vector<std::string> &labels)
    {
        std::string labelsString = "";
        
        for (int i=0; i < labels.size(); i++)
        {
            if (i+1 < labels.size())
            {
                labelsString += labels[i] + ":";
            }
            else
            {
                labelsString += labels[i];
            }
        }
        
        return labelsString;
    };
    
    unsigned long long int Kit::getNodeOrRelationshipID(std::string nodeOrRelationshipSelf)
    {
        size_t id;
        
        size_t begin = nodeOrRelationshipSelf.find_last_of("/");
        std::string idString = nodeOrRelationshipSelf.substr(begin + 1, nodeOrRelationshipSelf.length());
        
        std::stringstream sstream;
        sstream << idString;
        sstream >> id;
        sstream.clear();
        
        return id;
    };
    
    /*
     * 私有方法
     */
    
    std::string Kit::append(std::string fieldName, Json::Value &properties, std::string appendToken, std::string idFieldName)
    {
        std::string parameters = "";
        bool isFirst = true;
        
        for (Json::ValueIterator i = properties.begin(); i != properties.end(); i++)
        {
            if (isFirst)
            {
                if (idFieldName == "" || (idFieldName != "" && i.name() != idFieldName))
                {
                    parameters += fieldName + "." + i.name() + "={" + i.name() + "}";
                }
                else
                {
                    parameters += "id(" + fieldName + ")={" + i.name() + "}";
                }
                
                isFirst = false;
            }
            else
            {
                if (idFieldName == "" || (idFieldName != "" && i.name() != idFieldName))
                {
                    parameters += appendToken + fieldName + "." + i.name() + "={" + i.name() + "}";
                }
                else
                {
                    parameters += appendToken + "id(" + fieldName + ")={" + i.name() + "}";
                }
            }
        }
        
        return parameters;
    };
    
    std::string Kit::append(std::string fieldName, std::string propertiesNamePrefix, Json::Value &properties, std::string appendToken, std::string idFieldName)
    {
        std::string parameters = "";
        bool isFirst = true;
        
        for (Json::ValueIterator i = properties.begin(); i != properties.end(); i++)
        {
            if (isFirst)
            {
                if (idFieldName == "" || (idFieldName != "" && i.name() != idFieldName))
                {
                    parameters += fieldName + "." + i.name() + "={" + propertiesNamePrefix + i.name() + "}";
                }
                else
                {
                    parameters += "id(" + fieldName + ")={" + propertiesNamePrefix + i.name() + "}";
                }
                
                isFirst = false;
            }
            else
            {
                if (idFieldName == "" || (idFieldName != "" && i.name() != idFieldName))
                {
                    parameters += appendToken + fieldName + "." + i.name() + "={" + propertiesNamePrefix + i.name() + "}";
                }
                else
                {
                    parameters += appendToken + "id(" + fieldName + ")={" + propertiesNamePrefix + i.name() + "}";
                }
            }
        }
        
        return parameters;
    };
}
