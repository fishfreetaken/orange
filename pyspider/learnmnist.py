
import struct
import time
import numpy as np


import matplotlib.pyplot as plt

#fig,ax = plt.subplots(2,5)
#ax=ax.flatten()

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
        #images = np.insert(images,784,values=1,axis=1)

        norimages=np.ones((len(labels),784))
        for i in  range(len(labels)):
            norimages[i]=Normalize(images[i])

        norimages = np.insert(norimages, 784, values=1., axis=1)
        print(images[0])
        print(norimages[0])

        #for i in range(10):
        #    print(f"{i:4} ==> {labels[i]:4}")
        #    print(f"{images[i]}")
           # img=images[i].reshape(28,28)
         #   ax[i].imshow(img,cmap='Greys',interpolation='nearest')
         #   ax[i].set_xticks([])
         #   ax[i].set_yticks([])
        #plt.tight_layout()
        #plt.show()
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

        norimages = np.insert(norimages, 784, values=1., axis=1)

    return labels,norimages

start=time.time();

(x_labels,x_train)=loadmnist()


print(f"loadmnist finish {len(x_labels)} {len(x_train)} ==> {len(x_train[0])}")
rx_train=np.transpose(x_train)
print(f"rx_train transpose {len(rx_train)} ==> {len(rx_train[0])}")
m_xtrain=np.dot(rx_train,x_train)
print(f"m_xtrain generator {len(m_xtrain)} ==> {len(m_xtrain[0])}")


midxos=np.dot(np.linalg.pinv(m_xtrain),rx_train)
print(f"midxos {len(midxos)} ==> {len(midxos[0])}")
omiga = np.dot(midxos,np.transpose(x_labels))
print(f"result over! {len(omiga)} ==> {omiga[0]})")

(y_labels,y_train)=loadmnisttest()

lastre=np.dot(omiga,np.transpose(y_train))
print(f"result over! {len(lastre)} ==> {lastre[0]})")

end=time.time();
print(end-start)

cc=0

ttl={}
ttl2={}
for i in range(10):
    ttl[i]=0;
    ttl2[i] = 0;

for i in range(len(lastre)):

    if round(lastre[i]) == y_labels[i]:
        ttl[y_labels[i]] +=1;
        cc+=1;
    else :
        print(lastre[i]," ===> ",y_labels[i])
    ttl2[y_labels[i]]+=1
print(f"last result cc={cc} total={len(lastre)}")
print(ttl)
print(ttl2)
