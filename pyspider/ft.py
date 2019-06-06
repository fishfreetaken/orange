import requests as rq

t=rq.get("https://item.jd.com/100000913459.html");

print(type)
with open("./jd.html" ,"w") as f:
    f.write(t.text);