#!/bin/bash
declare -A dit
#var1={`awk '{ a[$2]++; } END{ for( x in a) {print x; print a[x]}}' userip.dat >>/dev/null`}

var2=`awk '{print $2}' userip.dat | sort | uniq -c | sort -n -r`
#eval $(awk '{printf"var1=%s;var2=%s",$1,$2}' userip.dat)

t=($var2)
for i in ${!t[@]}
do
if [ `expr $i % 2` -eq 0 ];then
#echo ${t[$i]}
echo ${t[$i+1]}
dit[${t[$i+1]}]=${t[$i]}
fi
done
echo "~~~-----~~"
for i in ${!dit[@]}
do
echo $i : ${dit[$i]}
done

echo "----------------------------"

declare -a sum
declare -A dic
while read line
do

#line=`echo $line | sed 's/\n//g'`
#echo $line
tmp=($line)
dic[${tmp[1]}]=`expr ${dic[${tmp[1]}]} + ${dit[${tmp[0]}]}`
#dic[${tmp[1]}]=(${dic${tmp[1]}},${tmp[0]});
done < ipweb.dat

for key in ${!dic[@]}
do
echo "-------------------" 
    echo "$key ${dic[$key]}" | sed -e 's/\n/,/g' >> result.dat
    echo "~~~~~~~~~~~~~~"
    tmp=0
    for i in ${!sum[@]}
    do
        echo $i
        echo ${dic[$key]}
        echo ${dic[${sum[$i]}]}

        echo "%%%%%%%%%%%%%%%%"
    done
    sum=(${sum[@]} $key)
done

echo ${#sum[@]}


exit

#c = awk '{ a[$2] = a[$2] +" "+$1 } END{ for( x in a) {print x; print a[x]}}'  ipweb.dat
'''
        if [ ${dic[$key]} -gt ${dic[${sum[$i]}]} ];then
            ${sum[$i]}=$key
            continue;
        fi
        '''
