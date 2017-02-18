//
//  neo4j.hpp
//  image_system
//  Neo4j数据库驱动头文件
//
//  Created by skyblue on 15/11/1.
//  Copyright © 2015年 skyblue. All rights reserved.
//

#ifndef database_neo4j_hpp
#define database_neo4j_hpp

#include <memory>

namespace databaseDriver
{
    class Neo4j
    {
    public:
        static std::shared_ptr<Neo4j> getNeo4j();
        
        void deleteNeo4j();
        
        ~Neo4j();
        
    private:
        Neo4j();
        
        static std::shared_ptr<Neo4j> neo4j;
    };
}

#endif /* database_neo4j_hpp */
