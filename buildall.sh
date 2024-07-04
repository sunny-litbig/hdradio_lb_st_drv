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
coverity_rule_misrac2012="--coding-standard-config ${coverity_rule_path}/misrac2012-telechips-210728.config"
coverity_rule_certc="--coding-standard-config ${coverity_rule_path}/cert-c-telechips-220708.config"
coverity_rule_certc_reco="--coding-standard-config ${coverity_rule_path}/cert-c-recommendation-telechips-221207.config"
coverity_rule_runtime="@@${coverity_rule_path}/runtime_rules_telechips_220708.txt"
coverity_commit="cov-commit-defects --dir idir --url ${coverity_url} --user ${coverity_id} --password ${coverity_pwd} --stream"

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

if [ ${MACHINE} =  "8030-main" ];then
	config_opt1="PROCESSOR=tcc803x "${MACHINE}
	echo -e "\033[33;1m"Processor = tcc803x Machine = 8030-main"\033[0m"
elif [ ${MACHINE} = "tcc8059-main" ];then
	config_opt1="PROCESSOR=tcc805x "${MACHINE}
	echo -e "\033[33;1m"Processor = tcc805x Machine = tcc8059-main"\033[0m"
elif [ ${MACHINE} = "tcc8053-main" ];then
	config_opt1="PROCESSOR=tcc805x "${MACHINE}
	echo -e "\033[33;1m"Processor = tcc805x Machine = tcc8053-main"\033[0m"
elif [ ${MACHINE} = "tcc8050-main" ];then
	config_opt1="PROCESSOR=tcc805x "${MACHINE}
	echo -e "\033[33;1m"Processor = tcc805x Machine = tcc8050-main"\033[0m"
elif [ ${MACHINE} = "tcc8031p-main" ];then
	config_opt1="PROCESSOR=tcc805x "${MACHINE}
	echo -e "\033[33;1m"Processor = tcc805x Machine = tcc8031p-main"\033[0m"
elif [ ${MACHINE} = "tcc8034p-main" ];then
	config_opt1="PROCESSOR=tcc805x "${MACHINE}
	echo -e "\033[33;1m"Processor = tcc805x Machine = tcc8034p-main"\033[0m"
elif [ ${MACHINE} = "tcc8070-main" ];then
	config_opt1="PROCESSOR=tcc807x "${MACHINE}
	echo -e "\033[33;1m"Processor = tcc807x Machine = tcc8070-main"\033[0m"
else
	echo -e "\033[33;1m"MACHINE is unknown. Please set MACHINE information."\033[0m"
	exit
fi

config_opt2="--enable-pulseaudio"
config_opt3="--enable-s0tuner"
config_opt5="--enable-hdradio"
config_opt6="USE_HDRADIO=YES"
echo -e "\033[33;1m"Option: Pulse Audio, Silab Tuner, HD Radio"\033[0m"

coverity_en=0
makestrip_en=1
makedebug_en=0

function build_peri()
{
		echo -e "\033[33;1m"    ====== starting build for PERI library... ======     "\033[0m"
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
		if [ ${coverity_en} -eq 1 ];then
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
		echo -e "\033[33;1m"    ====== starting build for TUNER library... ======     "\033[0m"
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
		if [ ${coverity_en} -eq 1 ];then
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
		echo -e "\033[33;1m"    ====== starting build for Tuner HAL library... ======     "\033[0m"
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
		if [ ${coverity_en} -eq 1 ];then
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
		echo -e "\033[33;1m"    ====== starting build for HD Radio Framework library... ======     "\033[0m"
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
		if [ ${coverity_en} -eq 1 ];then
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
		echo -e "\033[33;1m"    ====== starting build for HD Radio Framework library... ======     "\033[0m"
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
		if [ ${coverity_en} -eq 1 ];then
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
		echo -e "\033[33;1m"    ====== starting build for HD Radio Middleware library... ======     "\033[0m"
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
		if [ ${coverity_en} -eq 1 ];then
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
		echo -e "\033[33;1m"    ====== starting build for HD Radio App ... ======     "\033[0m"
		cd ${app_dir}
		qmake ${config_opt6}
		make clean
		make
		cp -a bin/TcRadioApp ${tcadt_local_dir}/bin
}

function build_main_manager()
{
		echo -e "\033[33;1m"    ====== starting build for Main Manager library... ======     "\033[0m"
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
		if [ ${coverity_en} -eq 1 ];then
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
		echo -e "\033[33;1m"    ====== starting build for Sub Manager library... ======     "\033[0m"
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
		if [ ${coverity_en} -eq 1 ];then
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

rm -rf ${tcadt_local_dir}
build_peri
cd ../../
build_tuner
cd ../../
build_hal
cd ../../
build_hd
cd ../../
build_mw
rm -rf ${tcadt_local_dir}/lib/*.la
echo -e "\033[36;1m"    ====== BUILD COMPLETE!!! ======    "\033[0m"

