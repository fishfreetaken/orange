
#!/bin/bash

gRootPath="/mnt/team1data/cd/6700_0312/" 

export gEc=""
export gHEc=""
export gLEc=""

gPatchNo=""

#相对路径code/xtn/ps/resmgr  sup/code/
gSourcePath=""
gWorkPath=""
gTargetDirPath=""

gMakeCmd="libftm_flexe"
declare -A gCPUTypeArray
gCPUArch="39"
export gCPUType="ARMQORIQ"

gBoardIDList=("2338","2339") #一个cpu对应多个boardid

gLittleVersion="Base_67_11_104_138"
gBigVersion="ZXCTN6700V5.00.10B71"
gVersion="6700" #6500

gProcessName="RESMGR"
gProcessLE="T_CPUM"
gDynamicTag="static"
gLinkSOFile=""


gUnifiedName="" # gMakeCmd_gCPUType_gLEc : libftm_flexe_T1020_5070598.so .pch .json
gUnifiedNameSo=""
gUnifiedNamePch=""
gUnifiedNameJson=""

gCOrCPlus="0"


replFuncNameArray=()
repldFuncNameArray=()
replClassNameArray=()
repldClassNameArray=()
replFuncSymbArray=()
repldFuncSymbArray=()

sdlBoardNameArray=()
gboardNameArray=()

gPatchConflictHot=""

gtargetCpuArray=()

gFuncNameList=()
gFuncRepledNameList=()

#使用一个变量进行补丁文件名统一管理其阿里
gpatchfilename=""

#异常抛出
function Exception()
{
	echo -e "--------------------------------------------------"
	#echo -e "$1"
	echo -e "\e[0;31;1m[ERROR]:$1\e[0m"
	echo -e "--------------------------------------------------"
	exit 1
}

#Patch-5102349-7
function filterConfictPatch()
{
    if [ -z "$1" ];then
		echo "conflict is  NULL"
        return
    fi
    val=${1//,/" "}
    arry=($val)
    for i in ${arry[@]}
    do
        val=$i
        val=${val#*-} #5102349-7
        jsonname=${val#*-}
        jsonname="H"$jsonname".json"
        re=$(find . -name $jsonname)
        if [ -z "$re" ];then
            Exception "Cannot find conflicthot json:$val, $jsonname;"
        fi
        val=${val%-*} #5102349

        r=$(echo $re | grep $val)
        
        #r=$(ls | grep $val )
        if [ -z "$r" ];then
            Exception "your input ConflictHOT cannot find the file :$re , $val;"
            #continue;
        fi
        gPatchConflictHot=$gPatchConflictHot" "$i
    done
	tmp=($gPatchConflictHot)
	gPatchConflictHot=""
	
	if [ ${#tmp[@]} -le 1 ];then
		gPatchConflictHot="$tmp"
		echo "small $tmp"
		return
	fi
	
	for i in ${tmp[@]}
	do
		gPatchConflictHot=$gPatchConflictHot"*"$i
	done
	#echo $gPatchConflictHot
	gPatchConflictHot=${gPatchConflictHot#*\*}
	echo "gPatchConflictHot:" $gPatchConflictHot
	
}

function initVar()
{
    #echo $1
    key=${1%:*}
    val=${1#*:}

    key=${key//\"/}
    
    val=${val%\"*}
    val=${val//\"/}
    #val=${val/,/}
    #echo $key $val

#    if [ $key == "EC" ];then
#        gEc=$val
#        return
#    fi

#   if [ $key == "NO" ];then
#        gPatchNo=$val
#        return
#    fi

    if [ $key == "PROCESSOR" ];then
        gProcessName=$val
        return
    fi

    if [ $key == "BOARD" ];then
        val=${val//,/" "}
        gboardNameArray=($val)
        return
    fi

    if [ $key == "TargetCpu" ];then
        val=${val//,/" "}
        gtargetCpuArray=($val)
        return
    fi

    if [ $key == "ConflictHOT" ];then
        val=${val#*[}
        #echo $val
        val=${val%]*}
        filterConfictPatch $val
        
        return
    fi

    if [ $key == "SourcePatch" ];then
        gSourcePath=$val
        return
    fi

    if [ $key == "CompileCMD" ];then
        gMakeCmd=$val
        return
    fi

    if [ $key == "TargetSo" ];then
        gLinkSOFile=$val
        if [ ${#gLinkSOFile} -ne 0 ];then
            gDynamicTag="dynamic"
        fi
        return
    fi

    if [ $key == "ReplaceClass" ];then
        replClassNameArray=(${replClassNameArray[*]} $val)
        return
    fi
    if [ $key == "ReplacedClass" ];then
        repldClassNameArray=(${repldClassNameArray[*]} $val)
        return
    fi

    if [ $key == "Replace" ];then
        replFuncNameArray=(${replFuncNameArray[*]} $val)
        return
    fi
    if [ $key == "ReplacedName" ];then
        repldFuncNameArray=(${repldFuncNameArray[*]} $val)
        return
    fi
    if [ $key == "LittleVersion" ];then
        gLittleVersion=$val
		if [ -z $gLittleVersion ];then
			Exception "LittleVersion is NULL"
		fi
		gVersion=${gLittleVersion#*_}
		gVersion=${gVersion%%_*}
		gVersion=$gVersion"00"
        return
    fi
    if [ $key == "BigVersion" ];then
        gBigVersion=$val
        return
    fi
}

function showConfig()
{
    echo "************************************************"
    echo "Please check your input wether is what you want!"
    echo "************************************************"
    echo "EC :              $gEc"
    echo "PatchNo :         H$gPatchNo"
    echo "Process :         $gProcessName"
    echo "SourcePatch :     $gSourcePath"
    echo "CompileCMD :      $gMakeCmd"
    echo "LittleVersion :   $gLittleVersion"
    echo "BigVersion :      $gBigVersion"
	echo "TargetCpu" :      ${gtargetCpuArray[@]}
    echo "TargetSo:"        $gLinkSOFile $gDynamicTag
    echo "BoardNameArray:"  ${gboardNameArray[@]}
    echo "ConflictHot:"     $gPatchConflictHot
    for i in ${!replFuncNameArray[@]}
    do
        echo "replclass:    ${replClassNameArray[i]}   repl: ${replFuncNameArray[i]}"
        echo "repldclass:   ${repldClassNameArray[i]} repld: ${repldFuncNameArray[i]}"
    done
    
    echo "************************************************"
}

function funcJsonParse()
{
    line=${1/\{/}
    line=${line%\}*}

    line=(${line//,/" "})

    #echo "funcjson>>" $line

    for i in ${line[@]}
    do
        initVar $i
    done
}

function parseJson()
{
    #$1 表示第一个参数所指定的路径的json文件
    #pchmaker2.sh H1.json
    if [ ! -f $1 ];then
        Exception "$1 not exist!"
    fi
    
    controlfunc="0";
    while read line 
    do
        if [ ${#line} -lt 2 ];then
            continue
        fi
        
        if [ $controlfunc == "1" ];then
            if [ -n "$(echo $line | grep ']')" ];then
            #if [ $line == "]," ];then
         #       echo "rplay" ${#line}
                controlfunc="0"
                #continue
            else 
         #       echo "funcJsonParse":$line
                funcJsonParse $line
            fi
            continue
        fi 
        #echo $line $controlfunc
        
        key=${line%:*}

        if [ $key == '"Function"' ];then
            controlfunc="1"
            continue
        fi

        #val=${line#*:}
        
        initVar $line #$key $val
        
        #echo "-------------------------"
    done < $1

    showConfig
     
}

#./pchmaker2.sh ./patch613005070598/H1.json  解析配置文件
#parseJson $1  

function parseSDL()
{
    boardline=""
    tb=""
    while read line
    do
        tb=$(echo $line | grep "board=[0-9]* & cpu.arch=[0-9]*")
        if [ -n "$tb" ];then
            boardline=""
            if [ "$1" != "$2" ];then
                tb=$(echo $line | grep "board=[0-9]* & cpu.arch=[0-9]* & bom=$2 .*")
            fi
            if [ -n "$tb" ];then
                tb=${tb//&/}
                #echo $t
                #echo "pre"=${#t}
                tb=${tb/:*/}
                #echo "after"=${#t}
                boardline=$tb
            fi
        fi

        if [ -z "$boardline" ];then
            #echo "not find dest board;"
            continue;
        fi

        t=$(echo $line | grep "app, file=bin/.*/$gProcessName;")
        if [ -n "$t" ];then
            t=${t/;*/}
            t=${t/app,/}
            #echo "after"=${#t}
            echo $boardline 
            echo $t

			if [ ${#gtargetCpuArray[@]} -eq 0 ];then
				sdlBoardNameArray[${#sdlBoardNameArray[*]}]="$boardline""$t"
			else
				for i in ${gtargetCpuArray[@]}
				do
					tmp=$(echo $t | grep $i)
					if [ ! -z $tmp ];then
						echo "i==>" $i "t==>" $t
						sdlBoardNameArray[${#sdlBoardNameArray[*]}]="$boardline""$t"
					fi
				done
			fi
            boardline=""
            tb=""
            #sdlBoardNameArray[${#sdlBoardNameArray[*]}]="$boardline""$t"
        fi
    done < $1.sdl
}

function parseSdlArray()
{
boardline="board=2343 cpu.arch=44 lu=T_CPUM:"
tmp=($boardline)

val=${tmp[2]#*"="}

val=${val/:*/}

echo "----------------------------"
for i in ${!sdlBoardNameArray[@]}
do
    echo $i ${sdlBoardNameArray[i]}
    tmp=(${sdlBoardNameArray[i]})
    for j in ${tmp[@]}
    do
        key=${j%=*}
        val=${j#*=}
        if [ $key = "file" ];then
            cpu=${val%/*}
            cpu=${cpu#*/}
            if [ ${#gtargetCpuArray[@]} -eq 0 ];then
                gCPUTypeArray[$cpu]="1"
				continue;
            fi
            for i  in ${gtargetCpuArray[@]}
            do
                if  [ $cpu == $i ];then
                    gCPUTypeArray[$cpu]="1"
                fi
            done
            #gCPUTypeArray[$cpu]="1"
        fi
    done
done
echo "***********************************"
echo "your find cpu!"
echo "***********************************"
if [ ${#gCPUTypeArray[@]}  -eq 0 ];then
    Exception "No CPU found!"
fi

for i in ${!gCPUTypeArray[@]}
do
    echo $i
done
echo "***********************************"
}

#parseSDL ../COMMON/ncptb.sdl
#parseSdlArray

function findCommonDir()
{
    if [ ! -d ../COMMON/ ];then
        Exception "pkg/COMMON dir not exist!"
    fi
    cd ../COMMON

    for i in ${gboardNameArray[@]}
    do
        bom=${i#*/}
        boardname=${i%/*}
        echo $bom
        echo $boardname
        if [ ! -f "$boardname.sdl" ];then
            Exception "$boardname.sdl file do not exist, please check json!"
        fi
        parseSDL $boardname $bom
    done
    parseSdlArray

    cd -
}

function init()
{
    cd ../../../../
        gRootPath=$(pwd)"/"
    cd -

    parseJson $1

    #寻找逻辑实体和CPU类型和单板类型
    findCommonDir

    #进入补丁路径
    cd ./$gpatchfilename/
    gWorkPath=$(pwd)
    if [ ! -d ./target ];then
        mkdir ./target
    fi
    if [ ! -d ./config ];then
        mkdir ./config
    fi
	
	if [ ${#gEc} -lt 11 ];then
        Exception "invalid EC NO. $gEc"
    fi
	
    gHEc=${gEc:0:5}
    gLEc=${gEc:5}
}

#交叉编译器选择
gCompilerGCC=""
gCPlusOrC="c"
function selectGCC()
{
    case $gCPUType in
    "T1020")
        if [ $gCPlusOrC == "cpp" ];then
            gCompilerGCC="/opt/zte/20180919/ppc_gcc4.8.2_glibc2.18.0_multi/bin/ppc64_e5500-hardfloat-linux-gnu-g++"
        else
            gCompilerGCC="/opt/zte/20180919/ppc_gcc4.8.2_glibc2.18.0_multi/bin/ppc64_e5500-hardfloat-linux-gnu-gcc"
        fi
    ;;
    "ARMQORIQ")
        if [ $gCPlusOrC == "cpp" ];then
            gCompilerGCC="/opt/zte/20180105/aarch64be_eabi_gcc6.2.0_glibc2.24.0_fp_be8/bin/aarch64_be-unknown-linux-gnueabi-g++"
        else
            gCompilerGCC="/opt/zte/20180105/aarch64be_eabi_gcc6.2.0_glibc2.24.0_fp_be8/bin/aarch64_be-unknown-linux-gnueabi-gcc"
        fi
    ;;
    "ARMADAXP")
        if [ $gCPlusOrC == "cpp" ];then
            gCompilerGCC="/opt/zte/20151112/armeb_eabi_gcc4.5.2_glibc2.13.0_be8/bin/armeb-unknown-linux-gnueabi-g++"
        else
            gCompilerGCC="/opt/zte/20151112/armeb_eabi_gcc4.5.2_glibc2.13.0_be8/bin/armeb-unknown-linux-gnueabi-gcc"
        fi
    ;;
    "ARMQORIQLE")
        if [ $gCPlusOrC == "cpp" ];then
            gCompilerGCC="/opt/zte/20180309/aarch64_eabi_gcc6.2.0_glibc2.24.0_fp/bin/aarch64-unknown-linux-gnueabi-g++"
        else
            gCompilerGCC="/opt/zte/20180309/aarch64_eabi_gcc6.2.0_glibc2.24.0_fp/bin/aarch64-unknown-linux-gnueabi-gcc"
        fi
    ;;
    "P2020")
        if [ $gCPlusOrC == "cpp" ];then
            gCompilerGCC="/opt/zte/20151112/ppc8540_gcc4.1.2_glibc2.5.0/bin/powerpc-8540-linux-gnu-g++"
        else
            gCompilerGCC="/opt/zte/20151112/ppc8540_gcc4.1.2_glibc2.5.0/bin/powerpc-8540-linux-gnu-gcc"
        fi
    ;;
    esac
}

#将原文件进行拷贝到目标目录下
declare -A gfindSrcArray

function delSrcFile()
{
    for i in ${!gfindSrcArray[@]}
    do
        echo "del: " $i ${gfindSrcArray[$i]}
        rm ${gfindSrcArray[$i]}$i
    done
}

function copySourceFile()
{
    tmpSrclist=(`ls ./source/`)
    tmpSrcpath=$gRootPath$gSourcePath

    echo tmpSrcpath: $tmpSrcpath
    echo ${tmpSrclist[@]}

    for i in ${tmpSrclist[@]}
    do
        tof=${i/_patch/}
        echo "findpath:" $tmpSrcpath
        findpath=$(find $tmpSrcpath -name $tof )
        
        if [ -z $findpath ];then
            Exception "$tmpSrcpath not find $tof file, Please check your path!"
        fi
        
        findpath=${findpath%/*}"/"
        gfindSrcArray[$i]=$findpath
        cp ./source/$i  $findpath
    done
    gCPlusOrC=${tmpSrclist#*.}

    echo $gCPlusOrC
    #delSrcFile
}

function testAndDel()
{
    if [ -n "$(ls $2\*.$1)" ];then
        rm $2*.$1
    fi
}

#编译
function genCoFile()
{
    lowcpu=$(echo $gCPUType | tr "A-Z" "a-z")

    cd ../../../make

    make $gMakeCmd/$lowcpu _PATCH_TYPE=123  REL=1 -kj128
    if [ $? -ne 0 ];then
        Exception "make $gMakeCmd/$1 rel=1 failed!"
    fi
    cd -

    #清理一下所有文件
    if [ ! -d ./target/$gCPUType/ ];then
        mkdir ./target/$gCPUType/
    fi
    if [ -n "$(ls ./target/$gCPUType/)" ];then
        rm ./target/$gCPUType/*
    fi

    targetDirPath=""
    suporno=${gSourcePath%%/*}
    
    if [ $suporno == "sup" ];then
        targetDirPath=$gRootPath"sup/target/tmp/no_nsr_release/"
        targetDirPath=$targetDirPath"lib/"${gMakeCmd:3}"_$lowcpu""_"$gDynamicTag"/"

        if [ $gDynamicTag == "dynamic" ];then
            cp $gRootPath"sup/target/no_nsr_release/lib/$lowcpu/$gLinkSOFile" ./target/$gCPUType/
            if [ $? -ne 0 ];then
                Exception "gDynamicTag cp $gRootPath""target/no_nsr_release/lib/$lowcpu/$gLinkSOFile failed"
            fi
        fi
    else
        targetDirPath=$gRootPath"target/tmp/no_nsr_release/"
        libOrBin=${gMakeCmd:0:3}
        if [ $libOrBin == "lib" ];then
            targetDirPath=$targetDirPath"lib/"${gMakeCmd:3}"_$lowcpu""_"$gDynamicTag"/"
        else
            targetDirPath=$targetDirPath"bin/"$lowcpu"/"$gProcessName"/"
        fi

        if [ $gDynamicTag == "dynamic" ];then
            cp $gRootPath"target/no_nsr_release/lib/$lowcpu/$gLinkSOFile" ./target/$gCPUType/
            if [ $? -ne 0 ];then
                Exception "gDynamicTag cp $gRootPath""target/no_nsr_release/lib/$lowcpu/$gLinkSOFile failed"
            fi
        fi
    fi
    echo targetDirPath: $targetDirPath

    tmpSrclist=(`ls ./source/`)
    for i in ${tmpSrclist[@]}
    do
        cof=${i/.$gCPlusOrC/}".$gCPlusOrC""o"
        tmppt=$(find $targetDirPath -name $cof)
        echo cof:$cof
        echo $tmppt
        
        cp $tmppt ./target/$gCPUType/
        if [ $? -ne 0 ];then
            Exception "cp tmppt $tmppt/$cof ./target/$gCPUType/ failed!"
        fi
        #_patch的nm输出
        nm $tmppt >> ./target/$gCPUType/$gUnifiedName.sori  
        if [ $? -ne 0 ];then
            Exception "nm tmppt $tmppt/$cof ./target/$gCPUType/ failed"
        fi

        #将原来的编译输出代码拷贝到本地进行对比
        tmppt=${tmppt/_patch/}
        echo soridtmppt $tmppt
        #cp $tmppt/$cof ./target/$gCPUType/
        nm $tmppt >> ./target/$gCPUType/$gUnifiedName.sorid
        if [ $? -ne 0 ];then
            Exception "nm $tmppt/$cof ./target/$gCPUType/ failed"
        fi

    done
}

#链接生成动态库文件
function genSOFile()
{
    #根据cpu类型选择好编译器
    selectGCC
    cd ./target/$gCPUType/
    if [ $gDynamicTag == "dynamic" ];then
        lib=${gLinkSOFile/.so/}
        lib=${lib/lib/}
        $gCompilerGCC -l$lib -L./  -shared -o $gUnifiedNameSo *.$gCPlusOrC"o"
    else
        $gCompilerGCC  -shared -o $gUnifiedNameSo *.$gCPlusOrC"o"
    fi
    if [ $? -ne 0 ];then
        Exception "$gCompilerGCC  -shared -o $gUnifiedNameSo *.$gCPlusOrC failed $gDynamicTag"
    fi
    cd -
}

function acquirFuncSymbal()
{
    gFuncNameList=()
    for i in ${!repldFuncNameArray[@]}
    do
        echo repldFuncNameArray $i ${repldFuncNameArray[i]}
        exgre=${repldFuncNameArray[i]}
        line=""
        if [ $gCPlusOrC == "cpp" ];then
            exgre=".*"${repldClassNameArray[i]}".*"${repldFuncNameArray[i]}".*"
            line=($(grep "[a-f0-9]\{8,\} [A-Z] $exgre" ./target/$gCPUType/$gUnifiedName.sorid))
        else
            line=($(grep "[a-f0-9]\{8,\} [A-Z] $exgre$" ./target/$gCPUType/$gUnifiedName.sorid))
        fi
        if [ -z "$line" ];then
            Exception "No repld symbal found in sorid file: $exgre"
        fi
        #echo repldexgre:$exgre $line

        num=${#line[@]}

        if [ $num -gt 3 ];then
            tmplist=()
            for i in ${line[@]}
            do
                tps=$(echo $i | grep $exgre)
                if [ -z $tps ];then
                    continue
                fi
                tmplist=(${tmplist[@]} $tps)
            done
            echo ${tmplist[@]}
            echo -e "\e[0;31;33m[Which func do you want to repld]:\e[0m"
            for((i=0;i<${#tmplist[@]};i++))
            do
                echo -e "\e[0;31;32m[$i: ${tmplist[i]}]:\e[0m"
            done
            read -p "input:" ith
            echo "your input is "$ith
            line=${tmplist[ith]}
        else
            num=$((num-1))
            line=${line[num]}
        fi

        line=${line%"\n*"}
        if [ -z "$line" ];then
            Exception "grep sorid $i ${repldFuncNameArray[i]} line: $line  "
        fi

        echo after $line
        gFuncRepledNameList=(${gFuncRepledNameList[@]} $line)
    done

    echo "---------------------------------------"

    for i in ${!replFuncNameArray[@]}
    do
        echo replFuncNameArray $i ${replFuncNameArray[i]}
        exgre=${replFuncNameArray[i]}
        line=""
        if [ $gCPlusOrC == "cpp" ];then
            exgre=".*"${replClassNameArray[i]}".*"${replFuncNameArray[i]}".*"
            line=($(grep "[a-f0-9]\{8,\} [A-Z] $exgre" ./target/$gCPUType/$gUnifiedName.sori))
        else
            line=($(grep "[a-f0-9]\{8,\} [A-Z] $exgre$" ./target/$gCPUType/$gUnifiedName.sori))
        fi
        if [ -z "$line" ];then
            Exception "No repl symbal found in sori file: $exgre"
        fi

        num=${#line[@]}

        if [ $num -gt 3 ];then
            tmplist=()
            for i in ${line[@]}
            do
                tps=$(echo $i | grep $exgre)
                if [ -z $tps ];then
                    continue
                fi
                tmplist=(${tmplist[@]} $tps)
            done
            echo ${tmplist[@]}
            echo -e "\e[0;31;33m[Which func do you want to repld]:\e[0m"
            for((i=0;i<${#tmplist[@]};i++))
            do
                echo -e "\e[0;31;32m[$i: ${tmplist[i]}]:\e[0m"
            done
            read -p "input:" ith
            echo "your input is "$ith
            line=${tmplist[ith]}
        else
            num=$((num-1))
            line=${line[num]}
        fi


        
        line=${line%"\n*"}
        if [ -z "$line" ];then
            Exception "grep sori $i ${repldFuncNameArray[i]} line: $line  "
        fi

        echo after $line
        gFuncNameList=(${gFuncNameList[@]} $line)
    done
   
}

#首先生成json文件
function genPchJsonFile()
{
    acquirFuncSymbal
    cd ./config
    

    echo  "{" >> $gUnifiedNameJson
    echo "  \"DynLib\"  : \"$gWorkPath/target/$gCPUType/$gUnifiedNameSo\"," >> $gUnifiedNameJson
    echo "  \"PatchFile\"   : \"$gWorkPath/config/$gUnifiedNamePch\"," >> $gUnifiedNameJson
    echo "  \"FuncTable\"   : [" >> $gUnifiedNameJson
    
    if [ ${#gFuncNameList[@]} -eq 0 ];then
        Exception "gFuncNameList has no val"
    fi
    cycnum=$((${#gFuncNameList[@]}-1))
    if [ ${#gFuncNameList[@]} -gt 1 ];then
        for((i=0;i<$cycnum;i++))
        do
            echo  "     { \"Replace\":\"${gFuncNameList[$i]}\", \"ReplacedName\":\"${gFuncRepledNameList[$i]}\"}," >> $gUnifiedNameJson
        done
    fi
    echo  "     { \"Replace\":\"${gFuncNameList[$cycnum]}\", \"ReplacedName\":\"${gFuncRepledNameList[$cycnum]}\"}" >> $gUnifiedNameJson

    echo "]," >> $gUnifiedNameJson
    echo "\"PatchVerNo\" : \"$gLEc\"," >> $gUnifiedNameJson
    echo "\"DepVerNo\" : \"0\","  >> $gUnifiedNameJson
    echo "\"Producer\" : \"zte\"," >> $gUnifiedNameJson
    echo "\"ProblemDesc\" : \"zte 6700V5\"" >> $gUnifiedNameJson

    echo "}" >> $gUnifiedNameJson
    #chmod 777 $gUnifiedNameJson
    cd -
}

#生成pch文件
function genPchFile()
{
    #首先需要生成pch所需要的json文件
    genPchJsonFile
    cd ../
    #删除重新生成
    ./patchtool_v3 -c ./$gpatchfilename/config/$gUnifiedNameJson
    if [ $? -ne 0 ];then
        Exception "./patchtool_v3 -c ./$gpatchfilename/config/$gUnifiedNameJson"
    fi
    cd -
}

function mainwork()
{
    init $1
	echo gLEc:$gLEc
    copySourceFile
	
	cd ./config
	if [ -n "$(ls | grep .json)"  ];then
        rm *.json  #清除之前生成的pch和json文件
    fi
    if [ -n "$(ls | grep .pch)"  ];then
        rm *.pch  #清除之前生成的pch和json文件
    fi
	cd -

    for i in ${!gCPUTypeArray[@]}
    do
        gCPUType=$(echo $i | tr "a-z" "A-Z")
        gUnifiedName=$gMakeCmd"_"$gCPUType"_"$gLEc
        gUnifiedNamePch=$gUnifiedName".pch"
        gUnifiedNameSo=$gUnifiedName".so"
        gUnifiedNameJson=$gUnifiedName".json"
        #echo gCPUType: $gCPUType
        #echo gUnifiedName: $gUnifiedName
        genCoFile
        genSOFile
        genPchFile
    done

    #删除原文件
    delSrcFile
}

#生成config打包配置文件
function genConfigMak()
{
    tmpfile="config.mak"
	tmpcfpath=$(pwd)
    if [ -f $tmpfile ];then
        rm $tmpfile
    fi
    echo "IR_NSR=nsr" >> $tmpfile
    echo "BUILD_DEFINE=-DARCH_TYPE=$gCPUType -DLIBPCAP_NUM=1 -DLIBPCAP_VERNUM=1.0.0 -DLIB_NSR=lib_nsr -DDIR_NSR=\$(DIR_NSR)" >> $tmpfile
    echo "PROJECT=\"$gBigVersion REL\"" >> $tmpfile
    echo "_PKG_CONFIG=hotpatch/$gpatchfilename/config/" >> $tmpfile
    #echo "EXTERN_FILEINFO=$gRootPath""product/"$gVersion"/pkg/hotpatch/$gpatchfilename/config/fileinfo" >> $tmpfile
	echo "EXTERN_FILEINFO="$tmpcfpath"/fileinfo" >> $tmpfile
    echo "VM_PKGS=Patch" >> $tmpfile
}

function genFileInfo()
{
    tmpfile="fileinfo"
    if [ -f $tmpfile ];then
        rm $tmpfile
    fi

    echo "FILE,SOURCE,COMPRESS,MAKE" >> $tmpfile
    echo "sdl/patch.sdl,\${PROJECT}/pkg/hotpatch/$gpatchfilename/config/patch.sdl,," >> $tmpfile
    pchlist=($(ls *.pch))
    for i in ${pchlist[@]}
    do
        echo "patch/$i,\${PROJECT}/pkg/hotpatch/$gpatchfilename/config/$i,," >> $tmpfile
    done
}

function genPackage()
{
    #pth=$gRootPath"target/pkg/6700/hotpatch/hot/H$gPatchNo/"
    
    tmpfile="package"
    if [ -f $tmpfile ];then
        rm $tmpfile
    fi
    echo "[Patch]" >> $tmpfile
    echo "type=patch" >> $tmpfile

    #if [ -d $pth ];then
    #    name=($(ls $pth))
    #    echo "id=$gLEc-$gPatchNo-${#name[@]}" >> $tmpfile
    #else
    #    echo "id=$gLEc-$gPatchNo-0" >> $tmpfile
    #fi
	echo "id=$gLEc-$gPatchNo" >> $tmpfile
    
    echo "suffix=set" >> $tmpfile
    echo "attrtype=hot" >> $tmpfile
    echo "depend=$gLittleVersion" >> $tmpfile
    echo "conflict="$gPatchConflictHot >> $tmpfile
    echo "activate=parallel-update" >> $tmpfile
    echo "deactivate=parallel-update" >> $tmpfile
    echo "merge=sdl/patch.sdl" >>   $tmpfile
}

function genPatchSdl()
{
    #sdlBoardNameArray[${#sdlBoardNameArray[*]}]="$boardline""$t" 
    #board=2305 cpu.arch=54 lu=T_CPUM file=bin/armqoriq/RESMGR

    tmpfile="patch.sdl"
    if [ -f $tmpfile ];then
        rm $tmpfile
    fi
    
    declare -a dicttmp;

    echo "__if lu<__merge<">> $tmpfile
	
    #for i in ${sdlBoardNameArray[@]}
    for i in ${!sdlBoardNameArray[@]}
    do
        echo $i ${sdlBoardNameArray[$i]}
        t=(${sdlBoardNameArray[$i]})
        if [ ${#t[@]} -lt 4 ];then
            Exception "genPatchSdl $i ${#t[@]} invalid"
        fi

        if [ "${t[2]%=*}" == "bom" ];then
            tmps="${t[0]#*=}${t[1]#*=}${t[2]#*=}"
        else
            tmps="${t[0]#*=}${t[1]#*=}"
        fi

        if [ ! -z ${dicttmp[$tmps]} ];then
            continue
        fi
        dicttmp["$tmps"]=1

        if [ "${t[2]%=*}" == "bom" ];then
            echo "  ${t[0]} & ${t[1]} & ${t[2]} & ${t[3]}:"  >>  $tmpfile
        else
            echo "  ${t[0]} & ${t[1]} & ${t[2]}:"  >>  $tmpfile
        fi

        cpu=${sdlBoardNameArray[$i]}
        cpu=${cpu#*/}
        cpu=${cpu%/*}
        cpu=$(echo $cpu| tr 'a-z' 'A-Z')
        pchname=$(ls *.pch | grep $cpu )
        echo pchname:$pchname cpu: $cpu
        if [ -z $pchname ];then
             Exception "genPatchSdl pchname is null! cpu:$cpu ${sdlBoardNameArray[$i]}"
        fi

        index=${#t[@]}
        if [ $index -eq 0 ];then
            Exception "genPatchSdl index is 0! t:${t[@]}"
        fi

        index=$((index-1))
        bin=${t[$index]}
        bin=${bin#*=}
        
        echo "      hotpatch, pchtype=app, file=patch/$pchname,pchdst=$gLittleVersion/$bin, pchid=$gLEc;;" >> $tmpfile
    done

	#echo "hotpatch, pchtype=app, file=patch/$gUnifiedNamePch,pchdst=$gLittleVersion/bin/t1020/$gProcessName, pchid=$gLEc;;" >> $tmpfile
    echo ">>" >> $tmpfile
}

function movrename()
{
    pth=$gRootPath"target/pkg/"$gVersion"/hotpatch"
    if [ ! -d $pth ];then
        Exception "movrename pth: $pth"
    fi
    dstpth=$pth/hot/
    if [ ! -d $dstpth ];then
        mkdir $dstpth
    fi
    dstpth=$dstpth"H$gPatchNo/"
    if [ ! -d $dstpth ];then
        mkdir $dstpth
    fi

    srcpth=$pth/$gpatchfilename/config/RELEASE/hotpatch/$gpatchfilename/config/
    if [ ! -d $srcpth ];then
        Exception "movrename srcpth: $srcpth"
    fi
    
    name=($(ls $dstpth))
    last=${#name[@]}
	last=$((last+1))
    cp $srcpth"Patch-$gLEc-$gPatchNo".set $dstpth"$gBigVersion"H$gPatchNo"B$last".set
    if [ $? -ne 0 ];then
        Exception "movrename cp failed $gLEc"
    fi
	
	echo "SUCCESS: Hotpatch is generated in " $dstpth"$gBigVersion"H$gPatchNo"B$last".set

}

function buildpkg()
{
    
    cd ./config/
    pwd
    genConfigMak
    genFileInfo
    genPackage
    genPatchSdl
    cd -
    cd ../../

make config _PKG_CONFIG=hotpatch/$gpatchfilename/config/ REL=1 

if [ $? -ne 0 ];then
    Exception "make config _PKG_CONFIG=hotpatch/$gpatchfilename/config/ REL=1  failed"
fi

make Patch _CONFIG=hotpatch/$gpatchfilename/config REL=1

if [ $? -ne 0 ];then
    Exception "make Patch _CONFIG=hotpatch/$gpatchfilename/config REL=1 failed"
fi

movrename

}

function helpt()
{
    echo "Useage:"
    #echo "./pchmaker2.sh ./patch613001234567/H3.json"
    echo  "./pchmaker2.sh 5"
    echo  "5 is the patch NO."
}

function begin(){

    if [ -z $1 ];then
        helpt
        exit
    fi

    gpatchfilename=$(ls | grep "patch61.*_H$1")

    if [ ! -d $gpatchfilename ];then
        Exception "Cannot find $gpatchfilename dir!"
    fi
    
    #echo "patch613005106648_H5" | sed -n "s/patch//p" | sed -n "s/_H[0-9]*//p"
    gEc=$(echo $gpatchfilename | sed -n "s/patch//p" | sed -n "s/_H[0-9]*//p")
    gPatchNo=$(echo $gpatchfilename | sed -n "s/patch[0-9]*_H"//p)
    echo $gEc  $gPatchNo

    if [ -z $gEc ];then
        Exception "not find EC";
    fi
    if [ -z $gPatchNo ];then
        Exception "not find patchNO";
    fi

    para="./$gpatchfilename/H$gPatchNo.json"
    echo $para
    if [ ! -f $para ];then
        Exception "Cannot find $para file!"
    fi

    mainwork $para
    buildpkg

}

begin $1
