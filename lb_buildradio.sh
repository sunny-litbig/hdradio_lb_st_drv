#!/bin/bash

project_name="hdr_15mrc"

#The coverage rule path must be an absolute path.
coverity_id="telechips12"
coverity_pwd="telechips12"
coverity_url="https://coverity.telechips.com:8443"
coverity_rule_path="/home/coverity/cov-analysis-linux64/telechips-config/latest-release"
coverity_build_cmd="cov-build --dir idir --emit-complementary-info"
coverity_analyze_cmd_tu_pattern="cov-analyze --dir idir --strip-path `pwd` --disable-default"
coverity_analyze_cmd="cov-analyze --dir idir --strip-path `pwd` --disable-default"
coverity_rule_misrac2012="--coding-standard-config ${coverity_rule_path}/misrac2012-telechips.config"
coverity_rule_certc="--coding-standard-config ${coverity_rule_path}/cert-c-telechips.config"
coverity_rule_certc_reco="--coding-standard-config ${coverity_rule_path}/cert-c-recommendation-telechips.config"
coverity_rule_runtime="@@${coverity_rule_path}/runtime_rules_telechips.txt"
coverity_commit="cov-commit-defects --dir idir --url ${coverity_url} --user ${coverity_id} --password ${coverity_pwd} --stream"

#Codesonar setting
codesonar_prefix="codesonar analyze"
codesonar_suffix="-preset telechips 192.168.33.105:7340"

tcadt_strip=${OECORE_NATIVE_SYSROOT}/usr/bin/arm-telechips-linux-gnueabi/arm-telechips-linux-gnueabi-strip
tcadt_local_dir=${OECORE_TARGET_SYSROOT}/usr/local

setup_file="setup.sh"
peri_dir="peri"
tuner_dir="tuner"
hal_dir="hal"
demod_dir="demod"
mw_dir="mw"
app_dir="app"
git_conf_dir="gitconf"

hd_dir="hd"
hd_core_dir="hdcore"
submgr_dir="submgr"
mainmgr_dir="tc-radio-hd-manager"

build_dir="build"

libperi_name="libtcradioperi.so*"
libsi4796xdrv_name="libsi4796xdriver.so*"
libx0tunerdrv_name="libx0tunerdriver.so*"
libmax2175drv_name="libmax2175driver.so*"
libhal_name="libtcradiohal.so*"
libhalif_name="libtcradiohalif.so*"
libmw_name="libtcradio.so*"
liball_name="lib*.so*"

tcc897x_brd="PROCESSOR=tcc897x MACHINE=tcc8971-lcn-2.0a"
tcc897x_lcn30_brd="PROCESSOR=tcc897x MACHINE=tcc8971-lcn-3.0"
tcc802x_brd="PROCESSOR=tcc802x MACHINE=tcc8021"
tcc802x_evm20_brd="PROCESSOR=tcc802x MACHINE=tcc8021-evm-2.0"
tcc802x_evm21_brd="PROCESSOR=tcc802x MACHINE=tcc8021-evm-2.1"
tcc8030_brd="PROCESSOR=tcc803x MACHINE=tcc8030"
tcc8031_brd="PROCESSOR=tcc803x MACHINE=tcc8031"
tcc8059_main_brd="PROCESSOR=tcc805x MACHINE=tcc8059-main"
tcc8059_sub_brd="PROCESSOR=tcc805x MACHINE=tcc8059-sub"
tcc8053_main_brd="PROCESSOR=tcc805x MACHINE=tcc8053-main"
tcc8053_sub_brd="PROCESSOR=tcc805x MACHINE=tcc8053-sub"
tcc8050_main_brd="PROCESSOR=tcc805x MACHINE=tcc8050-main"
tcc8050_sub_brd="PROCESSOR=tcc805x MACHINE=tcc8050-sub"
tcc8031p_main_brd="PROCESSOR=tcc805x MACHINE=tcc8031p-main"
tcc8034p_main_brd="PROCESSOR=tcc805x MACHINE=tcc8034p-main"
tcc8070_main_brd="PROCESSOR=tcc807x MACHINE=tcc8070-main"

config_hd_build=1
config_opt5="--enable-hdradio"
config_opt6="USE_HDRADIO=YES"

#Check Core, ARCH and mfloat
if [ ${ARCH} ];then
	if [[ "$CC" == *"cortex-a76"* ]];then
		echo -e "\033[33;1m"[Cortex-A76]"\033[0m"
	elif [[ "$CC" == *"cortex-a72"* ]];then
		echo -e "\033[33;1m"[Cortex-A72]"\033[0m"
	elif [[ "$CC" == *"cortex-a53"* ]];then
		echo -e "\033[33;1m"[Cortex-A53]"\033[0m"
	elif [[ "$CC" == *"cortex-a7"* ]];then
		echo -e "\033[33;1m"[Cortex-A7]"\033[0m"
	else
		echo -e "\033[33;1m"[Unknown-Core]"\033[0m"
	fi

	if [ ${ARCH} = "arm64" ];then
		config_opt4="--enable-aarch64"
		echo -e "\033[33;1m"ADT is AARCH64\(64bit\)"\033[0m"
	else
		if [[ "$CC" == *"-mfloat-abi=hard"* ]];then
			config_opt4="--enable-arch-hardfp"
			echo -e "\033[33;1m"ADT is 32bit hardfp"\033[0m"
		else
			config_opt4="--enable-arch-softfp"
			echo -e "\033[33;1m"ADT is 32bit softfp"\033[0m"
		fi
	fi
else
	echo -e "\033[31;1m"Please set the ADT!!!"\033[0m"
	exit
fi

echo -e "\033[33;1m"[TCRadio Job]"\033[0m"
echo -e "\033[34;1m"1. Build TCRadio"\033[0m"
echo -e "\033[34;1m"2. Delete All Output Files \(${tcadt_local_dir}\)"\033[0m"
echo -e "\033[34;1m"3. Source Code Compression For Backup"\033[0m"
echo -e -n "\033[36;1m"Select your job \(1-3\) : "\033[0m"
read input0

case $input0 in
	1)
		;;
	2)
		rm -rf ${tcadt_local_dir}
		exit;;
	3)
		tar -zcvf tcradio_src_$(date +%Y%m%d)_$(date +%H%M).tar.gz ./*.sh ./readme.txt ./.gitignore ./*.bp ./*.go \
										  ${peri_dir}/src ${peri_dir}/configure.ac ${peri_dir}/Makefile.am ${peri_dir}/*.bp ${peri_dir}/*.go \
										  ${tuner_dir}/src ${tuner_dir}/configure.ac ${tuner_dir}/Makefile.am ${tuner_dir}/*.bp ${tuner_dir}/*.go \
										  ${hal_dir}/src ${hal_dir}/configure.ac ${hal_dir}/Makefile.am ${hal_dir}/*.bp ${hal_dir}/*.go \
										  ${mw_dir}/src ${mw_dir}/configure.ac ${mw_dir}/Makefile.am ${mw_dir}/*.bp ${mw_dir}/*.go \
										  ${app_dir}/res ${app_dir}/src ${app_dir}/*.pro \
										  ${git_conf_dir}/ \
										  ${hd_dir}/src ${hd_dir}/lib ${hd_dir}/lib64  ${hd_dir}/doc ${hd_dir}/configure.ac ${hd_dir}/Makefile.am ${hd_dir}/*.bp ${hd_dir}/*.go \
										  ${mainmgr_dir}/src ${mainmgr_dir}/configure.ac ${mainmgr_dir}/Makefile.am \
										  ${hd_core_dir}/src ${hd_core_dir}/configure.ac ${hd_core_dir}/Makefile.am ${hd_core_dir}/makelib.sh

		exit;;
	*)
		echo -e "\033[31;1m"Invalid Input!!!"\033[0m"
		exit;;
esac

echo -e "\033[33;1m"[MACHINE]"\033[0m"
echo -e "\033[34;1m"1. TCC897x LCN2.0a"\033[0m"
echo -e "\033[34;1m"2. TCC897x LCN3.0"\033[0m"
echo -e "\033[34;1m"3. TCC802x"\033[0m"
echo -e "\033[34;1m"4. TCC802x EVM2.1"\033[0m"
echo -e "\033[34;1m"5. TCC8030"\033[0m"
echo -e "\033[34;1m"6. TCC8031"\033[0m"
echo -e "\033[34;1m"7. TCC8059 Main-Core"\033[0m"
echo -e "\033[34;1m"8. TCC8059 Sub-Core"\033[0m"
echo -e "\033[34;1m"9. TCC8053 Main-Core"\033[0m"
echo -e "\033[34;1m"10. TCC8053 Sub-Core"\033[0m"
echo -e "\033[34;1m"11. TCC8050 Main-Core"\033[0m"
echo -e "\033[34;1m"12. TCC8050 Sub-Core"\033[0m"
echo -e "\033[34;1m"13. TCC8031P Main-Core"\033[0m"
echo -e "\033[34;1m"14. TCC8034P Main-Core"\033[0m"
echo -e "\033[34;1m"15. TCC8070 Main-Core"\033[0m"
echo -e -n "\033[36;1m"Select your machine \(1-15\) : "\033[0m"
read input1

case $input1 in
	1)
		config_opt1=${tcc897x_brd};;
	2)
		config_opt1=${tcc897x_lcn30_brd};;
	3)
		config_opt1=${tcc802x_brd};;
	4)
		config_opt1=${tcc802x_evm21_brd};;
	5)
		config_opt1=${tcc8030_brd};;
	6)
		config_opt1=${tcc8031_brd};;
	7)
		config_opt1=${tcc8059_main_brd};;
	8)
		config_opt1=${tcc8059_sub_brd};;
	9)
		config_opt1=${tcc8053_main_brd};;
	10)
		config_opt1=${tcc8053_sub_brd};;
	11)
		config_opt1=${tcc8050_main_brd};;
	12)
		config_opt1=${tcc8050_sub_brd};;
	13)
		config_opt1=${tcc8031p_main_brd};;
	14)
		config_opt1=${tcc8034p_main_brd};;
	15)
		config_opt1=${tcc8070_main_brd};;
	*)
		echo -e "\033[31;1m"Invalid Input!!!"\033[0m"
		exit;;
esac

echo -e "\033[33;1m"[Audio Service]"\033[0m"
echo -e "\033[34;1m"1. Pulse Audio"\033[0m"
echo -e "\033[34;1m"2. TC Audio"\033[0m"
echo -e -n "\033[36;1m"Select your audio driver \(1-2\) : "\033[0m"
read input2

case $input2 in
	1)
		config_opt2="--enable-pulseaudio";;
	2)
		config_opt2="";;
	*)
		echo -e "\033[31;1m"Invalid Input!!!"\033[0m"
		exit;;
esac

echo -e "\033[33;1m"[Tuner Driver]"\033[0m"
echo -e "\033[34;1m"1. Include All Tuner Driver"\033[0m"
echo -e "\033[34;1m"2. Si4796x"\033[0m"
echo -e "\033[34;1m"3. TDA770x"\033[0m"
echo -e "\033[34;1m"4. X0Tuner"\033[0m"
echo -e "\033[34;1m"5. MAX2175\(SDR\)"\033[0m"
echo -e -n "\033[36;1m"Select tuner driver \(1-5\) : "\033[0m"
read input3

case $input3 in
	1)
		config_opt3="--enable-s0tuner --enable-x0tuner --enable-m0tuner";;
	2)
		config_opt3="--enable-s0tuner";;
	3)
		config_opt3="--enable-t0tuner";;
	4)
		config_opt3="--enable-x0tuner";;
	5)
		config_opt3="--enable-m0tuner";;
	*)
		echo -e "\033[31;1m"Invalid Input!!!"\033[0m"
		exit;;
esac

echo -e "\033[33;1m"[Codesonar Make Option]"\033[0m"
echo -e "\033[34;1m"1. No Make Coverity"\033[0m"
echo -e "\033[34;1m"2. Make Coverity"\033[0m"
echo -e "\033[34;1m"3. Make Codesonar MISRA-C"\033[0m"
echo -e -n "\033[36;1m"Select Codesonar Option \(1-4\) : "\033[0m"
read input4

case $input4 in
	2)
		coverity_en=1
		echo -e "\033[32;1m"Make Coverity!!!"\033[0m"
		;;
	3)
		codesonar_en=1
		echo -e "\033[32;1m"Make Codesonar MISRA-C!!!"\033[0m"
		;;
	*)
		codesonar_en=0
		coverity_en=0
		echo -e "\033[32;1m"Not Make Codesonar/Coverity!!!"\033[0m"
		;;
esac

echo -e "\033[33;1m"[Make Strip Option]"\033[0m"
echo -e "\033[34;1m"1. Make Strip"\033[0m"
echo -e "\033[34;1m"2. No Make Strip"\033[0m"
echo -e -n "\033[36;1m"Select Strip Option \(1-2\) : "\033[0m"
read input4

case $input4 in
	2)
		makestrip_en=0
		echo -e "\033[32;1m"No Make Strip!!!"\033[0m"
		;;
	*)
		makestrip_en=1
		echo -e "\033[32;1m"Make Strip!!!"\033[0m"
		;;
esac

echo -e "\033[33;1m"[Make Debug Option]"\033[0m"
echo -e "\033[34;1m"1. Normal Make Option"\033[0m"
echo -e "\033[34;1m"2. Enable Debug Option"\033[0m"
echo -e -n "\033[36;1m"Select Debug Option \(1-2\) : "\033[0m"
read input5

case $input5 in
	2)
		makedebug_en=1
		echo -e "\033[32;1m"Enable Debug Option!!!"\033[0m"
		;;
	*)
		makedebug_en=0
		echo -e "\033[32;1m"Normal Make Option!!!"\033[0m"
		;;
esac

while :
do
echo -e "\033[33;1m"[Build Option]"\033[0m"
echo -e "\033[34;1m"1. Build Peripheral Devices"\033[0m"
echo -e "\033[34;1m"2. Build Tuner Drivers"\033[0m"
echo -e "\033[34;1m"3. Build HAL"\033[0m"
echo -e "\033[34;1m"4. Build DEMOD"\033[0m"
echo -e "\033[34;1m"5. Build Middleware \(included CUI App\)"\033[0m"
echo -e "\033[34;1m"6. QMAKE GUI Appication"\033[0m"
echo -e "\033[34;1m"7. Build Main-Core Manager for TCC805x"\033[0m"
echo -e "\033[34;1m"9. Build All \(Delete output files and build\)"\033[0m"
echo -e -n "\033[36;1m"Select build option \(1-9\) : "\033[0m"
read input5

function build_peri()
{
		cd ${peri_dir}
		../${setup_file}
		if [ -d ${build_dir} ];then
			rm -rf ${build_dir}
		fi
		mkdir ${build_dir}
		cd ${build_dir}
		if [ ${makedebug_en} -eq 1 ];then
			../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt2} CPPFLAGS=-DDEBUG CFLAGS="-g" 
		else
			../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt2}
		fi
		make clean
		if [ ${codesonar_en} -eq 1 ];then
			${codesonar_prefix} ${project_name}_peri ${codesonar_suffix} make
		elif [ ${coverity_en} -eq 1 ];then
			${coverity_build_cmd} make
			${coverity_analyze_cmd} ${coverity_rule_runtime}
			${coverity_analyze_cmd_tu_pattern} ${coverity_rule_misrac2012} ${coverity_rule_certc} ${coverity_rule_certc_reco}
			${coverity_commit} ${project_name}_peri
		else
			make -j16
		fi
		if [ ${makestrip_en} -eq 1 ];then
			make install-strip DESTDIR=$SDKTARGETSYSROOT
		else
			make install DESTDIR=$SDKTARGETSYSROOT
		fi
}

function build_tuner()
{
		cd ${tuner_dir}
		../${setup_file}
		if [ -d ${build_dir} ];then
			rm -rf ${build_dir}
		fi
		mkdir ${build_dir}
		cd ${build_dir}
		if [ ${makedebug_en} -eq 1 ];then
			../configure $CONFIGURE_FLAGS ${config_opt1} CPPFLAGS=-DDEBUG CFLAGS="-g" 
		else
			../configure $CONFIGURE_FLAGS ${config_opt1}
		fi
		make clean
		if [ ${codesonar_en} -eq 1 ];then
			${codesonar_prefix} ${project_name}_tunerdrv ${codesonar_suffix} make
		elif [ ${coverity_en} -eq 1 ];then
			${coverity_build_cmd} make
			${coverity_analyze_cmd} ${coverity_rule_runtime}
			${coverity_analyze_cmd_tu_pattern} ${coverity_rule_misrac2012} ${coverity_rule_certc} ${coverity_rule_certc_reco}
			${coverity_commit} ${project_name}_tuner
		else
			make -j16
		fi
		if [ ${makestrip_en} -eq 1 ];then
			make install-strip DESTDIR=$SDKTARGETSYSROOT
		else
			make install DESTDIR=$SDKTARGETSYSROOT
		fi
}

function build_hal()
{
		cd ${hal_dir}
		../${setup_file}
		if [ -d ${build_dir} ];then
			rm -rf ${build_dir}
		fi
		mkdir ${build_dir}
		cd ${build_dir}
		if [ ${makedebug_en} -eq 1 ];then
			../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt2} ${config_opt3} ${config_opt5} CPPFLAGS=-DDEBUG CFLAGS="-g" 
		else
			../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt2} ${config_opt3} ${config_opt5}
		fi
		make clean
		if [ ${codesonar_en} -eq 1 ];then
			${codesonar_prefix} ${project_name}_hal ${codesonar_suffix} make
		elif [ ${coverity_en} -eq 1 ];then
			${coverity_build_cmd} make
			${coverity_analyze_cmd} ${coverity_rule_runtime}
			${coverity_analyze_cmd_tu_pattern} ${coverity_rule_misrac2012} ${coverity_rule_certc} ${coverity_rule_certc_reco}
			${coverity_commit} ${project_name}_hal
		else
			make -j16
		fi
		if [ ${makestrip_en} -eq 1 ];then
			make install-strip DESTDIR=$SDKTARGETSYSROOT
		else
			make install DESTDIR=$SDKTARGETSYSROOT
		fi
}

function build_hd()
{
		cd ${hd_dir}
		../${setup_file}
		if [ -d ${buid_dir} ];then
			rm -rf ${build_dir}
		fi
		mkdir ${build_dir}
		cd ${build_dir}
		if [ ${makedebug_en} -eq 1 ];then
			../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt4} --enable-tcevb --enable-hdrcui CPPFLAGS=-DDEBUG CFLAGS="-g"
		else
			../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt4} --enable-tcevb --enable-hdrcui
		fi
		if [ ${codesonar_en} -eq 1 ];then
			${codesonar_prefix} ${project_name}_hdradio ${codesonar_suffix} make
		elif [ ${coverity_en} -eq 1 ];then
			${coverity_build_cmd} make
			${coverity_analyze_cmd} ${coverity_rule_runtime}
			${coverity_analyze_cmd} --tu-pattern="file('.*\.c$')" ${coverity_rule_misrac2012} ${coverity_rule_certc} ${coverity_rule_certc_reco}
			${coverity_commit} ${project_name}_framework
		else
			make -j16
		fi
		if [ ${makestrip_en} -eq 1 ];then
			make install-strip DESTDIR=$SDKTARGETSYSROOT
		else
			make install DESTDIR=$SDKTARGETSYSROOT
		fi
}

function resume_build_hd()
{
		cd ${hd_dir}
		../${setup_file}
		if [ -d ${buid_dir} ];then
			cd ${build_dir}
		else
			return
		fi
		if [ ${makedebug_en} -eq 1 ];then
			../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt4} --enable-tcevb --enable-hdrcui CPPFLAGS=-DDEBUG CFLAGS="-g"
		else
			../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt4} --enable-tcevb --enable-hdrcui
		fi
		if [ ${codesonar_en} -eq 1 ];then
			${codesonar_prefix} ${project_name}_hdradio ${codesonar_suffix} make
		elif [ ${coverity_en} -eq 1 ];then
			${coverity_build_cmd} make
			${coverity_analyze_cmd} ${coverity_rule_runtime}
			${coverity_analyze_cmd} --tu-pattern="file('.*\.c$')" ${coverity_rule_misrac2012} ${coverity_rule_certc} ${coverity_rule_certc_reco}
			${coverity_commit} ${project_name}_framework
		else
			make -j16
		fi
		if [ ${makestrip_en} -eq 1 ];then
			make install-strip DESTDIR=$SDKTARGETSYSROOT
		else
			make install DESTDIR=$SDKTARGETSYSROOT
		fi
}

function build_mw()
{
		cd ${mw_dir}
		../${setup_file}
		if [ -d ${build_dir} ];then
			rm -rf ${build_dir}
		fi
		mkdir ${build_dir}
		cd ${build_dir}
		if [ ${makedebug_en} -eq 1 ];then
			../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt5} CPPFLAGS=-DDEBUG CFLAGS="-g" 
		else
			../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt5}
		fi
		make clean
		if [ ${codesonar_en} -eq 1 ];then
			${codesonar_prefix} ${project_name}_mw ${codesonar_suffix} make
		elif [ ${coverity_en} -eq 1 ];then
			${coverity_build_cmd} make
			${coverity_analyze_cmd} ${coverity_rule_runtime}
			${coverity_analyze_cmd_tu_pattern} ${coverity_rule_misrac2012} ${coverity_rule_certc} ${coverity_rule_certc_reco}
			${coverity_commit} ${project_name}_mw
		else
			make
		fi
		if [ ${makestrip_en} -eq 1 ];then
			make install-strip DESTDIR=$SDKTARGETSYSROOT
		else
			make install DESTDIR=$SDKTARGETSYSROOT
		fi
}

function build_mw_a()
{
		cd ${mw_dir}
		../${setup_file}
		if [ -d ${build_dir} ];then
			rm -rf ${build_dir}
		fi
		mkdir ${build_dir}
		cd ${build_dir}
		if [ ${makedebug_en} -eq 1 ];then
			../configure $CONFIGURE_FLAGS ${config_opt1} CPPFLAGS=-DDEBUG CFLAGS="-g" 
		else
			../configure $CONFIGURE_FLAGS ${config_opt1}
		fi
		make clean
		if [ ${codesonar_en} -eq 1 ];then
			${codesonar_prefix} ${project_name}_mw ${codesonar_suffix} make
		elif [ ${coverity_en} -eq 1 ];then
			${coverity_build_cmd} make
			${coverity_analyze_cmd} ${coverity_rule_runtime}
			${coverity_analyze_cmd_tu_pattern} ${coverity_rule_misrac2012} ${coverity_rule_certc} ${coverity_rule_certc_reco}
			${coverity_commit} ${project_name}_mw
		else
			make
		fi
		if [ ${makestrip_en} -eq 1 ];then
			make install-strip DESTDIR=$SDKTARGETSYSROOT
		else
			make install DESTDIR=$SDKTARGETSYSROOT
		fi
}

function build_app()
{
		cd ${app_dir}
		qmake ${config_opt6}
		make clean
		if [ ${codesonar_en} -eq 1 ];then
			${codesonar_prefix} ${project_name}_app ${codesonar_suffix} make
		else
			make
		fi
		
		cp -a bin/TcRadioApp ${tcadt_local_dir}/bin
}

function build_main_manager()
{
		cd ${mainmgr_dir}
		../${setup_file}
		if [ -d ${build_dir} ];then
			rm -rf ${build_dir}
		fi
		mkdir ${build_dir}
		cd ${build_dir}
		if [ ${makedebug_en} -eq 1 ];then
			../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt5} CPPFLAGS=-DDEBUG CFLAGS="-g" 
		else
			../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt5}
		fi
		make clean
		if [ ${codesonar_en} -eq 1 ];then
			${codesonar_prefix} ${project_name}_mw ${codesonar_suffix} make
		elif [ ${coverity_en} -eq 1 ];then
			${coverity_build_cmd} make
			${coverity_analyze_cmd} ${coverity_rule_runtime}
			${coverity_analyze_cmd_tu_pattern} ${coverity_rule_misrac2012} ${coverity_rule_certc} ${coverity_rule_certc_reco}
			${coverity_commit} ${project_name}_main_manager
		else
			make
		fi
		if [ ${makestrip_en} -eq 1 ];then
			make install-strip DESTDIR=$SDKTARGETSYSROOT
		else
			make install DESTDIR=$SDKTARGETSYSROOT
		fi
}

function build_sub_manager()
{
		cd ${submgr_dir}
		../${setup_file}
		if [ -d ${build_dir} ];then
			rm -rf ${build_dir}
		fi
		mkdir ${build_dir}
		cd ${build_dir}
		if [ ${makedebug_en} -eq 1 ];then
			../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt5} CPPFLAGS=-DDEBUG CFLAGS="-g" 
		else
			../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt5}
		fi
		make clean
		if [ ${codesonar_en} -eq 1 ];then
			${codesonar_prefix} ${project_name}_mw ${codesonar_suffix} make
		elif [ ${coverity_en} -eq 1 ];then
			${coverity_build_cmd} make
			${coverity_analyze_cmd} ${coverity_rule_runtime}
			${coverity_analyze_cmd_tu_pattern} ${coverity_rule_misrac2012} ${coverity_rule_certc} ${coverity_rule_certc_reco}
			${coverity_commit} ${project_name}_sub_manager
		else
			make
		fi
		if [ ${makestrip_en} -eq 1 ];then
			make install-strip DESTDIR=$SDKTARGETSYSROOT
		else
			make install DESTDIR=$SDKTARGETSYSROOT
		fi
}

case $input5 in
	1)
# PERI
		build_peri
		break;;
	2)
# TUNER DRIVERS
		build_tuner
		break;;
	3)
#HAL
		build_hal
		break;;
	4)
#HD Radio DEMOD
		build_hd
#		resume_build_hd
		break;;
	5)
#MIDDLEWARE
		build_mw
		break;;
	6)
#GUI APPLICATION
		build_app
		break;;
	7)
#MAIN-Core Manager
		build_main_manager
		break;;

	8)
#SUB-Core Manager
		build_sub_manager
		break;;

	9)
		# rm -rf ${tcadt_local_dir}
		build_peri
		cd ../../
		build_tuner
		cd ../../
		build_hal
		if [ ${config_hd_build} -eq 1 ];then
		    cd ../../
		    build_hd
        fi
		cd ../../
		build_mw
		cd ../../
		build_mw_a
		echo -e "\033[36;1m"BUILD COMPLETE!!!"\033[0m"
		break;;

	*)
		echo -e "\033[31;1m"Invalid Input!!!"\033[0m"
		exit;;
esac
done

rm -rf ${tcadt_local_dir}/lib/*.la
