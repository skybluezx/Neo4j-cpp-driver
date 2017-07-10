# Neo4j-cpp-driver
C++的Neo4j驱动,该驱动基于libcurl和Neo4j的REST接口实现了C++对Neo4j的操作，具体的接口方法参照了Neo4j官方为Node.js提供的功能。

##依赖
该驱动依赖libcurl和JsonCpp，在使用该驱动前请先安装配置好上述依赖库。
libcurl头文件在api.hpp中被引入，JsonCpp头文件在api.hpp和kit.hpp中被引入，在安装配置完依赖库后，需对应调整此处的头文件并引入库文件。
