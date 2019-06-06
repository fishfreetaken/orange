import numpy as np

from learnmnist import loadmnist,loadmnisttest


(x_labels,x_train)=loadmnist()

(y_labels,y_train)=loadmnisttest()

k=10;

iniarr=[]
for i in range(10):
    iniarr.append(x_train[i])

b = [[] for i in range(10)]
for i in x_train:
    dis=0
    for j in range(10):
        tmp=np.linalg.norm(i-iniarr[j]);
        if tmp < dis:
            dis=j
        b[dis].append(i);
for i in range(10):
    t=np.zeros()
    for j in b[i]:

