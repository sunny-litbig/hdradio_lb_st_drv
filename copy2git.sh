#!/bin/bash

git_dir="../../git"
git_conf_dir="gitconf"
tcadt_arch64=sysroots/aarch64-telechips-linux
tcadt_arch32=sysroots/cortexa7-vfp-neon-telechips-linux-gnueabi
tcadt_local=${OECORE_TARGET_SYSROOT}/usr/local

peri_dir="tc-radio-drv-peri"
silab_dir="tc-radio-drv-silab"
x0tuner_dir="tc-radio-drv-x0"
maxim_dir="tc-radio-drv-maxim"
st_dir="tc-radio-drv-st"
fmam_dir="tc-radio-demod-fmam"
hdr_dir="tc-radio-demod-hdr"
hd_dir="tc-radio-hd"
hd10mrc_dir="tc-radio-hd10mrc"
hd15_dir="tc-radio-hd15"
hd15mrc_dir="tc-radio-hd15mrc"
hal_dir="tc-radio-hal"
mw_dir="tc-radio-mw"
app_dir="tc-radio-app"
hd_mw_dir="tc-radio-hd-mw"
hd_app_dir="tc-radio-hd-app"

if [ ${ARCH} ];then
	if [ ${ARCH} = "arm64" ];then
		aarch64_en=1
		echo -e "\033[33;1m"ADT is AARCH64\(64bit\)"\033[0m"
	else
		aarch64_en=0
		echo -e "\033[33;1m"ADT is 32bit"\033[0m"
	fi
else
	echo -e "\033[31;1m"Please set the ADT!!!"\033[0m"
	exit
fi

echo -e "\033[33;1m"[Chipset]"\033[0m"
echo -e "\033[34;1m"1. TCC805x"\033[0m"
echo -e "\033[34;1m"2. The Others"\033[0m"
echo -e -n "\033[36;1m"Select your machine \(1-4\) : "\033[0m"
read input1

case $input1 in
	1)
		if [[ "$CC" == *"-mcpu=cortex-a72"* ]];then
			cpu_a72=1
		else
			cpu_a72=0
		fi
		config_v4px=1;;
	*)
		config_v4px=0;;
esac

if [ ${aarch64_en} -eq 1 ];then
	if [ ${config_v4px} -eq 1 ];then
		if [ ${cpu_a72} -eq 1 ];then
			lib_dir_name=lib64/a72
		else
			lib_dir_name=lib64/a53
		fi
	else
		lib_dir_name=lib64
	fi
else
	if [ ${config_v4px} -eq 1 ];then
		if [ ${cpu_a72} -eq 1 ];then
			lib_dir_name=lib/a72
		else
			lib_dir_name=lib/a53
		fi
	else
		lib_dir_name=lib
	fi
fi

echo "[Copy to tc-radio git]"
echo "1. DRV-PERI"
echo "2. DRV-SILAB"
echo "3. DRV-X0"
echo "4. DRV-MAXIM"
echo "5. DRV-ST"
echo "6. HAL"
echo "7. MW"
echo "8. GUI"
echo "9. HD Radio"
echo "10. HD MW"
echo "11. HD GUI"
echo -n "Select the git you want to copy (1-11) : "
read input2

case $input2 in
	1)
#PERI
		rm -rf ${git_dir}/tc-radio-drv-peri/src
		cp -rf peri/src ${git_dir}/tc-radio-drv-peri
		cp -rf peri/configure.ac ${git_dir}/tc-radio-drv-peri
		cp -rf peri/Makefile.am ${git_dir}/tc-radio-drv-peri
		;;
	2)
#TUNER - SILAB
		mkdir -p ${git_dir}/${silab_dir}/${lib_dir_name}
		cp -rf ${tcadt_local}/lib/libsi4796xdriver.so.*.*.* ${git_dir}/${silab_dir}/${lib_dir_name}
		mkdir -p ${git_dir}/${silab_dir}/src
		cp -rf tuner/src/si479xx/*_core.h ${git_dir}/${silab_dir}/src
		rm -rf ${git_dir}/${silab_dir}/bin
		cp -rf tuner/src/si479xx/bin ${git_dir}/${silab_dir}
		if [ ${config_v4px} -eq 0 ];then
			cp -rf ${git_conf_dir}/${silab_dir}/default/* ${git_dir}/${silab_dir}
		else
			cp -rf ${git_conf_dir}/${silab_dir}/v4px/* ${git_dir}/${silab_dir}
		fi
		;;
	3)
#TUNER - X0 
		mkdir -p ${git_dir}/${x0tuner_dir}/${lib_dir_name}
		cp -rf ${tcadt_local}/lib/libx0tunerdriver.so.*.*.* ${git_dir}/${x0tuner_dir}/${lib_dir_name}
		mkdir -p ${git_dir}/${x0tuner_dir}/src
		cp -rf tuner/src/x0tuner/*_core.h ${git_dir}/${x0tuner_dir}/src
		if [ ${config_v4px} -eq 0 ];then
			cp -rf ${git_conf_dir}/${x0tuner_dir}/default/* ${git_dir}/${x0tuner_dir}
		else
			cp -rf ${git_conf_dir}/${x0tuner_dir}/v4px/* ${git_dir}/${x0tuner_dir}
		fi
		;;
	4)
#TUNER - MAXIM
		mkdir -p ${git_dir}/${maxim_dir}/${lib_dir_name}
		cp -rf ${tcadt_local}/lib/libmax2175driver.so.*.*.* ${git_dir}/${maxim_dir}/${lib_dir_name}
		mkdir -p ${git_dir}/${maxim_dir}/src
		cp -rf tuner/src/max2175/*_core.h ${git_dir}/${maxim_dir}/src
		if [ ${config_v4px} -eq 0 ];then
			cp -rf ${git_conf_dir}/${maxim_dir}/default/* ${git_dir}/${maxim_dir}
		else
			cp -rf ${git_conf_dir}/${maxim_dir}/v4px/* ${git_dir}/${maxim_dir}
		fi
		;;
	5)
#TUNER - ST 
		mkdir -p ${git_dir}/${st_dir}/${lib_dir_name}
		cp -rf ${tcadt_local}/lib/libtda770xdriver.so.*.*.* ${git_dir}/${st_dir}/${lib_dir_name}
		mkdir -p ${git_dir}/${st_dir}/src
		cp -rf tuner/src/star/*_core.h ${git_dir}/${st_dir}/src
		if [ ${config_v4px} -eq 0 ];then
			cp -rf ${git_conf_dir}/${st_dir}/default/* ${git_dir}/${st_dir}
		else
			cp -rf ${git_conf_dir}/${st_dir}/v4px/* ${git_dir}/${st_dir}
		fi
		;;
	6)
#HAL
		mkdir -p ${git_dir}/${hal_dir}/${lib_dir_name}
		cp -rf ${tcadt_local}/lib/libtcradiohal.so.*.*.* ${git_dir}/${hal_dir}/${lib_dir_name}
		mkdir -p ${git_dir}/tc-radio-hal/src
		mkdir -p ${git_dir}/tc-radio-hal/src/fifo
		cp -rf hal/src/audio ${git_dir}/tc-radio-hal/src
		cp -rf hal/src/tcradio_hal.h ${git_dir}/tc-radio-hal/src
		cp -rf hal/src/fifo/tcradio_hal_fifo.h ${git_dir}/tc-radio-hal/src/fifo
		cp -rf hal/src/conf ${git_dir}/tc-radio-hal/src
		cp -rf hal/src/cui ${git_dir}/tc-radio-hal/src
		if [ ${config_v4px} -eq 0 ];then
			cp -rf ${git_conf_dir}/${hal_dir}/default/* ${git_dir}/${hal_dir}
		else
			cp -rf ${git_conf_dir}/${hal_dir}/v4px/* ${git_dir}/${hal_dir}
		fi
		;;
	7)
#MW
		mkdir -p ${git_dir}/${mw_dir}/${lib_dir_name}
		cp -rf ${tcadt_local}/lib/libtcradio.so.*.*.*  ${git_dir}/${mw_dir}/${lib_dir_name}
		mkdir -p ${git_dir}/tc-radio-mw/src/cui
		cp -rf mw/src/tcradio_api.h ${git_dir}/tc-radio-mw/src
		cp -rf mw/src/cui/ ${git_dir}/tc-radio-mw/src/
		cp -rf mw/src/rds/tcradio_rds_api.h ${git_dir}/tc-radio-mw/src
		if [ ${config_v4px} -eq 0 ];then
			cp -rf ${git_conf_dir}/${mw_dir}/default/* ${git_dir}/${mw_dir}
		else
			cp -rf ${git_conf_dir}/${mw_dir}/v4px/* ${git_dir}/${mw_dir}
		fi
		;;
	8)
#GUI
		if [ ${aarch64_en} -eq 1 ];then
			mkdir -p ${git_dir}/tc-radio-app/bin64
			cp -rf ${tcadt_local}/bin/TcRadioApp ${git_dir}/tc-radio-app/bin64/TcRadioApp
		else
			mkdir -p ${git_dir}/tc-radio-app/bin
			cp -rf ${tcadt_local}/bin/TcRadioApp ${git_dir}/tc-radio-app/bin/TcRadioApp
		fi
		cp -rf ${git_conf_dir}/${app_dir}/* ${git_dir}/${app_dir} 
		;;
	9)
#HD Radio
		echo -e "\033[33;1m"[HDR Type]"\033[0m"
		echo -e "\033[34;1m"1. HD1.0"\033[0m"
		echo -e "\033[34;1m"2. HD1.0+MRC"\033[0m"
		echo -e "\033[34;1m"3. HD1.5"\033[0m"
		echo -e "\033[34;1m"4. HD1.5+MRC"\033[0m"
		echo -e -n "\033[36;1m"Select HDR type \(1-4\) : "\033[0m"
		read input_hdrtype

		case $input_hdrtype in
			1)
			selhdrgit=${hd_dir};;
			2)
			selhdrgit=${hd10mrc_dir};;
			3)
			selhdrgit=${hd15_dir};;
			4)
			selhdrgit=${hd15mrc_dir};;
			*)
			echo "Invalid HDR Type!!!"
			exit;;
		esac

		mkdir -p ${git_dir}/${selhdrgit}/${lib_dir_name}
		rm -rf ${git_dir}/${selhdrgit}/${lib_dir_name}/libtchdradio.so*
		rm -rf ${git_dir}/${selhdrgit}/doc
		cp -rf ${tcadt_local}/lib/libtchdradio.so.*.*.*  ${git_dir}/${selhdrgit}/${lib_dir_name}
		filename=$(basename ${tcadt_local}/lib/libtchdradio.so.*.*.*)
		libname=$(echo $filename | cut -d. -f1-2)
		ln -s ${filename} ${git_dir}/${selhdrgit}/${lib_dir_name}/${libname}
		mkdir -p ${git_dir}/${selhdrgit}/src
		mkdir -p ${git_dir}/${selhdrgit}/src/api
		mkdir -p ${git_dir}/${selhdrgit}/src/example
		cp -rf hd/src/api/tchdr_aas.h ${git_dir}/${selhdrgit}/src/api
		cp -rf hd/src/api/tchdr_alert.h ${git_dir}/${selhdrgit}/src/api
		cp -rf hd/src/api/tchdr_api.h ${git_dir}/${selhdrgit}/src/api
		cp -rf hd/src/api/tchdr_callback.c ${git_dir}/${selhdrgit}/src/api
		cp -rf hd/src/api/tchdr_callback.h ${git_dir}/${selhdrgit}/src/api
		cp -rf hd/src/api/tchdr_psd.h ${git_dir}/${selhdrgit}/src/api
		cp -rf hd/src/api/tchdr_sig.h ${git_dir}/${selhdrgit}/src/api
		cp -rf hd/src/api/tchdr_sis.h ${git_dir}/${selhdrgit}/src/api
		cp -rf hd/src/api/tchdr_types.h ${git_dir}/${selhdrgit}/src/api
		cp -rf hd/src/example/* ${git_dir}/${selhdrgit}/src/example
		cp -rf hd/doc ${git_dir}/${selhdrgit}
		if [ ${config_v4px} -eq 0 ];then
			cp -rf ${git_conf_dir}/${hd_dir}/default/* ${git_dir}/${selhdrgit}
		else
			cp -rf ${git_conf_dir}/${hd_dir}/v4px/* ${git_dir}/${selhdrgit}
		fi
		;;
	10)
#HD MW
		mkdir -p ${git_dir}/${hd_mw_dir}/${lib_dir_name}
		cp -rf ${tcadt_local}/lib/libtcradio.so.*.*.*  ${git_dir}/${hd_mw_dir}/${lib_dir_name}
		mkdir -p ${git_dir}/${hd_mw_dir}/src/cui
		mkdir -p ${git_dir}/${hd_mw_dir}/src/submgr
		cp -rf mw/src/tcradio_api.h ${git_dir}/${hd_mw_dir}/src
		cp -rf mw/src/cui/ ${git_dir}/${hd_mw_dir}/src/
		cp -rf mw/src/rds/tcradio_rds_api.h ${git_dir}/${hd_mw_dir}/src
		cp -rf mw/src/hd/tcradio_hdr_if.h ${git_dir}/${hd_mw_dir}/src
		if [ ${config_v4px} -eq 0 ];then
			cp -rf ${git_conf_dir}/${hd_mw_dir}/default/* ${git_dir}/${hd_mw_dir}
		else
			cp -rf ${git_conf_dir}/${hd_mw_dir}/v4px/* ${git_dir}/${hd_mw_dir}
			cp -rf mw/src/submgr/*.* ${git_dir}/${hd_mw_dir}/src/submgr
		fi
		;;
	11)
#HD GUI
		if [ ${aarch64_en} -eq 1 ];then
			mkdir -p ${git_dir}/${hd_app_dir}/bin64
			cp -rf ${tcadt_local}/bin/TcRadioApp ${git_dir}/${hd_app_dir}/bin64/TcRadioApp
		else
			mkdir -p ${git_dir}/${hd_app_dir}/bin
			cp -rf ${tcadt_local}/bin/TcRadioApp ${git_dir}/${hd_app_dir}/bin/TcRadioApp
		fi
		cp -rf ${git_conf_dir}/${hd_app_dir}/* ${git_dir}/${hd_app_dir} 
		;;

	*)
		echo "Invalid Input!!!"
		exit;;
esac

