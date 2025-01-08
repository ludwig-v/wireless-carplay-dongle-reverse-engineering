########################
#File:    module_upgrade.sh
#author:  hcw
#date:    7Aug23
########################
#bin/bash

function failed_reason(){
    echo $1
    exit 1
}


FILEPATH=''
if [ ! -n "$1" ]; then
    failed_reason "Please pass in the package name"
fi

if [ -e /mnt/UPAN/${1} ]; then
    FILEPATH="/mnt/UPAN/"
    if [ -d "${FILEPATH}.M6_Update/" ]; then
 	    rm -r ${FILEPATH}.M6_Update
    fi
    mkdir ${FILEPATH}.M6_Update
    IMGFILE=${1}
    cp /mnt/UPAN/${IMGFILE} /mnt/UPAN/.M6_Update/
    IMGNAME=${IMGFILE%.*}   # such as M6_2023.05.24.1437
    echo ${IMGNAME}
    hwfsTools /mnt/UPAN/.M6_Update/${IMGFILE} /mnt/UPAN/.M6_Update/${IMGNAME}.zip
    unzip /mnt/UPAN/.M6_Update/${IMGNAME}.zip -d /mnt/UPAN/.M6_Update/ || failed_reason Unzip /mnt/UPAN/.M6_Update/${IMGNAME}.zip failed
elif [ -e /tmp/${1} ]; then
    FILEPATH="/tmp/"
    if [ ! -d "/tmp/.M6_Update/" ]; then
        mkdir /tmp/.M6_Update
        unzip /tmp/${1} -d /tmp/.M6_Update/ || failed_reason /tmp/${1}
    fi
else
    failed_reason "The Upgrade package not found"
fi

MODULELIST=`/script/getFuncModule.sh`
MODULEARR=`echo $MODULELIST | tr ',' ' '`
#echo ${MODULELIST}
echo ${MODULEARR}

LATESTVER='0000.00.00.0000'
MAJORVER='0000.00.00.0000'
for MODULE in ${MODULEARR}
do
    echo ${MODULE}
    MODULEVER='0000.00.00.0000'
    if [ -e ${FILEPATH}.M6_Update/${MODULE}_*.hwfs ]; then  #判断是否有模块对应的升级包文件并匹配hwfs后缀
        test -e /etc/module_version/${MODULE}.version && MODULEVER=`cat /etc/module_version/${MODULE}.version`
        if [ ${MODULE}  = "Upg" ]; then
            MODULEVER='0000.00.00.00'
        fi
        echo ${MODULEVER}
        NEWFILE=`ls ${FILEPATH}.M6_Update/${MODULE}_*`
        FILENAME=${NEWFILE##*/} # such as "common.2023.05.22.1409.hwfs"
        # echo ${FILENAME}
        PURENAME=${FILENAME%.*} # such as "common.2023.05.22.1409"
        echo ${PURENAME}
        if [ -z "${FILENAME##*$MODULEVER*}" ]; then
            echo "${MODULE} does not need to be upg"
        else
            if [ ! -d "/tmp/update/" ]; then
	            mkdir /tmp/update
            fi
            cp ${FILEPATH}.M6_Update/${FILENAME} /tmp
            hwfsTools /tmp/${FILENAME} /tmp/${PURENAME}.tar.gz
            tar -xvf /tmp/${PURENAME}.tar.gz -C /tmp/update || failed_reason Decompression of tar.gz failed
            rm /tmp/${PURENAME}.tar.gz
            MAJORVER=`cat /tmp/update/etc/module_version/${MODULE}.majorVersion`
            TMPLATEST=`echo ${LATESTVER} |sed s/'\.'//g`
            TMPMAJORVER=`echo ${MAJORVER} |sed s/'\.'//g`
            if [ ${TMPLATEST} -lt ${TMPMAJORVER} ]; then
                LATESTVER=${MAJORVER}
            fi
            echo "${MODULE} upgrade successful"
	    fi
    else    #不是hwfs后缀的文件提示
        echo " ${MODULE} upgrade file format error or no need to upgrade"
    fi
done

if [ "${MAJORVER}" != "0000.00.00.0000" ]; then
    touch /tmp/update/etc/software_version
    echo  -n ${LATESTVER} > /tmp/update/etc/software_version
    cat /tmp/update/etc/software_version
fi
if [ -d "${FILEPATH}.M6_Update/" ]; then
 	rm -r ${FILEPATH}.M6_Update
fi

exit 0
