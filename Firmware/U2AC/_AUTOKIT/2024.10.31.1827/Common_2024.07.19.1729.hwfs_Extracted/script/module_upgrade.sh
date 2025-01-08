########################
#File:    module_upgrade.sh
#author:  hcw
#date:    7Aug23
########################
#bin/bash

function failed_reason(){
    echo $1
    if [ -d "${FILEPATH}.M6_Update/" ]; then
        rm -r ${FILEPATH}.M6_Update
        rm -f ${FILEPATH}.hw_sign
        rm -f ${FILEPATH}.upfile_sign
    fi
    exit 1
}

function is_module_in_wirelessList() {
    local string="$1"
    local modulever=`cat /etc/module_version/$1.version`
    shift
    local WIFICHIPLIST="RTL8822BS BCM4358 BCM4335 BCM4354 SD8987 RTL8822CS"
    local found=0

    for item in ${WIFICHIPLIST}
    do
        if [ "$item" == "$string" ]; then
            found=1
            break
        fi
    done

    if [ $found -eq 1 ] && [ ${modulever} = '0000.00.00.0000' ]; then
        echo "Wireless driver is installed for the first time"
        return 1
    fi
}

function check_hwfs_md5() {
    if [ -f "/mnt/UPAN/.hw_sign" ]; then
        local new_hwfs_md5=$(md5sum "/mnt/UPAN/${IMGFILE}" | awk '{ print $1 }')
        local old_hwfs_md5=`cat /mnt/UPAN/.hw_sign`
        if [ "$new_hwfs_md5" == "$old_hwfs_md5" ]; then
            return 1
        else
            echo "Upgrade file changes, reprocess files"
            return 0
        fi
    else
        echo "The /mnt/UPAN/.hw_sign file is missing, reprocess the file"
        return 0
    fi
}

function calculate_folder_md5() {
    local directory="$1"
    if [ ! -d "$directory" ]; then
        echo "Directory does not exist: $directory"
        return 1
    fi

    touch "./upg_md5"
    local temp_file="/mnt/UPAN/upg_md5"
    #排序生成md5，以保证一致性
    for file in $(find "$directory" -type f | sort); do
        md5sum "$file" >> "$temp_file"
    done

    local folder_md5=$(md5sum "$temp_file" | awk '{ print $1 }')
    rm "$temp_file"
    echo "$folder_md5" #最后调用这个函数的时候获取这个值
}

function handle_hwfs_and_unzip() {
    if [ -d "${FILEPATH}.M6_Update/" ]; then
        rm -rf "${FILEPATH}.M6_Update/"
    fi
    mkdir ${FILEPATH}.M6_Update
    cp /mnt/UPAN/${IMGFILE} /mnt/UPAN/.M6_Update/
    echo 3 > /proc/sys/vm/drop_caches
    IMGNAME=${IMGFILE%.*}   # such as M6_2023.05.24.1437
    echo ${IMGNAME}
    hwfsTools /mnt/UPAN/.M6_Update/${IMGFILE} /mnt/UPAN/.M6_Update/${IMGNAME}.zip
    unzip /mnt/UPAN/.M6_Update/${IMGNAME}.zip -d /mnt/UPAN/.M6_Update/ || failed_reason "Unzip /mnt/UPAN/.M6_Update/${IMGNAME}.zip failed"
    local zip_md5=$(calculate_folder_md5 "/mnt/UPAN/.M6_Update/")
    local hwfs_md5=$(md5sum "/mnt/UPAN/${IMGFILE}" | awk '{ print $1 }')
    echo "$zip_md5" > "/mnt/UPAN/.upfile_sign"
    echo "$hwfs_md5" > "/mnt/UPAN/.hw_sign"
}

# Start processing hwfs files
FILEPATH=''
if [ ! -n "$1" ]; then
    failed_reason "Please pass in the package name"
fi

if [ -e /mnt/UPAN/${1} ]; then
    IMGFILE=${1}
    FILEPATH="/mnt/UPAN/"
    hwfs_flag=1
    check_hwfs_md5
    result=$?
    if [ $result -eq 0 ]; then
        hwfs_flag=0
    fi

    if [ $hwfs_flag -eq 0 ]; then   #重新处理 M6_Update.hwfs文件
        handle_hwfs_and_unzip
    else
        if [ -d "${FILEPATH}.M6_Update/" ]; then
            if [ -f "/mnt/UPAN/.upfile_sign" ]; then
                new_md5=$(calculate_folder_md5 "$FILEPATH.M6_Update/")
                upfile_md5=`cat /mnt/UPAN/.upfile_sign`
                if [ "$new_md5" == "$upfile_md5" ]; then
                    echo "Not need for decompression, can be upgraded directly"
                else
                    echo "MD5 verification failed, decompressed again"
                    handle_hwfs_and_unzip
                fi
            else
                echo "MD5 verification file missing"
                handle_hwfs_and_unzip
            fi
        else
            echo "/mnt/UPAN/.M6_Update does not exist"
            handle_hwfs_and_unzip 
        fi
    fi
elif [ -e /tmp/debug_update/${1} ]; then
    FILEPATH="/tmp/debug_update/"
    if [ -d "${FILEPATH}.M6_Update/" ]; then
        rm -r ${FILEPATH}.M6_Update
    fi
    mkdir ${FILEPATH}.M6_Update
    IMGFILE=${1}
    cp /tmp/debug_update/${IMGFILE} /tmp/debug_update/.M6_Update/
    IMGNAME=${IMGFILE%.*}   # such as M6_2023.05.24.1437
    echo ${IMGNAME}
    hwfsTools /tmp/debug_update/.M6_Update/${IMGFILE} /tmp/debug_update/.M6_Update/${IMGNAME}.zip
    unzip /tmp/debug_update/.M6_Update/${IMGNAME}.zip -d /tmp/debug_update/.M6_Update/ || failed_reason "Unzip /tmp/debug_update/.M6_Update/${IMGNAME}.zip failed"
elif [ -e /tmp/${1} ]; then
    FILEPATH="/tmp/"
    if [ ! -d "/tmp/.M6_Update/" ]; then
        mkdir /tmp/.M6_Update
        unzip /tmp/${1} -d /tmp/.M6_Update/ || failed_reason "/tmp/${1}"
    fi
else
    failed_reason "The Upgrade package not found"
fi

MODULELIST=`/script/getFuncModule.sh`
MODULEARR=`echo $MODULELIST | tr ',' ' '`
#echo ${MODULELIST}
echo ${MODULEARR}

HAVEUPG=0
#NEEDUPG=0
LATESTVER='0000.00.00.0000'
MAJORVER='0000.00.00.0000'
for MODULE in ${MODULEARR}
do
    echo ${MODULE}
    MODULEVER='0000.00.00.0000'
    if [ -e ${FILEPATH}.M6_Update/${MODULE}_*.hwfs ]; then  #判断是否有模块对应的升级包文件并匹配hwfs后缀
        test -e /etc/module_version/${MODULE}.version && MODULEVER=`cat /etc/module_version/${MODULE}.version`
        if [ ${MODULE}  = "Upg" ]; then
            MODULEVER='0000.00.00.0000'
            HAVEUPG=1
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
            echo 3 > /proc/sys/vm/drop_caches
            hwfsTools /tmp/${FILENAME} /tmp/${PURENAME}.tar.gz
            tar -xvf /tmp/${PURENAME}.tar.gz -C /tmp/update || failed_reason "Decompression of tar.gz failed"
            rm /tmp/${PURENAME}.tar.gz
            is_module_in_wirelessList "$MODULE"
            RESULT=$?
            if [ ${MODULE} = "Upg" ] || [ $RESULT -eq 1 ]; then	#ignore Upg.majorVarsion and wireless.majorVersion
                MAJORVER='0000.00.00.0000'
            else
                MAJORVER=`cat /tmp/update/etc/module_version/${MODULE}.majorVersion`
                #NEEDUPG=1	#Are there any modules that need to be updated?
            fi
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

if [ "$HAVEUPG" -eq 0 ]; then
    failed_reason "The package does not have the Upg module"
fi

if [ "${LATESTVER}" != "0000.00.00.0000" ]; then
    touch /tmp/update/etc/software_version
    echo  -n ${LATESTVER} > /tmp/update/etc/software_version
    cat /tmp/update/etc/software_version
fi
# 优化升级速度，升级成功不删除升级包
# if [ -d "${FILEPATH}.M6_Update/" ]; then
#     rm -r ${FILEPATH}.M6_Update
# fi

exit 0
