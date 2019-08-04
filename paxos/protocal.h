#ifndef PROTOCAL_HEADER_H
#define PROTOCAL_HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#define SINGLESTATION

/*
多机的花：
id+端口号+timestamp
单击：
端口号+timstamp+一个随机递增值
*/
class OneId{
public:
    OneId(uint32_t &port);

private:
    uint16_t pid_;//如果多机器版本则用ip
    size_t sec_;
};


class Propose{

public:
    void ParseBIncreaseId(size_t &m){id=m;};

/*创建对象的时候就应该生成一个 */
private:
    /*提供调用生成一个Id */
    void ParseAGenProposeId();

private:


    size_t id; //这个怎么生成？
    std::string key;
    std::string value;
};

#endif