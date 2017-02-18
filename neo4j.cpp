//
//  neo4j.cpp
//  image_system
//
//  Created by skyblue on 15/11/1.
//  Copyright © 2015年 skyblue. All rights reserved.
//

#include "neo4j.hpp"

#include "/usr/local/opt/curl/include/curl/curl.h"
#include "../exception/database_exception.hpp"

namespace databaseDriver
{
    std::shared_ptr<Neo4j> Neo4j::neo4j;
    
    Neo4j::Neo4j()
    {
        //执行curl库的初始化操作
        if (curl_global_init(CURL_GLOBAL_ALL) != 0)
        {
            exception::DatabaseException exception(0);
            throw exception;
        }
    };
    
    std::shared_ptr<Neo4j> Neo4j::getNeo4j()
    {
        if (Neo4j::neo4j == nullptr)
        {
            Neo4j::neo4j = std::shared_ptr<Neo4j>(new Neo4j::Neo4j());
        }
        
        return Neo4j::neo4j;
    };

    void Neo4j::deleteNeo4j()
    {
        if (Neo4j::neo4j.use_count() == 1)
        {
            Neo4j::neo4j.reset();
        }
    };
    
    Neo4j::~Neo4j()
    {
        curl_global_cleanup();  //curl库的清理操作
    };
}
