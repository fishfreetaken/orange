import os

filepath="data/sds.h"
wfilepath="data/analyse.txt"
fw=open(wfilepath,"a")

fo=open(filepath,"r")

g_bAnnotation=0
def annotationline(p,ith):
    global g_bAnnotation
    if 1 == g_bAnnotation:
        for i in range(len(p)):
            if p[i]=='*':
                if ((i+1)<len(p))and(p[i+1]=='/'):
                    g_bAnnotation=0
                    print("find the crmp end :",ith)
        return
    for i in range(len(p)):
        if p[i]=='/':
            if ((i+1)<len(p))and(p[i+1]=='*'):
                g_bAnnotation=1
                print("find the first :",ith)
    




line = fo.readline()

root=os.getcwd()
fw.write("Locations: %s\n"%(root))
fw.write("first line :%s"%(line))

g_iNumLine=1
while line:
   # print(line)
    annotationline(line,g_iNumLine)
    line=fo.readline()
    g_iNumLine +=1


#cpath  line   

fo.close()
