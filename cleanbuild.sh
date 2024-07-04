#!/bin/bash

peri_dir="peri"
tuner_dir="tuner"
hal_dir="hal"
mw_dir="mw"
app_dir="app"
hd_dir="hd"
hd_core_dir="hdcore"
mainmgr_dir="tc-radio-hd-manager"

function remove_build_files()
{
	echo ============================
	echo Folder Name To Clean: $1
	echo ============================
	rm -rf $1/build
	rm -rf $1/autom4te.cache
	rm -rf $1/aclocal.m4
	rm -rf $1/compile
	rm -rf $1/config.guess
	rm -rf $1/config.h.in
	rm -rf $1/config.sub
	rm -rf $1/configure
	rm -rf $1/depcomp
	rm -rf $1/install-sh
	rm -rf $1/ltmain.sh
	rm -rf $1/Makefile.in
	rm -rf $1/missing
	rm -rf $1/config.log
}

if [ "$1" == "${peri_dir}" ] || [ "$1" == "${tuner_dir}" ] || [ "$1" == "${hal_dir}" ] || [ "$1" == "${hd_dir}" ] || [ "$1" == "${mw_dir}" ] || [ "$1" == "${hd_core_dir}" ] || [ "$1" == "${mainmgr_dir}" ]
then
	remove_build_file_en=1
	remove_build_all_files_en=0
elif [ "$1" == 'all' ];then
	remove_build_file_en=0
	remove_build_all_files_en=1
else
	remove_build_file_en=0
	remove_build_all_files_en=0
fi

if [ $remove_build_file_en -eq 1 ];then
	remove_build_files $1
elif [ $remove_build_all_files_en -eq 1 ];then
	remove_build_files $peri_dir
	remove_build_files $tuner_dir
	remove_build_files $hal_dir
	remove_build_files $hd_dir
	remove_build_files $mw_dir
	remove_build_files $hd_core_dir
	remove_build_files $mainmgr_dir
else
	echo -e "\033[31;1m"Invalid Folder Name!!!"\033[0m"
fi

