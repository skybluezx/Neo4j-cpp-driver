//
//  api.hpp
//  Neo4j-cpp-driver
//
//  Created by skyblue on 2017/7/9.
//  Copyright © 2017年 skyblue. All rights reserved.
//

#ifndef api_hpp
#define api_hpp

#include <string>
#include <vector>

#include "curl/curl.h"
#include "json/json.h"

#include "./database.hpp"

namespace neo4jDriver
{
    class Neo4jAPI
    {
    public:
        
        /****************/
        /*    构造方法   */
        /****************/
        
        //构造方法
        Neo4jAPI(std::shared_ptr<neo4jDriver::Neo4j> database);
        
        //构造方法
        Neo4jAPI(std::shared_ptr<neo4jDriver::Neo4j> database, std::string host, std::string port, std::string user, std::string password);
        
        /****************/
        /*  基本信息及操作 */
        /****************/
        
        std::string getHost();
        
        std::string getPort();
        
        std::string getUser();
        
        std::string getPassword();
        
        void connectDatabase();
        
        void closeDatabase();
        
        /****************/
        /*    驱动接口   */
        /*     点部分    */
        /****************/
        
        //执行Cypher语句
        Json::Value cypherQuery(std::string cypher, Json::Value properties = Json::nullValue);
        
        //创建点（单Label）
        Json::Value createNode(const Json::Value &properties, const std::string &label);
        
        //创建点（多Label）
        Json::Value createNode(const Json::Value &properties, const std::vector<std::string> &labels);
        
        //根据点ID删除点
        bool deleteNode(unsigned long long int nodeID);
        
        //根据点ID删除点和该点的全部关系
        /*
         * 这个版本有问题，可能出现删除了全部边后因为网络问题没有将点删除的问题，健壮性依赖于网络
         * 改进方案，使用事务处理
         */
        bool deleteNodeAndAllRelationshipsOfTheNode(unsigned long long int nodeID);
        
        //根据点Label和属性删除点（单Label）
        unsigned long long int deleteNodeByLabelAndProperties(std::string label, Json::Value &properties);
        
        //根据点Label和属性删除点（多Label）
        unsigned long long int deleteNodeByLabelsAndProperties(std::vector<std::string> &labels, Json::Value &properties);
        
        //获得指定ID的点
        bool getNode(unsigned long long int nodeID, Json::Value &node);
        
        /*
         * 查询指定Label和属性的点（单Label）
         */
        Json::Value selectNodesByLabelAndProperties(std::string label, Json::Value &properties);
        
        /*
         * 查询指定Label和属性的点（多Label）
         */
        Json::Value selectNodesByLabelsAndProperties(std::vector<std::string> &labels, Json::Value &properties);
        
        /*
         * 查询和指定点存在指定关系的点
         */
        Json::Value selectNodesByAnotherLinkedNode(std::string selectedLabel, Json::Value &selectedProperties, std::string relationTypeName, Json::Value &relationProperties, std::string linkedLabel, Json::Value &linkedProperties);
        
        Json::Value selectNodesByAnotherLinkedNode(std::string selectedLabel, Json::Value &selectedProperties, std::string relationTypeName, Json::Value &relationProperties, unsigned long long int linkedNodeID);
        
        /*
         * 更新点的属性
         * 该方法只会修改properties中存在的属性，点的其他属性仍将保留
         */
        bool updateNodeByID(unsigned long long int nodeID, Json::Value &properties);
        
        /*
         * 替换点的属性
         * 该方法会用properties替换点的全部属性，properties中没有的属性将丢失
         */
        bool replaceNodeByID(unsigned long long int nodeID, Json::Value &properties);
        
        /****************/
        /*    驱动接口   */
        /*     边部分    */
        /****************/
        
        //根据始点和终点的ID插入两点的关系
        //返回值表示是否插入成功，关系本身由引用参数返回
        bool insertRelationship(unsigned long long int rootNodeID, unsigned long long int otherNodeID, std::string typeName, Json::Value &properties, Json::Value &relationship);
        
        //根据始点和终点的ID插入两点的关系(关系不含属性)
        //返回值表示是否插入成功，关系本身由引用参数返回
        bool insertRelationship(unsigned long long int rootNodeID, unsigned long long int otherNodeID, std::string typeName, Json::Value &relationship);
        
        //根据始点和终点的ID插入两点的关系(关系不含属性)
        //返回值表示是否插入成功，不返回关系
        bool insertRelationship(unsigned long long int rootNodeID, unsigned long long int otherNodeID, std::string typeName);
        
        //删除指定ID的关系
        bool deleteRelationship(unsigned long long int relationshipID);
        
        //删除指定ID的点的指定类型的边
        unsigned long long int deleteRelationshipsOfOneNode(unsigned long long int nodeID, std::string type);
        
        //删除指定ID的点的全部边
        unsigned long long int deleteAllRelationshipsOfOneNode(unsigned long long int nodeID);
        
        /*
         * 删除指定ID的点的全部出边
         */
        unsigned long long int deleteAllOutgoingRelationshipsOfOneNode(unsigned long long int nodeID);
        
        /*
         * 删除指定ID的点的全部入边
         */
        unsigned long long int deleteAllIncomingRelationshipsOfOneNode(unsigned long long int nodeID);
        
        /*
         * 获得指定ID的关系
         * 返回值表示是否找到，关系本身由引用参数返回
         */
        bool getRelationship(unsigned long long int relationshipID, Json::Value &relationship);
        
        /*
         * 获得指定ID的点的指定类型的所有边
         */
        Json::Value getRelationshipsOfOneNode(unsigned long long int nodeID, std::string type);
        
        /*
         * 获得指定ID的点的所有边
         */
        Json::Value getAllRelationshipsOfOneNode(unsigned long long int nodeID);
        
        /*
         * 获得指定ID的点的所有出边
         */
        Json::Value getAllOutgoingRelationshipsOfOneNode(unsigned long long int nodeID);
        
        /*
         * 获得指定ID的点的所有入边
         */
        Json::Value getAllIncomingRelationshipsOfOneNode(unsigned long long int nodeID);
        
        /*
         * 查询指定type和属性的关系
         */
        //        Json::Value selectRelationshipsByTypeAndProperties(std::string type, Json::Value &properties);
        
        //获得指定ID的起点和终点之间的全部边
        Json::Value getRelationsBetweenTwoNodes(unsigned long long int fromNodeID, unsigned long long int toNodeID, std::string type = "");
        
        /*
         * 更换关系的属性
         */
        bool replaceRelationshipProperties(unsigned long long int relationshipID, Json::Value &properties);
        
        /*
         * 析构方法
         */
        ~Neo4jAPI();
        
    private:
        std::shared_ptr<neo4jDriver::Neo4j> database;
        std::string host, port, user, password;
        CURL* curl;
        struct curl_slist* headers;
        std::string responseString;
        std::string responseHeaderString;
        
        /*
         * 对HTTP协议返回的协议头进行处理的回调方法
         */
        static size_t responseHeaderHandeler(char *buffer, size_t size, size_t nitems, void *userdata);
        
        /*
         * 对HTTP协议返回的数据进行处理的回调方法
         */
        static size_t responseHandeler(char *ptr, size_t size, size_t nmemb, void *userdata);
    };
}

#endif /* api_hpp */
