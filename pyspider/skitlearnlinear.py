import struct
import time
import numpy as np

from sklearn import linear_model



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

print("load over!")
reg = linear_model.LinearRegression()

#reg = linear_model.Ridge(alpha=.5)
#reg = linear_model.Lasso(alpha=0.1)
print("linear over!")
reg.fit(x_train, x_labels);

print("fit over!")
print(reg.coef_);
ccc=0;
s=reg.predict(y_train)
print(len(s))
for i in range(len(s)):
    if round(s[i]) == y_labels[i]:
        ccc+=1;

print("ccc:",ccc);

score = reg.score(y_train, y_labels)
print(" score: {:.6f}".format(score))