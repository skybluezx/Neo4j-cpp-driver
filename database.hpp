//
//  neo4j.hpp
//  Neo4j-cpp-driver
//
//  Created by skyblue on 2017/7/9.
//  Copyright © 2017年 skyblue. All rights reserved.
//

#ifndef neo4j_hpp
#define neo4j_hpp

#include <memory>

namespace neo4jDriver
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

#endif /* neo4j_hpp */
