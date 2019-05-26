
#ifndef __CTREE_H

#define __CTREE_H

# include <stddef.h>
#include <string>
#include <set>
namespace oragne{

typedef struct structTreeNode{
    char val;
    treeNode *pre;
    treeNode *left;
    treeNode *right;
} treeNode;

class ctree{
public:
    ctree ();
    ~ctree();

    void treeBuild(const std::string &input_string);

    int treeBuild(const char *input_char,size_t len);

    void clearAllTree(treeNode *t);

private :
    treeNode *root[26];
    std::set <std::string> pSetString;
};

}
#endif
