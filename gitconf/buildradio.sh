#!/bin/bash

tcadt_strip=${OECORE_NATIVE_SYSROOT}/usr/bin/arm-telechips-linux-gnueabi/arm-telechips-linux-gnueabi-strip
tcadt_local_dir=${OECORE_TARGET_SYSROOT}/usr/local

setup_file="setup.sh"
peri_dir="tc-radio-drv-peri"
silab_dir="tc-radio-drv-silab"
x0tuner_dir="tc-radio-drv-x0"
maxim_dir="tc-radio-drv-maxim"
fmam_dir="tc-radio-demod-fmam"
hdr_dir="tc-radio-hd"
hal_dir="tc-radio-hal"
mw_dir="tc-radio-mw"
app_dir="tc-radio-app"
gui_dir="tc-radio-gui"
hd_mw_dir="tc-radio-hd-mw"
hd_app_dir="tc-radio-hd-app"

build_dir="build"

tcc897x_brd="PROCESSOR=tcc897x MACHINE=tcc8971-lcn-2.0a"
tcc897x_lcn30_brd="PROCESSOR=tcc897x MACHINE=tcc8971-lcn-3.0"
tcc802x_brd="PROCESSOR=tcc802x MACHINE=tcc8021"
tcc802x_evm21_brd="PROCESSOR=tcc802x MACHINE=tcc8021-evm-2.1"
tcc803x_brd="PROCESSOR=tcc803x MACHINE=tcc8030"
tcc8059_main_brd="PROCESSOR=tcc805x MACHINE=tcc8059-main"
tcc8059_sub_brd="PROCESSOR=tcc805x MACHINE=tcc8059-sub"
tcc8053_main_brd="PROCESSOR=tcc805x MACHINE=tcc8053-main"
tcc8053_sub_brd="PROCESSOR=tcc805x MACHINE=tcc8053-sub"
tcc8050_main_brd="PROCESSOR=tcc805x MACHINE=tcc8050-main"
tcc8050_sub_brd="PROCESSOR=tcc805x MACHINE=tcc8050-sub"
tcc8070_main_brd="PROCESSOR=tcc807x MACHINE=tcc8070-main"

if [ ${ARCH} ];then
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
echo -e "\033[34;1m"2. Delete All Output Files \(\$\{ADT\}/user/local\)"\033[0m"
echo -e "\033[34;1m"3. Git Structure Compression For Backup"\033[0m"
echo -e -n "\033[36;1m"Select your job \(1-3\) : "\033[0m"
read input0

case $input0 in
	1)
		;;
	2)
		rm -rf ${tcadt_local_dir}
		exit;;
	3)
		tar -zcvf tcradio_git_$(date +%Y%m%d)_$(date +%H%M).tar.gz ./*.sh ./readme.txt \
										  ${peri_dir}/src ${peri_dir}/configure.ac ${peri_dir}/Makefile.am ${peri_dir}/.gitignore \
										  ${silab_dir}/bin ${silab_dir}/lib ${silab_dir}/lib64 ${silab_dir}/configure.ac ${silab_dir}/Makefile.am ${silab_dir}/.gitignore \
										  ${x0tuner_dir}/lib ${x0tuner_dir}/lib64 ${x0tuner_dir}/configure.ac ${x0tuner_dir}/Makefile.am ${x0tuner_dir}/.gitignore \
										  ${maxim_dir}/lib ${maxim_dir}/lib64 ${maxim_dir}/configure.ac ${maxim_dir}/Makefile.am ${maxim_dir}/.gitignore \
										  ${hal_dir}/src ${hal_dir}/lib ${hal_dir}/lib64 ${hal_dir}/configure.ac ${hal_dir}/Makefile.am ${hal_dir}/.gitignore \
										  ${mw_dir}/src ${mw_dir}/lib ${mw_dir}/lib64 ${mw_dir}/configure.ac ${mw_dir}/Makefile.am ${mw_dir}/.gitignore \
										  ${app_dir}/bin ${app_dir}/bin64 ${app_dir}/configure.ac ${app_dir}/Makefile.am \
										  ${hdr_dir}/lib ${hdr_dir}/src ${hdr_dir}/configure.ac ${hdr_dir}/Makefile.am

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
echo -e "\033[34;1m"5. TCC803x CV0.1"\033[0m"
echo -e "\033[34;1m"6. TCC8059 Main-Core"\033[0m"
echo -e "\033[34;1m"7. TCC8059 Sub-Core"\033[0m"
echo -e "\033[34;1m"8. TCC8053 Main-Core"\033[0m"
echo -e "\033[34;1m"9. TCC8053 Sub-Core"\033[0m"
echo -e "\033[34;1m"10. TCC8050 Main-Core"\033[0m"
echo -e "\033[34;1m"11. TCC8050 Sub-Core"\033[0m"
echo -e "\033[34;1m"12. TCC8070 Main-Core"\033[0m"
echo -e -n "\033[36;1m"Select your machine \(1-12\) : "\033[0m"
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
		config_opt1=${tcc803x_brd};;
    6)
        config_opt1=${tcc8059_main_brd};;
    7)
        config_opt1=${tcc8059_sub_brd};;
    8)
        config_opt1=${tcc8053_main_brd};;
    9)
        config_opt1=${tcc8053_sub_brd};;
    10)
        config_opt1=${tcc8050_main_brd};;
    11)
        config_opt1=${tcc8050_sub_brd};;
    12)
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
echo -e "\033[34;1m"3. X0Tuner"\033[0m"
echo -e "\033[34;1m"4. MAX2175\(SDR\)"\033[0m"
echo -e -n "\033[36;1m"Select tuner driver \(1-4\) : "\033[0m"
read input3

case $input3 in
	1)
		config_opt3="--enable-s0tuner --enable-x0tuner --enable-m0tuner";;
	2)
		config_opt3="--enable-s0tuner";;
	3)
		config_opt3="--enable-x0tuner";;
	4)
		config_opt3="--enable-m0tuner";;
	*)
		echo -e "\033[31;1m"Invalid Input!!!"\033[0m"
		exit;;
esac

echo -e "\033[33;1m"[HD Radio]"\033[0m"
echo -e "\033[34;1m"1. FM/AM/HD Radio"\033[0m"
echo -e "\033[34;1m"2. FM/AM Radio"\033[0m"
echo -e -n "\033[36;1m"Select your radio type \(1-2\) : "\033[0m"
read input5

case $input5 in
	1)
		config_opt5="--enable-hdradio"
		config_opt6="USE_HDRADIO=YES";;
	2)
		config_opt5=""
		config_opt6="";;
	*)
		echo -e "\033[31;1m"Invalid Input!!!"\033[0m"
		exit;;
esac

while :
do
echo -e "\033[33;1m"[Build Option]"\033[0m"
echo -e "\033[34;1m"1. Build Peripheral Devices"\033[0m"
echo -e "\033[34;1m"2. Build Silab Tuner Driver"\033[0m"
echo -e "\033[34;1m"3. Build X0 Tuner Driver"\033[0m"
echo -e "\033[34;1m"4. Build Maxim Tuner Driver"\033[0m"
echo -e "\033[34;1m"5. Build FMAM SDR Demod"\033[0m"
echo -e "\033[34;1m"6. Build HDR SDR Demod"\033[0m"
echo -e "\033[34;1m"7. Build HAL"\033[0m"
echo -e "\033[34;1m"8. Build Middleware \(include CUI App\)"\033[0m"
echo -e "\033[34;1m"9. Build GUI\(binary\) Application"\033[0m"
echo -e "\033[34;1m"10. Build HDR Middleware \(include CUI App\)"\033[0m"
echo -e "\033[34;1m"11. Build HDR GUI\(binary\) Application"\033[0m"
echo -e "\033[34;1m"12. Build GUI\(source\) Application"\033[0m"
echo -e "\033[34;1m"13. Build All \(not ready\)"\033[0m"
echo -e "\033[34;1m"14. Make All \(not ready\)"\033[0m"
echo -e -n "\033[36;1m"Select build option \(1-14\) : "\033[0m"
read input4

export LC_ALL=C

case $input4 in
	1)
# PERI
		cd ${peri_dir}
		../${setup_file}
		if [ -d ${build_dir} ];then
			rm -rf ${build_dir}
		fi
		mkdir ${build_dir}
		cd ${build_dir}
		../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt2} ${config_opt4}
		make clean
		make -j16
		make install-strip DESTDIR=$SDKTARGETSYSROOT
		break;;
	2)
# Silab Tuner
		cd ${silab_dir}
		../${setup_file}
		if [ -d ${build_dir} ];then
			rm -rf ${build_dir}
		fi
		mkdir ${build_dir}
		cd ${build_dir}
		../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt4}
		make clean
#		make -j16
		make
		make install-strip DESTDIR=$SDKTARGETSYSROOT
		break;;
	3)
# X0 Tuner
		cd ${x0tuner_dir}
		../${setup_file}
		if [ -d ${build_dir} ];then
			rm -rf ${build_dir}
		fi
		mkdir ${build_dir}
		cd ${build_dir}
		../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt4}
		make clean
		make -j16
		make install-strip DESTDIR=$SDKTARGETSYSROOT
		break;;
	4)
# Maxim Tuner
		cd ${maxim_dir}
		../${setup_file}
		if [ -d ${build_dir} ];then
			rm -rf ${build_dir}
		fi
		mkdir ${build_dir}
		cd ${build_dir}
		../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt4}
		make clean
		make -j16
		make install-strip DESTDIR=$SDKTARGETSYSROOT
		break;;
	5)
# FMAM SDR Demod
		cd ${fmam_dir}
		../${setup_file}
		if [ -d ${build_dir} ];then
			rm -rf ${build_dir}
		fi
		mkdir ${build_dir}
		cd ${build_dir}
		../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt4}
		make clean
		make -j16
		make install-strip DESTDIR=$SDKTARGETSYSROOT
		break;;
	6)
# HDR SDR Demod
		cd ${hdr_dir}
		../${setup_file}
		if [ -d ${build_dir} ];then
			rm -rf ${build_dir}
		fi
		mkdir ${build_dir}
		cd ${build_dir}
		../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt4}
		make clean
		make -j16
		make install-strip DESTDIR=$SDKTARGETSYSROOT
		break;;
	7)
# HAL
		cd ${hal_dir}
		../${setup_file}
		if [ -d ${build_dir} ];then
			rm -rf ${build_dir}
		fi
		mkdir ${build_dir}
		cd ${build_dir}
		../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt2} ${config_opt3} ${config_opt4}
		make clean
		make -j16
		make install-strip DESTDIR=$SDKTARGETSYSROOT
		break;;
	8)
# Middleware and CUI
		cd ${mw_dir}
		../${setup_file}
		if [ -d ${build_dir} ];then
			rm -rf ${build_dir}
		fi
		mkdir ${build_dir}
		cd ${build_dir}
		../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt4}
		make clean
		make -j16
		make install-strip DESTDIR=$SDKTARGETSYSROOT
		break;;
	9)
# GUI
		cd ${app_dir}
		../${setup_file}
		if [ -d ${build_dir} ];then
			rm -rf ${build_dir}
		fi
		mkdir ${build_dir}
		cd ${build_dir}
		../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt4}
		make clean
		make -j16
		make install-strip DESTDIR=$SDKTARGETSYSROOT
		break;;
	10)
# HDR Middleware and CUI
		cd ${hd_mw_dir}
		../${setup_file}
		if [ -d ${build_dir} ];then
			rm -rf ${build_dir}
		fi
		mkdir ${build_dir}
		cd ${build_dir}
		../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt4} ${config_opt5}
		make clean
		make -j16
		make install-strip DESTDIR=$SDKTARGETSYSROOT
		break;;
	11)
# HDR APP - binary
		cd ${hd_app_dir}
		../${setup_file}
		if [ -d ${build_dir} ];then
			rm -rf ${build_dir}
		fi
		mkdir ${build_dir}
		cd ${build_dir}
		../configure $CONFIGURE_FLAGS ${config_opt1} ${config_opt4}
		make clean
		make -j16
		make install-strip DESTDIR=$SDKTARGETSYSROOT
		break;;
	12)
# HDR GUI - open source
		cd ${gui_dir}
		qmake ${config_opt6}
		make clean
		make
		cp -a bin/TcRadioApp ${tcadt_local_dir}/bin
		break;;

	13)
# Build All
		echo -e "\033[31;1m"Not ready!!! Please select other commands."\033[0m"
		;;
	14)
# Make All
		echo -e "\033[31;1m"Not ready!!! Please select other commands."\033[0m"
		;;
	*)
		echo -e "\033[31;1m"Invalid Input!!!"\033[0m"
		exit;;
esac
done
