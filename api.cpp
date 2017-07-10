//
//  api.cpp
//  Neo4j-cpp-driver
//
//  Created by skyblue on 2017/7/9.
//  Copyright © 2017年 skyblue. All rights reserved.
//

#include "api.hpp"

#include <sstream>

#include "./base64.hpp"

#include "./kit.hpp"

namespace neo4jDriver
{
    Neo4jAPI::Neo4jAPI(std::shared_ptr<neo4jDriver::Neo4j> database)
    :database(database), headers(NULL), curl(NULL), responseHeaderString(""), responseString("")
    {
        
    };
    
    Neo4jAPI::Neo4jAPI(std::shared_ptr<neo4jDriver::Neo4j> database, std::string host, std::string port, std::string user, std::string password)
    :database(database), host(host), port(port), user(user), password(password), headers(NULL), curl(NULL), responseHeaderString(""), responseString("")
    {
        
    };
    
    std::string Neo4jAPI::getHost()
    {
        return this->host;
    };
    
    std::string Neo4jAPI::getPort()
    {
        return this->port;
    };
    
    std::string Neo4jAPI::getUser()
    {
        return this->user;
    };
    
    std::string Neo4jAPI::getPassword()
    {
        return this->password;
    };
    
    void Neo4jAPI::connectDatabase()
    {
        this->headers = curl_slist_append(this->headers, "Accept: application/json; charset=UTF-8");
        this->headers = curl_slist_append(this->headers, "Content-Type: application/json");
        this->headers = curl_slist_append(this->headers, "X-Stream: true");
        
        std::string userAndPasswordString = this->getUser() + ":" + this->getPassword(), userAndPasswordEncodeString;
        Base64::Encode(userAndPasswordString, &userAndPasswordEncodeString);
        userAndPasswordEncodeString = "Authorization: Basic " + userAndPasswordEncodeString;
        this->headers = curl_slist_append(this->headers, userAndPasswordEncodeString.c_str());
        
        this->curl = curl_easy_init();
        
        if (this->curl)
        {
            //指定header
            curl_easy_setopt(this->curl, CURLOPT_HTTPHEADER, this->headers);
            //指定接收返回协议头的回调方法
            curl_easy_setopt(this->curl, CURLOPT_HEADERFUNCTION, Neo4jAPI::responseHeaderHandeler);
            //指定接收返回协议头的存储位置
            curl_easy_setopt(this->curl, CURLOPT_HEADERDATA, &this->responseHeaderString);
            //指定接收数据的回调方法
            curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, Neo4jAPI::responseHandeler);
            //指定接收数据的存储位置
            curl_easy_setopt(this->curl, CURLOPT_WRITEDATA, &this->responseString);
            
            //发送请求，测试数据库是否已连接成功
            std::string httpUrlForTransaction = "http://" + host + ":" + port + "/user/neo4j";
            //指定访问点
            curl_easy_setopt(this->curl, CURLOPT_URL, httpUrlForTransaction.c_str());
            //执行POST请求
            CURLcode res = curl_easy_perform(curl);
            //根据执行结果返回执行结果或抛出异常
            if (res != 0)
            {
                //post请求执行失败，抛出异常
                throw "POST REQUEST EXECUTION IS FAILED!";
            }
            
            //对返回数据的协议状态进行检查
            std::string statusCode = Kit::getStatusCode(this->responseHeaderString);
            
            if (statusCode != "200")
            {
                this->responseHeaderString = "";
                this->responseString = "";
                
                throw "RESPONSE STATUS IS NOT 200!";
            }
            
            this->responseHeaderString = "";
            this->responseString = "";
        }
        else
        {
            //curl_easy初始化失败，抛出异常
            throw "CURL EASY INIT IS FAILED!";
        }
    };
    
    void Neo4jAPI::closeDatabase()
    {
        this->responseHeaderString = "";
        this->responseString = "";
        
        if (this->headers != NULL)
        {
            curl_slist_free_all(this->headers);
            this->headers = NULL;
        }
        
        if (this->curl)
        {
            curl_easy_cleanup(this->curl);
            this->curl = NULL;
        }
    };
    
    Json::Value Neo4jAPI::cypherQuery(std::string cypher, Json::Value properties)
    {
        if (this->curl)
        {
            std::string httpUrlForTransaction = "http://" + host + ":" + port + "/db/data/transaction/commit";
            
            //指定访问点
            curl_easy_setopt(this->curl, CURLOPT_URL, httpUrlForTransaction.c_str());
            
            //通过Json构建Cypher语句的查询字符串
            Json::Value statement;
            Json::Value statements;
            Json::Value cypherJson;
            Json::FastWriter writer;
            
            //构建单条statement
            statement["statement"] = cypher;
            
            if (properties != Json::nullValue)
            {
                statement["parameters"] = properties;
            }
            
            //构建statement数组
            statements.append(statement);
            //构建查询对象
            cypherJson["statements"] = statements;
            
            //将Json格式的查询对象输出为字符串
            std::string cypherString = writer.write(cypherJson);
            
            //指定请求方式为POST
            curl_easy_setopt(this->curl, CURLOPT_POST, 1);
            //将Cypher语句填充至POST的数据段中
            curl_easy_setopt(this->curl, CURLOPT_POSTFIELDS, cypherString.c_str());
            curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, "POST");
            
            //执行POST请求
            CURLcode res = curl_easy_perform(curl);
            
            //根据执行结果返回执行结果或抛出异常
            if (res != 0)
            {
                //post请求执行失败，抛出异常
                throw "POST REQUEST EXECUTION IS FAILED!";
            }
            else
            {
                //对返回数据的协议状态进行检查
                std::string statusCode = Kit::getStatusCode(this->responseHeaderString);
                
                if (statusCode != "200")
                {
                    this->responseHeaderString = "";
                    this->responseString = "";
                    
                    throw "RESPONSE STATUS IS NOT 200!";
                }
                
                //成功返回执行结果，对结果进行分析处理
                
                Json::Value responseJson;
                Json::Value results;
                Json::Value errors;
                Json::Reader reader;
                
                //将返回字符串解析为Json对象
                if (reader.parse(this->responseString, responseJson))
                {
                    this->responseHeaderString = "";
                    this->responseString = "";
                    
                    //取出其中的错误部分
                    errors = responseJson["errors"];
                    
                    //判断语句执行是否发生错误
                    if (errors.size() != 0)
                    {
                        //若错误部分不为空，抛出异常
                        throw "CYPHER IS ERROR!";
                    }
                    else
                    {
                        //未发生错误,提取查询结果
                        results = responseJson["results"];
                        
                        //判断查询结果是否为空
                        if (results.size() == 1)
                        {
                            //不为空，返回结果（本方法只支持单Cypher语句的执行，所以若有结果只有一条）
                            return results[0];
                        }
                        else
                        {
                            //为空或多于一条，格式错误，抛出异常
                            throw "RESULT IS ERROR!";
                        }
                    }
                }
                else
                {
                    this->responseHeaderString = "";
                    this->responseString = "";
                    
                    //解析失败，抛出异常
                    throw "RESPONSE FORMAT IS ERROR!";
                }
            }
        }
        else
        {
            throw "CURL EASY INIT IS FAILED!";
        }
    };
    
    Json::Value Neo4jAPI::createNode(const Json::Value &properties, const std::string &label)
    {
        if (this->curl)
        {
            std::string httpUrlForTransaction = "http://" + this->host + ":" + this->port + "/db/data/transaction/commit";
            
            //指定访问点
            curl_easy_setopt(this->curl, CURLOPT_URL, httpUrlForTransaction.c_str());
            
            //通过Json构建Cypher语句的查询字符串
            Json::Value statement;
            Json::Value parameters;
            Json::Value statements;
            Json::Value cypherJson;
            Json::FastWriter writer;
            std::string cypher, cypherString;
            
            if (label != "")
            {
                if (!properties.empty())
                {
                    cypher = "CREATE (data:" + label + " {propertyes}) RETURN data";
                }
                else
                {
                    cypher = "CREATE (data:" + label + ") RETURN data";
                }
            }
            else
            {
                if (!properties.empty())
                {
                    cypher = "CREATE (data {propertyes}) RETURN data";
                }
                else
                {
                    cypher = "CREATE (data) RETURN data";
                }
            }
            
            statement["statement"] = cypher;
            
            parameters["propertyes"] = properties;
            statement["parameters"] = parameters;
            
            statement["resultDataContents"].append("REST");
            
            statements.append(statement);
            
            cypherJson["statements"] = statements;
            cypherString = writer.write(cypherJson);
            
            //指定请求方式为POST
            curl_easy_setopt(this->curl, CURLOPT_POST, 1);
            //将Cypher语句填充至POST的数据段中
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, cypherString.c_str());
            
            //执行POST请求
            CURLcode res = curl_easy_perform(this->curl);
            
            //根据执行结果返回执行结果或抛出异常
            if (res != 0)
            {
                //post请求执行失败，抛出异常
                throw "POST REQUEST ERROR!";
            }
            else
            {
                //对返回数据的协议状态进行检查
                std::string statusCode = Kit::getStatusCode(this->responseHeaderString);
                
                if (statusCode != "200")
                {
                    this->responseHeaderString = "";
                    this->responseString = "";
                    
                    throw "RESPONSE STATUS IS NOT 200!";
                }
                
                //成功返回执行结果，对结果进行分析处理
                
                Json::Value responseJson;
                Json::Value results;
                Json::Value errors;
                Json::Reader reader;
                
                //将返回字符串解析为Json对象
                if (reader.parse(this->responseString, responseJson))
                {
                    this->responseHeaderString = "";
                    this->responseString = "";
                    
                    //取出其中的错误部分
                    errors = responseJson["errors"];
                    
                    //判断语句执行是否发生错误
                    if (errors.size() != 0)
                    {
                        //若错误部分不为空，抛出异常
                        throw "CYPHER IS ERROR!";
                    }
                    else
                    {
                        //未发生错误,提取查询结果
                        results = responseJson["results"];
                        
                        //判断查询结果是否为空
                        if (results.size() == 1)
                        {
                            //不为空，返回结果
                            Json::Value result = results[0];
                            Json::Value data = result["data"];
                            Json::Value rests = data[0]["rest"];
                            Json::Value rest = rests[0];
                            Json::Value node = rest["data"];
                            Json::Value metadata = rest["metadata"];
                            
                            node["_id"] = metadata["id"];
                            node["_labels"] = metadata["labels"];
                            
                            return node;
                        }
                        else
                        {
                            //为空或多于一条，格式错误，抛出异常
                            throw "RESULT IS ERROR!";
                        }
                    }
                }
                else
                {
                    this->responseHeaderString = "";
                    this->responseString = "";
                    
                    //解析失败，抛出异常
                    throw "RESPONSE PARSE IS ERROR!";
                }
            }
        }
        else
        {
            throw "CURL EASY INIT IS FAILED!";
        }
    };
    
    Json::Value Neo4jAPI::createNode(const Json::Value &properties, const std::vector<std::string> &labels)
    {
        return this->createNode(properties, Kit::getLabelString(labels));
    };
    
    bool Neo4jAPI::deleteNode(unsigned long long int nodeID)
    {
        if (this->curl)
        {
            std::stringstream sstream;
            sstream << nodeID;
            std::string nodeIDString;
            sstream >> nodeIDString;
            sstream.clear();
            
            std::string httpUrl = "http://" + this->host + ":" + this->port + "/db/data/node/" + nodeIDString;
            
            //指定访问点
            curl_easy_setopt(this->curl, CURLOPT_URL, httpUrl.c_str());
            //指定请求方式为DELETE
            curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            
            //执行DELETE请求
            CURLcode res = curl_easy_perform(curl);
            
            //根据执行结果返回执行结果或抛出异常
            if (res != 0)
            {
                //post请求执行失败，抛出异常
                throw "DELETE REQUEST EXECUTION IS FAILED!";
            }
            else
            {
                //对返回数据的协议状态进行检查
                std::string statusCode = Kit::getStatusCode(this->responseHeaderString);
                
                this->responseHeaderString = "";
                this->responseString = "";
                
                /*
                 * 删除成功后的状态码和文档描述不一致
                 * 文档中为204，但实际返回200
                 */
                
                if (statusCode == "204" || statusCode == "200")
                {
                    //删除成功
                    return true;
                }
                else if (statusCode == "404")
                {
                    //未找到该点
                    return false;
                }
                else if (statusCode == "409")
                {
                    //该点存在依附于它的边，无法删除
                    return false;
                }
                else
                {
                    //解析失败，抛出异常
                    throw "RESPONSE STATUS IS ERROR!";
                }
            }
        }
        else
        {
            throw "CURL EASY INIT IS FAILED!";
        }
    };
    
    bool Neo4jAPI::deleteNodeAndAllRelationshipsOfTheNode(unsigned long long int nodeID)
    {
        /*
         * 这个版本有问题，可能出现删除了全部边后因为网络问题没有将点删除的问题，健壮性依赖于网络
         * 改进方案，使用事务处理
         */
        
        this->deleteAllRelationshipsOfOneNode(nodeID);
        
        return this->deleteNode(nodeID);
    };
    
    unsigned long long int Neo4jAPI::deleteNodeByLabelAndProperties(std::string label, Json::Value &properties)
    {
        std::string cypher = "";
        std::string whereString = Kit::getWhereString("data", properties);
        
        if (label != "")
        {
            cypher = "MATCH (data:" + label + ")";
        }
        else
        {
            cypher = "MATCH (data)";
        }
        
        if (whereString != "")
        {
            cypher += " WHERE " + whereString;
        }
        
        cypher += " DELETE data RETURN count(data)";
        
        //解析过程是否发生异常严重依赖返回的数据，该怎么看待和处理？
        Json::Value result = this->cypherQuery(cypher, properties);
        Json::Value data = result["data"];
        Json::Value rows = data[0];
        Json::Value row = rows["row"];
        Json::Value count = row[0];
        
        return count.asLargestUInt();
    };
    
    unsigned long long int Neo4jAPI::deleteNodeByLabelsAndProperties(std::vector<std::string> &labels, Json::Value &properties)
    {
        return this->deleteNodeByLabelAndProperties(Kit::getLabelString(labels), properties);
    };
    
    bool Neo4jAPI::getNode(unsigned long long int nodeID, Json::Value &node)
    {
        if (this->curl)
        {
            std::stringstream sstream;
            sstream << nodeID;
            std::string nodeIDString;
            sstream >> nodeIDString;
            sstream.clear();
            
            std::string httpUrl = "http://" + this->host + ":" + this->port + "/db/data/node/" + nodeIDString;
            
            //指定访问点
            curl_easy_setopt(this->curl, CURLOPT_URL, httpUrl.c_str());
            //指定请求方式为GET
            curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, "GET");
            
            //执行GET请求
            CURLcode res = curl_easy_perform(this->curl);
            
            //根据执行结果返回执行结果或抛出异常
            if (res != 0)
            {
                //post请求执行失败，抛出异常
                throw "GET REQUEST EXECUTION IS FAILED!";
            }
            else
            {
                //对返回数据的协议状态进行检查
                std::string statusCode = Kit::getStatusCode(this->responseHeaderString);
                
                if (statusCode == "200")
                {
                    //成功找到该点
                    Json::Reader reader;
                    Json::Value responseJson;
                    Json::Value metaData;
                    
                    reader.parse(this->responseString, responseJson);
                    
                    this->responseHeaderString = "";
                    this->responseString = "";
                    
                    metaData = responseJson["metadata"];
                    
                    node = responseJson["data"];
                    node["_id"] = metaData["id"];
                    node["_labels"] = metaData["labels"];
                    
                    return true;
                }
                else if (statusCode == "404")
                {
                    this->responseHeaderString = "";
                    this->responseString = "";
                    
                    //未找到该点
                    return false;
                }
                else
                {
                    this->responseHeaderString = "";
                    this->responseString = "";
                    
                    //解析失败，抛出异常
                    throw "RESPONSE STATUS IS ERROR!";
                }
            }
        }
        else
        {
            throw "CURL EASY INIT IS FAILED!";
        }
    };
    
    Json::Value Neo4jAPI::selectNodesByLabelAndProperties(std::string label, Json::Value &properties)
    {
        if (this->curl)
        {
            std::string httpUrlForTransaction = "http://" + this->host + ":" + this->port + "/db/data/transaction/commit";
            
            //指定访问点
            curl_easy_setopt(this->curl, CURLOPT_URL, httpUrlForTransaction.c_str());
            
            //通过Json构建Cypher语句的查询字符串
            Json::Value statement;
            Json::Value statements;
            Json::Value root;
            Json::FastWriter writer;
            std::string cypher = "", whereString = Kit::getWhereString("data", properties), rootString;
            
            if (label != "")
            {
                cypher = "MATCH (data:" + label + ")";
            }
            else
            {
                cypher = "MATCH (data)";
            }
            
            if (whereString != "")
            {
                cypher += " WHERE " + whereString;
            }
            
            cypher += " RETURN data";
            
            statement["statement"] = cypher;
            statement["parameters"] = properties;
            statement["resultDataContents"].append("REST");
            
            statements.append(statement);
            
            root["statements"] = statements;
            rootString = writer.write(root);
            
            //指定请求方式为POST
            curl_easy_setopt(this->curl, CURLOPT_POST, 1);
            //将Cypher语句填充至POST的数据段中
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, rootString.c_str());
            curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, "POST");
            
            //执行POST请求
            CURLcode res = curl_easy_perform(curl);
            
            //根据执行结果返回执行结果或抛出异常
            if (res != 0)
            {
                //post请求执行失败，抛出异常
                throw "POST REQUEST EXECUTION IS FAILED!";
            }
            else
            {
                //对返回数据的协议状态进行检查
                std::string statusCode = Kit::getStatusCode(this->responseHeaderString);
                
                if (statusCode != "200")
                {
                    this->responseHeaderString = "";
                    this->responseString = "";
                    
                    throw "RESPONSE STATUS IS ERROR!";
                }
                
                //成功返回执行结果，对结果进行分析处理
                
                Json::Value responseJson;
                Json::Value results;
                Json::Value errors;
                Json::Reader reader;
                
                //将返回字符串解析为Json对象
                if (reader.parse(this->responseString, responseJson))
                {
                    this->responseHeaderString = "";
                    this->responseString = "";
                    
                    //取出其中的错误部分
                    errors = responseJson["errors"];
                    
                    //判断语句执行是否发生错误
                    if (errors.size() != 0)
                    {
                        //若错误部分不为空，抛出异常
                        throw "CYPHER IS ERROR!";
                    }
                    else
                    {
                        //未发生错误,提取查询结果
                        results = responseJson["results"];
                        
                        //判断查询结果是否为空
                        if (results.size() == 1)
                        {
                            //不为空，返回结果
                            Json::Value result = results[0];
                            Json::Value data = result["data"];
                            Json::Value rests;
                            Json::Value rest;
                            Json::Value node;
                            Json::Value metadata;
                            Json::Value nodeList;
                            
                            for (int i = 0; i < data.size(); i++)
                            {
                                rests = data[i]["rest"];
                                rest = rests[0];
                                node = rest["data"];
                                metadata = rest["metadata"];
                                node["_id"] = metadata["id"];
                                node["_labels"] = metadata["labels"];
                                
                                nodeList.append(node);
                            }
                            
                            return nodeList;
                        }
                        else
                        {
                            //为空或多于一条，格式错误，抛出异常
                            throw "RESULT IS ERROR!";
                        }
                    }
                }
                else
                {
                    this->responseHeaderString = "";
                    this->responseString = "";
                    
                    //解析失败，抛出异常
                    throw "RESPONSE PARSE IS ERROR!";
                }
            }
        }
        else
        {
            throw "CURL EASY INIT IS FAILED!";
        }
    };
    
    Json::Value Neo4jAPI::selectNodesByLabelsAndProperties(std::vector<std::string> &labels, Json::Value &properties)
    {
        return this->selectNodesByLabelAndProperties(Kit::getLabelString(labels), properties);
    };
    
    Json::Value Neo4jAPI::selectNodesByAnotherLinkedNode(std::string selectedLabel, Json::Value &selectedProperties, std::string relationType, Json::Value &relationProperties, std::string linkedLabel, Json::Value &linkedProperties)
    {
        //初始化cypher语句为空
        std::string cypher = "";
        
        //初始化查询实体、关联关系和关联实体的where子句为空
        std::string selectedWhereString = "";
        std::string relationWhereString = "";
        std::string linkedWhereString = "";
        
        //初始化查询参数
        Json::Value properties;
        //声明参数枚举器
        Json::ValueIterator property;
        
        //处理查询实体的参数并添加进最终的查询参数对象中
        if (selectedProperties != Json::nullValue)
        {
            for (property = selectedProperties.begin(); property != selectedProperties.end(); property++)
            {
                properties["n_" + property.name()] = selectedProperties[property.name()];
            }
            
            selectedWhereString = Kit::getWhereString("n", "n_", selectedProperties, "_id");
        }
        
        if (relationProperties != Json::nullValue)
        {
            for (property = relationProperties.begin(); property != relationProperties.end(); property++)
            {
                properties["r_" + property.name()] = relationProperties[property.name()];
            }
            
            relationWhereString = Kit::getWhereString("r", "r_", relationProperties, "_id");
        }
        
        if (linkedProperties != Json::nullValue)
        {
            for (property = linkedProperties.begin(); property != linkedProperties.end(); property++)
            {
                properties["m_" + property.name()] = linkedProperties[property.name()];
            }
            
            linkedWhereString = Kit::getWhereString("m", "m_", linkedProperties, "_id");
        }
        
        //拼接cypher语句
        
        if (linkedLabel != "")
        {
            cypher = "MATCH (m:" + linkedLabel + ")";
        }
        else
        {
            cypher = "MATCH (m)";
        }
        
        if (relationType != "")
        {
            cypher += "-[r:" + relationType +"]-";
        }
        else
        {
            cypher += "-[r]-";
        }
        
        if (selectedLabel != "")
        {
            cypher += "(n:" + selectedLabel +")";
        }
        else
        {
            cypher += "(n)";
        }
        
        /*
         *拼接where字句
         */
        
        if (selectedWhereString != "")
        {
            //selectedWhereString不为空时
            cypher += " WHERE " + selectedWhereString;
        }
        
        if (selectedWhereString != "" && relationWhereString != "")
        {
            //selectedWhereString和relationWhereString全部不为空
            cypher += " AND " + relationWhereString;
        }
        else if (relationWhereString != "")
        {
            //selectedWhereString为空且relationWhereString不为空
            cypher += " WHERE " + relationWhereString;
        }
        
        if ((selectedWhereString != "" || relationWhereString != "") && linkedWhereString != "")
        {
            //selectedWhereString和relationWhereString至少一个不为空且linkedWhereString不为空
            cypher += " AND " + linkedWhereString;
        }
        else if (linkedWhereString != "")
        {
            //selectedWhereString和relationWhereString全为空且linkedWhereString不为空
            cypher += " WHERE " + linkedWhereString;
        }
        
        cypher += " RETURN id(n),n";
        
        //解析过程是否发生异常严重依赖返回的数据，该怎么看待和处理？
        Json::Value result = this->cypherQuery(cypher, properties);
        Json::Value data = result["data"];
        
        Json::Value nodeList;
        
        for (int i = 0; i < data.size(); i++)
        {
            Json::Value row = data[i]["row"];
            
            Json::Value node = row[1];
            node["_id"] = row[0].asLargestUInt();
            
            nodeList.append(node);
        }
        
        return nodeList;
    };
    
    Json::Value Neo4jAPI::selectNodesByAnotherLinkedNode(std::string selectedLabel, Json::Value &selectedProperties, std::string relationType, Json::Value &relationProperties, unsigned long long int linkNodeID)
    {
        std::string linkNodeIDString;
        
        std::stringstream sstream;
        sstream << linkNodeID;
        sstream >> linkNodeIDString;
        sstream.clear();
        
        //初始化cypher语句为空
        std::string cypher = "";
        
        //初始化查询实体和关联关系的where子句为空
        std::string selectedWhereString = "";
        std::string relationWhereString = "";
        
        //初始化查询参数
        Json::Value properties;
        //声明参数枚举器
        Json::ValueIterator property;
        
        //处理查询实体的参数并添加进最终的查询参数对象中
        if (selectedProperties != Json::nullValue)
        {
            for (property = selectedProperties.begin(); property != selectedProperties.end(); property++)
            {
                properties["n_" + property.name()] = selectedProperties[property.name()];
            }
            
            selectedWhereString = Kit::getWhereString("n", "n_", selectedProperties, "_id");
        }
        
        if (relationProperties != Json::nullValue)
        {
            for (property = relationProperties.begin(); property != relationProperties.end(); property++)
            {
                properties["r_" + property.name()] = relationProperties[property.name()];
            }
            
            relationWhereString = Kit::getWhereString("r", "r_", relationProperties, "_id");
        }
        
        //拼接cypher语句
        
        cypher = "MATCH (m)";
        
        if (relationType != "")
        {
            cypher += "-[r:" + relationType +"]-";
        }
        else
        {
            cypher += "-[r]-";
        }
        
        if (selectedLabel != "")
        {
            cypher += "(n:" + selectedLabel +")";
        }
        else
        {
            cypher += "(n)";
        }
        
        /*
         *拼接where字句
         */
        
        if (selectedWhereString != "")
        {
            //selectedWhereString不为空时
            cypher += " WHERE " + selectedWhereString;
        }
        
        if (selectedWhereString != "" && relationWhereString != "")
        {
            //selectedWhereString和relationWhereString全部不为空
            cypher += " AND " + relationWhereString;
        }
        else if (relationWhereString != "")
        {
            //selectedWhereString为空且relationWhereString不为空
            cypher += " WHERE " + relationWhereString;
        }
        
        if (selectedWhereString != "" || relationWhereString != "")
        {
            //selectedWhereString和relationWhereString至少一个不为空
            cypher += " AND id(m)=" + linkNodeIDString;
        }
        else
        {
            //selectedWhereString和relationWhereString全为空
            cypher += " WHERE id(m)=" + linkNodeIDString;
        }
        
        cypher += " RETURN id(n),n";
        
        //解析过程是否发生异常严重依赖返回的数据，该怎么看待和处理？
        Json::Value result = this->cypherQuery(cypher, properties);
        Json::Value data = result["data"];
        
        Json::Value nodeList;
        
        for (int i = 0; i < data.size(); i++)
        {
            Json::Value row = data[i]["row"];
            
            Json::Value node = row[1];
            node["_id"] = row[0].asLargestUInt();
            
            nodeList.append(node);
        }
        
        return nodeList;
    };
    
    bool Neo4jAPI::updateNodeByID(unsigned long long int nodeID, Json::Value &properties)
    {
        std::string cypher = "START data=node({_id}) SET " + Kit::getSetString("data", properties) + " RETURN data";
        
        properties["_id"] = nodeID;
        
        //解析过程是否发生异常严重依赖返回的数据，该怎么看待和处理？
        Json::Value result = this->cypherQuery(cypher, properties);
        Json::Value data = result["data"];
        Json::Value rows = data[0];
        Json::Value count = rows.size();
        
        if (count.asLargestUInt() == 1)
        {
            return true;
        }
        else
        {
            return false;
        }
    };
    
    bool Neo4jAPI::replaceNodeByID(unsigned long long int nodeID, Json::Value &properties)
    {
        if (this->curl)
        {
            std::stringstream sstream;
            sstream << nodeID;
            std::string nodeIDString;
            sstream >> nodeIDString;
            sstream.clear();
            
            std::string httpUrl = "http://" + this->host + ":" + this->port + "/db/data/node/" + nodeIDString + "/properties";
            
            //指定访问点
            curl_easy_setopt(this->curl, CURLOPT_URL, httpUrl.c_str());
            
            Json::FastWriter writer;
            
            std::string propertiesString = writer.write(properties);
            
            //指定请求方式为PUT
            curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, "PUT");
            
            /*
             * 指定请求方式为PUT，但却将数据放进了POSTFIELDS，这样是稳定的吗？
             */
            
            //将Cypher语句填充至POST的数据段中
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, propertiesString.c_str());
            
            //执行PUT请求
            CURLcode res = curl_easy_perform(curl);
            
            //根据执行结果返回执行结果或抛出异常
            if (res != 0)
            {
                //post请求执行失败，抛出异常
                throw "POST REQUEST EXECUTION IS FAILED!";
            }
            else
            {
                //对返回数据的协议状态进行检查
                std::string statusCode = Kit::getStatusCode(this->responseHeaderString);
                
                this->responseHeaderString = "";
                this->responseString = "";
                
                if (statusCode == "204")
                {
                    return true;
                }
                else if (statusCode == "404")
                {
                    return false;
                }
                else
                {
                    //解析失败，抛出异常
                    throw "RESPONSE STATUS IS ERROR!";
                }
            }
        }
        else
        {
            throw "CURL EASY INIT IS FAILED!";
        }
    };
    
    bool Neo4jAPI::insertRelationship(unsigned long long int rootNodeID, unsigned long long int otherNodeID, std::string typeName, Json::Value &properties, Json::Value &relationship)
    {
        if (this->curl)
        {
            std::stringstream sstream;
            sstream << rootNodeID;
            std::string rootNodeIDString;
            sstream >> rootNodeIDString;
            sstream.clear();
            
            std::string httpUrl = "http://" + this->host + ":" + this->port + "/db/data/node/" + rootNodeIDString + "/relationships";
            
            //指定访问点
            curl_easy_setopt(this->curl, CURLOPT_URL, httpUrl.c_str());
            
            Json::Value root;
            Json::FastWriter writer;
            
            sstream << otherNodeID;
            std::string otherNodeIDString;
            sstream >> otherNodeIDString;
            sstream.clear();
            
            root["to"] = "http://" + this->host + ":" + this->port + "/db/data/node/" + otherNodeIDString;
            root["type"] = typeName;
            root["data"] = properties;
            
            std::string rootString = writer.write(root);
            
            //指定请求方式为POST
            curl_easy_setopt(this->curl, CURLOPT_POST, 1);
            //将Cypher语句填充至POST的数据段中
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, rootString.c_str());
            
            //执行DELETE请求
            CURLcode res = curl_easy_perform(curl);
            
            //根据执行结果返回执行结果或抛出异常
            if (res != 0)
            {
                //post请求执行失败，抛出异常
                throw "POST REQUEST EXECUTION IS FAILED!";
            }
            else
            {
                //对返回数据的协议状态进行检查
                std::string statusCode = Kit::getStatusCode(this->responseHeaderString);
                
                if (statusCode == "201")
                {
                    //添加成功
                    Json::Reader reader;
                    Json::Value responseJson;
                    Json::Value metaData;
                    
                    reader.parse(this->responseString, responseJson);
                    
                    this->responseHeaderString = "";
                    this->responseString = "";
                    
                    metaData = responseJson["metadata"];
                    
                    relationship = responseJson["data"];
                    relationship["_id"] = metaData["id"];
                    relationship["_type"] = metaData["type"];
                    
                    return true;
                }
                else if (statusCode == "400")
                {
                    this->responseHeaderString = "";
                    this->responseString = "";
                    
                    //终端节点未找到
                    relationship = Json::nullValue;
                    
                    return false;
                }
                else if (statusCode == "404")
                {
                    this->responseHeaderString = "";
                    this->responseString = "";
                    
                    //始端节点未找到
                    relationship = Json::nullValue;
                    
                    return false;
                }
                else
                {
                    this->responseHeaderString = "";
                    this->responseString = "";
                    
                    //解析失败，抛出异常
                    throw "RESPONSE STATUS IS ERROR!";
                }
            }
        }
        else
        {
            throw "CURL EASY INIT IS FAILED!";
        }
    };
    
    bool Neo4jAPI::insertRelationship(unsigned long long int rootNodeID, unsigned long long int otherNodeID, std::string typeName, Json::Value &relationship)
    {
        Json::Value properties;
        return Neo4jAPI::insertRelationship(rootNodeID, otherNodeID, typeName, properties, relationship);
    };
    
    bool Neo4jAPI::insertRelationship(unsigned long long int rootNodeID, unsigned long long int otherNodeID, std::string typeName)
    {
        Json::Value properties;
        Json::Value relationship;
        return Neo4jAPI::insertRelationship(rootNodeID, otherNodeID, typeName, properties, relationship);
    };
    
    bool Neo4jAPI::deleteRelationship(unsigned long long int relationshipID)
    {
        if (this->curl)
        {
            std::stringstream sstream;
            sstream << relationshipID;
            std::string relationshipIDString;
            sstream >> relationshipIDString;
            sstream.clear();
            
            std::string httpUrl = "http://" + this->host + ":" + this->port + "/db/data/relationship/" + relationshipIDString;
            
            //指定访问点
            curl_easy_setopt(this->curl, CURLOPT_URL, httpUrl.c_str());
            //指定请求方式为DELETE
            curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            
            //执行DELETE请求
            CURLcode res = curl_easy_perform(curl);
            
            //根据执行结果返回执行结果或抛出异常
            if (res != 0)
            {
                //post请求执行失败，抛出异常
                throw "DELETE REQUEST EXECUTION IS FAILED!";
            }
            else
            {
                //对返回数据的协议状态进行检查
                std::string statusCode = Kit::getStatusCode(this->responseHeaderString);
                
                this->responseHeaderString = "";
                this->responseString = "";
                
                /*
                 * 此处与文档不同，文档中删除关系成功后返回204，但实际返回的却是200
                 */
                
                if (statusCode == "204" || statusCode == "200")
                {
                    //删除成功
                    return true;
                }
                else if (statusCode == "404")
                {
                    //未找到该关系
                    return false;
                }
                else
                {
                    //解析失败，抛出异常
                    throw "RESPONSE STATUS IS ERROR!";
                }
            }
        }
        else
        {
            throw "CURL EASY INIT IS FAILED!";
        }
    };
    
    unsigned long long int Neo4jAPI::deleteRelationshipsOfOneNode(unsigned long long int nodeID, std::string type)
    {
        Json::Value relationshipList;
        Json::Value relationship;
        int count = 0;
        
        if (type == "in")
        {
            relationshipList = this->getRelationshipsOfOneNode(nodeID, "in");
        }
        else if (type == "out")
        {
            relationshipList = this->getRelationshipsOfOneNode(nodeID, "out");
        }
        else
        {
            relationshipList = this->getRelationshipsOfOneNode(nodeID, "all");
        }
        
        for (int i = 0; i<relationshipList.size(); i++)
        {
            relationship = relationshipList[i];
            
            if (this->deleteRelationship(relationship["_id"].asLargestUInt()))
            {
                count++;
            }
        }
        
        return count;
    };
    
    unsigned long long int Neo4jAPI::deleteAllRelationshipsOfOneNode(unsigned long long int nodeID)
    {
        return this->deleteRelationshipsOfOneNode(nodeID, "all");
    };
    
    unsigned long long int Neo4jAPI::deleteAllOutgoingRelationshipsOfOneNode(unsigned long long int nodeID)
    {
        return this->deleteRelationshipsOfOneNode(nodeID, "out");
    };
    
    unsigned long long int Neo4jAPI::deleteAllIncomingRelationshipsOfOneNode(unsigned long long int nodeID)
    {
        return this->deleteRelationshipsOfOneNode(nodeID, "in");
    };
    
    bool Neo4jAPI::getRelationship(unsigned long long int relationshipID, Json::Value &relationship)
    {
        if (this->curl)
        {
            std::stringstream sstream;
            sstream << relationshipID;
            std::string relationshipIDString;
            sstream >> relationshipIDString;
            sstream.clear();
            
            std::string httpUrl = "http://" + this->host + ":" + this->port + "/db/data/relationship/" + relationshipIDString;
            
            //指定访问点
            curl_easy_setopt(this->curl, CURLOPT_URL, httpUrl.c_str());
            
            //执行GET请求
            CURLcode res = curl_easy_perform(curl);
            
            //根据执行结果返回执行结果或抛出异常
            if (res != 0)
            {
                //post请求执行失败，抛出异常
                throw "GET REQUEST EXECUTION IS FAILED!";
            }
            else
            {
                //对返回数据的协议状态进行检查
                std::string statusCode = Kit::getStatusCode(this->responseHeaderString);
                
                if (statusCode == "200")
                {
                    //成功获得该关系
                    //添加成功
                    Json::Reader reader;
                    Json::Value responseJson;
                    Json::Value metaData;
                    
                    reader.parse(this->responseString, responseJson);
                    
                    this->responseHeaderString = "";
                    this->responseString = "";
                    
                    metaData = responseJson["metadata"];
                    
                    relationship = responseJson["data"];
                    relationship["_id"] = metaData["id"];
                    relationship["_type"] = metaData["type"];
                    relationship["_start"] = Kit::getNodeOrRelationshipID(responseJson["start"].asString());
                    relationship["_end"] = Kit::getNodeOrRelationshipID(responseJson["end"].asString());
                    
                    return true;
                }
                else if (statusCode == "404")
                {
                    this->responseHeaderString = "";
                    this->responseString = "";
                    
                    //未找到该关系
                    return false;
                }
                else
                {
                    this->responseHeaderString = "";
                    this->responseString = "";
                    
                    //解析失败，抛出异常
                    throw "RESPONSE PARSE IS ERROR!";
                }
            }
        }
        else
        {
            throw "CURL EASY INIT IS FAILED!";
        }
    };
    
    Json::Value Neo4jAPI::getRelationshipsOfOneNode(unsigned long long int nodeID, std::string type)
    {
        if (this->curl)
        {
            if (type != "all" && type != "in" && type != "out")
            {
                type = "all";
            }
            
            std::stringstream sstream;
            sstream << nodeID;
            std::string nodeIDString;
            sstream >> nodeIDString;
            sstream.clear();
            
            std::string httpUrl = "http://" + this->host + ":" + this->port + "/db/data/node/" + nodeIDString + "/relationships/" + type;
            
            //指定请求方式为GET
            //若不设置该项，默认执行什么方法？？？？
            curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, "GET");
            
            //指定访问点
            curl_easy_setopt(this->curl, CURLOPT_URL, httpUrl.c_str());
            
            //执行GET请求
            CURLcode res = curl_easy_perform(curl);
            
            //根据执行结果返回执行结果或抛出异常
            if (res != 0)
            {
                //post请求执行失败，抛出异常
                throw "GET  REQUEST EXECUTION IS FAILED!";
            }
            else
            {
                //对返回数据的协议状态进行检查
                std::string statusCode = Kit::getStatusCode(this->responseHeaderString);
                
                if (statusCode == "200")
                {
                    //获得成功
                    Json::Reader reader;
                    Json::Value relationshipInfoList;
                    Json::Value relationshipInfo;
                    Json::Value relationshipList;
                    Json::Value relationship;
                    
                    Json::Value metadata;
                    
                    reader.parse(this->responseString, relationshipInfoList);
                    
                    this->responseHeaderString = "";
                    this->responseString = "";
                    
                    for (int i = 0; i < relationshipInfoList.size(); i++)
                    {
                        relationshipInfo = relationshipInfoList[i];
                        
                        metadata = relationshipInfo["metadata"];
                        
                        relationship = relationshipInfo["data"];
                        relationship["_id"] = metadata["id"];
                        relationship["_type"] = metadata["type"];
                        relationship["_start"] = Kit::getNodeOrRelationshipID(relationshipInfo["start"].asString());
                        relationship["_end"] = Kit::getNodeOrRelationshipID(relationshipInfo["end"].asString());
                        
                        relationshipList.append(relationship);
                    }
                    
                    return relationshipList;
                }
                else
                {
                    this->responseHeaderString = "";
                    this->responseString = "";
                    
                    //解析失败，抛出异常
                    throw "RESPONSE PARSE IS ERROR!";
                }
            }
        }
        else
        {
            throw "CURL EASY INIT IS FAILED!";
        }
    };
    
    Json::Value Neo4jAPI::getAllRelationshipsOfOneNode(unsigned long long int nodeID)
    {
        return this->getRelationshipsOfOneNode(nodeID, "all");
    };
    
    Json::Value Neo4jAPI::getAllOutgoingRelationshipsOfOneNode(unsigned long long int nodeID)
    {
        return this->getRelationshipsOfOneNode(nodeID, "out");
    };
    
    Json::Value Neo4jAPI::getAllIncomingRelationshipsOfOneNode(unsigned long long int nodeID)
    {
        return this->getRelationshipsOfOneNode(nodeID, "in");
    };
    
    //    Json::Value Neo4jAPI::selectRelationshipsByTypeAndProperties(std::string type, Json::Value &properties)
    //    {
    //        if (this->curl)
    //        {
    //            std::string httpUrlForTransaction = "http://" + this->host + ":" + this->port + "/db/data/transaction/commit";
    //
    //            //指定访问点
    //            curl_easy_setopt(this->curl, CURLOPT_URL, httpUrlForTransaction.c_str());
    //
    //            //通过Json构建Cypher语句的查询字符串
    //            Json::Value statement;
    //            Json::Value statements;
    //            Json::Value root;
    //            Json::FastWriter writer;
    //            std::string cypher = "", whereString = Neo4jDatabaseInterfaceKit::getWhereString("data", properties), rootString;
    //
    //            if (label != "")
    //            {
    //                cypher = "MATCH (data:" + label + ")";
    //            }
    //            else
    //            {
    //                cypher = "MATCH (data)";
    //            }
    //
    //            if (whereString != "")
    //            {
    //                cypher += " WHERE " + whereString;
    //            }
    //
    //            cypher += " RETURN data";
    //
    //            statement["statement"] = cypher;
    //            statement["parameters"] = properties;
    //            statement["resultDataContents"].append("REST");
    //
    //            statements.append(statement);
    //
    //            root["statements"] = statements;
    //            rootString = writer.write(root);
    //
    //            //指定请求方式为POST
    //            curl_easy_setopt(this->curl, CURLOPT_POST, 1);
    //            //将Cypher语句填充至POST的数据段中
    //            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, rootString.c_str());
    //
    //            //执行POST请求
    //            CURLcode res = curl_easy_perform(curl);
    //
    //            //根据执行结果返回执行结果或抛出异常
    //            if (res != 0)
    //            {
    //                //post请求执行失败，抛出异常
    //                throw exception::DatabaseException(4, this->getHost(), this->getPort(),  this->getUser(), this->getPassword(), "");
    //            }
    //            else
    //            {
    //                //对返回数据的协议状态进行检查
    //                std::string statusCode = Neo4jDatabaseInterfaceKit::getStatusCode(this->responseHeaderString);
    //
    //                if (statusCode != "200")
    //                {
    //                    this->responseHeaderString = "";
    //                    this->responseString = "";
    //
    //                    throw exception::DatabaseException(5, this->getHost(), this->getPort(),  this->getUser(), this->getPassword(), "");
    //                }
    //
    //                //成功返回执行结果，对结果进行分析处理
    //
    //                Json::Value responseJson;
    //                Json::Value results;
    //                Json::Value errors;
    //                Json::Reader reader;
    //
    //                //将返回字符串解析为Json对象
    //                if (reader.parse(this->responseString, responseJson))
    //                {
    //                    this->responseHeaderString = "";
    //                    this->responseString = "";
    
    //                    //取出其中的错误部分
    //                    errors = responseJson["errors"];
    //
    //                    //判断语句执行是否发生错误
    //                    if (errors.size() != 0)
    //                    {
    //                        //若错误部分不为空，抛出异常
    //                        throw exception::DatabaseException(42, this->getHost(), this->getPort(),  this->getUser(), this->getPassword(), "", errors[0].toStyledString());
    //                    }
    //                    else
    //                    {
    //                        //未发生错误,提取查询结果
    //                        results = responseJson["results"];
    //
    //                        //判断查询结果是否为空
    //                        if (results.size() == 1)
    //                        {
    //                            //不为空，返回结果
    //                            Json::Value result = results[0];
    //                            Json::Value data = result["data"];
    //                            Json::Value rests;
    //                            Json::Value rest;
    //                            Json::Value node;
    //                            Json::Value metadata;
    //                            Json::Value nodeList;
    //
    //                            for (int i = 0; i < data.size(); i++)
    //                            {
    //                                rests = data[i]["rest"];
    //                                rest = rests[0];
    //                                node = rest["data"];
    //                                metadata = rest["metadata"];
    //                                node["_id"] = metadata["id"];
    //                                node["_labels"] = metadata["labels"];
    //
    //                                nodeList.append(node);
    //                            }
    //
    //                            return nodeList;
    //                        }
    //                        else
    //                        {
    //                            //为空或多于一条，格式错误，抛出异常
    //                            throw exception::DatabaseException(41, this->getHost(), this->getPort(),  this->getUser(), this->getPassword(), "");
    //                        }
    //                    }
    //                }
    //                else
    //                {
    //                    this->responseHeaderString = "";
    //                    this->responseString = "";
    //
    //                    //解析失败，抛出异常
    //                    throw exception::DatabaseException(40, this->getHost(), this->getPort(),  this->getUser(), this->getPassword(), "");
    //                }
    //            }
    //        }
    //        else
    //        {
    //            throw "CURL EASY INIT IS FAILED!";
    //        }
    //
    //    };
    
    Json::Value Neo4jAPI::getRelationsBetweenTwoNodes(unsigned long long int fromNodeID, unsigned long long int toNodeID, std::string type)
    {
        std::stringstream sstream;
        std::string fromNodeIDString, toNodeIDString;
        sstream << fromNodeID;
        sstream >> fromNodeIDString;
        sstream.clear();
        sstream << toNodeID;
        sstream >> toNodeIDString;
        sstream.clear();
        
        std::string cypher;
        
        if (type == "")
        {
            cypher = "MATCH (m)-[data]->(n) WHERE id(m)=" + fromNodeIDString + " AND id(n)=" + toNodeIDString +" RETURN id(data), type(data), data";
        }
        else
        {
            cypher = "MATCH (m)-[data]->(n) WHERE id(m)=" + fromNodeIDString + " AND id(n)=" + toNodeIDString + " AND type(data)=" + type + " RETURN id(data), type(data), data";
        }
        //解析过程是否发生异常严重依赖返回的数据，该怎么看待和处理？
        Json::Value result = this->cypherQuery(cypher);
        Json::Value data = result["data"];
        Json::Value relations;
        
        for (auto i = 0; i < data.size(); i++)
        {
            Json::Value relation, row = data[0]["row"];
            
            relation["_id"] = row[0];
            relation["_type"] = row[1];
            relation["properties"] = row[2];
            
            relations[i] = relation;
        }
        
        return relations;
    };
    
    bool Neo4jAPI::replaceRelationshipProperties(unsigned long long int relationshipID, Json::Value &properties)
    {
        if (this->curl)
        {
            std::stringstream sstream;
            sstream << relationshipID;
            std::string relationshipIDString;
            sstream >> relationshipIDString;
            sstream.clear();
            
            std::string httpUrl = "http://" + this->host + ":" + this->port + "/db/data/relationship/" + relationshipIDString + "/properties";
            
            //指定访问点
            curl_easy_setopt(this->curl, CURLOPT_URL, httpUrl.c_str());
            
            Json::FastWriter writer;
            
            std::string propertiesString = writer.write(properties);
            
            //指定请求方式为PUT
            curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, "PUT");
            
            /*
             * 指定请求方式为PUT，但却将数据放进了POSTFIELDS，这样是稳定的吗？
             */
            
            //将Cypher语句填充至POST的数据段中
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, propertiesString.c_str());
            
            //执行PUT请求
            CURLcode res = curl_easy_perform(curl);
            
            //恢复请求方式
            curl_easy_setopt(this->curl, CURLOPT_CUSTOMREQUEST, NULL);
            
            //根据执行结果返回执行结果或抛出异常
            if (res != 0)
            {
                //post请求执行失败，抛出异常
                throw "PUT REQUEST EXECUTION IS FAILED!";
            }
            else
            {
                //对返回数据的协议状态进行检查
                std::string statusCode = Kit::getStatusCode(this->responseHeaderString);
                
                this->responseHeaderString = "";
                this->responseString = "";
                
                if (statusCode == "204")
                {
                    return true;
                }
                else if (statusCode == "404")
                {
                    return false;
                }
                else
                {
                    //解析失败，抛出异常
                    throw "RESPONSE STATUS IS ERROR!";
                }
            }
        }
        else
        {
            throw "CURL EASY INIT IS FAILED!";
        }
    };
    
    Neo4jAPI::~Neo4jAPI()
    {
        //清理headers和curl
        if (this->headers != NULL)
        {
            curl_slist_free_all(this->headers);
            this->headers = NULL;
        }
        if (this->curl)
        {
            curl_easy_cleanup(this->curl);
            this->curl = NULL;
        }
        
        //删除该数据库接口占用的数据库驱动
        this->database->deleteNeo4j();
    };
    
    /*
     * 私有方法
     */
    
    size_t Neo4jAPI::responseHeaderHandeler(char *buffer, size_t size, size_t nitems, void *userdata)
    {
        //计算本次收到的数据长度
        size_t responseHeaderDataLength = size * nitems;
        //根据算出的数据长度建立字节数组用于接收本次数据
        char* responseHeaderData = new char[responseHeaderDataLength];
        
        //若字节数组建立失败，返回0触发接受失败，停止接受
        if(NULL == responseHeaderData)
        {
            return 0;
        }
        
        //获取最终数据存放的位置
        std::string* responseHeaderDataPtr = (std::string*)userdata;
        
        //接收本次数据
        memcpy(responseHeaderData, buffer, responseHeaderDataLength);
        
        //将本次接收到的数据存储至最终的数据位置
        *responseHeaderDataPtr  = *responseHeaderDataPtr + std::string(responseHeaderData, responseHeaderDataLength);
        
        //释放用于存储本次接收到的数据的字节数组
        delete[] responseHeaderData;
        
        //返回本次接收到的数据的长度
        return responseHeaderDataLength;
    };
    
    size_t Neo4jAPI::responseHandeler(char *ptr, size_t size, size_t nmemb, void *userdata)
    {
        //计算本次收到的数据长度
        size_t responseDataLength = size * nmemb;
        //根据算出的数据长度建立字节数组用于接收本次数据
        char* responseData = new char[responseDataLength];
        
        //若字节数组建立失败，返回0触发接受失败，停止接受
        if(NULL == responseData)
        {
            return 0;
        }
        
        //获取最终数据存放的位置
        std::string* responseDataPtr = (std::string*)userdata;
        
        //接收本次数据
        memcpy(responseData, ptr, responseDataLength);
        
        //将本次接收到的数据存储至最终的数据位置
        *responseDataPtr  = *responseDataPtr + std::string(responseData, responseDataLength);
        
        //释放用于存储本次接收到的数据的字节数组
        delete[] responseData;
        
        //返回本次接收到的数据的长度
        return responseDataLength;
    };
}
