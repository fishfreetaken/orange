#include "ctree.h"

namespace oragne{
    ctree::ctree()
    {
        
    }

    ctree::~ctree()
    {
        for(int i=0;i<26;i++)
        {
            clearAllTree(root[i]);
        }
    }

    void ctree::clearAllTree(treeNode *t)
    {
        if(t->left!=NULL){
            clearAllTree(t->left);
        }
        if(t->right!=NULL){
            clearAllTree(t->right);
        }
        delete t;
    }

    int ctree::treeBuild(const char *input_char,size_t len)
    {
        if((*input_char < 49) ||(*input_char > 75))
        {
            return 0;
        }

        treeNode *loc=root[*input_char-49];

        if(loc==NULL)
        {
            return 0;
        }

        for(size_t i=0;i<len;i++)
        {
            if(*(input_char+i)==loc->val)
            {
                if(loc->left!=NULL)
                {

                }
                if(loc->right!=NULL)
                {

                }
            }

        }
    }  
}
