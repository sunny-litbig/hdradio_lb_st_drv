#!/bin/bash

peri_dir="peri"
tuner_dir="tuner"
hal_dir="hal"
mw_dir="mw"
app_dir="app"
git_conf_dir="gitconf"
hd_dir="hd"
mainmgr_dir="tc-radio-hd-manager"
hd_core_dir="hdcore"

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

