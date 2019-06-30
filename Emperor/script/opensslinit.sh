

function Exception()
{
	echo -e "--------------------------------------------------"
	#echo -e "$1"
	echo -e "\e[0;31;1m[ERROR]:$1\e[0m"
	echo -e "--------------------------------------------------"
	exit 1
}

#/usr/local/include/ 头文件应该也可以拷贝到这个地方把

cp  -rf ../openssllib/include/openssl/   /usr/include/
if [ $? -ne 0 ];then
    Exception "cp failed!"
fi

cp ../openssllib/include/lib/*  /usr/lib   