//
//  kit.hpp
//  Neo4j-cpp-driver
//
//  Created by skyblue on 2017/7/9.
//  Copyright © 2017年 skyblue. All rights reserved.
//

#ifndef kit_hpp
#define kit_hpp

#include <iostream>
#include <string>

#include <json/json.h>

namespace neo4jDriver
{
    class Kit
    {
    public:
        
        /*
         * 解析出HTTP协议头中的状态码
         */
        static std::string getStatusCode(std::string httpHeader);
        
        /*
         * 根据字段名和属性数组生成条件字符串
         */
        static std::string getWhereString(std::string fieldName, Json::Value &properties, std::string idFieldName = "");
        
        static std::string getWhereString(std::string fieldName, std::string propertiesNamePrefix, Json::Value &properties, std::string idFieldName = "");
        
        /*
         * 根据字段名和属性数组生成设置字符串
         */
        static std::string getSetString(std::string fieldName, Json::Value &properties);
        
        /*
         * 根据label数组生成label字符串
         */
        static std::string getLabelString(const std::vector<std::string> &labels);
        
        static unsigned long long int getNodeOrRelationshipID(std::string nodeOrRelationshipSelf);
        
    private:
        
        static std::string append(std::string fieldName, Json::Value &properties, std::string appendToken, std::string idFieldName = "");
        
        static std::string append(std::string fieldName, std::string propertiesNamePrefix, Json::Value &properties, std::string appendToken, std::string idFieldName = "");
    };
}

#endif /* kit_hpp */
