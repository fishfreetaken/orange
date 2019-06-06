import struct
import time
import numpy as np


def Normalize(data):
    m = np.mean(data)
    mx = max(data)
    mn = min(data)
    return np.array([(float(i) - m) / (mx-mn) for i in data])

def loadmnist():
    with open("/home/vr/mnist/train-labels-idx1-ubyte","rb") as fs:
        magic, n = struct.unpack('>II',fs.read(8))
        labels = np.fromfile(fs,dtype=np.uint8)

    with open("/home/vr/mnist/train-images-idx3-ubyte",'rb') as fd:
        magic, n, rows, cols= struct.unpack('>IIII', fd.read(16))

        images = np.fromfile(fd, dtype=np.uint8).reshape(len(labels), 784)

        norimages=np.ones((len(labels),784))
        for i in  range(len(labels)):
            norimages[i]=Normalize(images[i])

        return labels , norimages

def loadmnisttest():
    with open("/home/vr/mnist/t10k-labels-idx1-ubyte","rb") as fs:
        magic, n = struct.unpack('>II',fs.read(8))
        labels = np.fromfile(fs,dtype=np.uint8)

    with open("/home/vr/mnist/t10k-images-idx3-ubyte",'rb') as fd:
        magic, n, rows, cols= struct.unpack('>IIII', fd.read(16))

        images = np.fromfile(fd, dtype=np.uint8).reshape(len(labels), 784)

        norimages = np.ones((len(labels), 784))
        for i in range(len(labels)):
            norimages[i] = Normalize(images[i])


    return labels,norimages

st=time.time();
(x_labels,x_train)=loadmnist()

(y_labels,y_train)=loadmnisttest()




stl=time.time();
print("load time: ",stl-st);
K=3;
def knnsearch(data,labid):
    neast={}
    #ls=[]
    for i in  range(10):
        neast[i]=1000000000
    for i in range(len(x_labels)):
        tt=np.linalg.norm(x_train[i]-data)
        #ls.append(tt)
        if neast[x_labels[i]] > tt:
            neast[x_labels[i]]=tt;
    list1 = sorted(neast.items(), key=lambda x: x[1])
    #print("knnsearch: ",labid)

    #print(list1[0],list1[1],list1[2])
    #ls.sort()
    #print(ls[0],ls[1],ls[2])
    #print("-----end---------line--------------")
    return list1[0][0]

sct=stl;
kcc=0;

for i in range(len(y_labels)):
    t=knnsearch(y_train[i],y_labels[i])
    if i %500 ==0:
        print("miliston ",i," time: ",time.time()-sct," correct: ",kcc," rate: ",kcc/i);
        sct=time.time();
    if t == y_labels[i]:
      kcc+=1;
sttl=time.time()

print(kcc," vs ",len(y_labels))

print("elapse :",sttl-stl);