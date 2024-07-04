#!/bin/bash

default_dir="/w/works/app/tcradio_app_v2p0/src/tc-radio"
peri_dir="peri"
tuner_dir="tuner"
hal_dir="hal"
demod_dir="demod"
mw_dir="mw"
app_dir="app"
git_conf_dir="gitconf"
hd_dir="hd"

while :
do
echo -e "\033[33;1m"[Copy Source To Target with Ethernet]"\033[0m"
echo -e "\033[36;1m"Input Target Address \(default: root@192.168.21.99\): "\033[0m"
read addr
if [ -z "$addr" ]; then
	addr="root@192.168.21.99"
fi
echo -e "\033[36;1m"Input Target Directory \(default: /opt/data\): "\033[0m"
read pos
if [ -z "$pos" ]; then
	pos="/opt/data"
fi
word=$addr:$pos
echo -e "\033[33;1m"Input String : $word"\033[0m"
echo -e "\033[33;1m"Is it correct? [y/n]: "\033[0m"
read yesno
if [ "$yesno" == "y" ]; then
	break
fi
done

tar -zcvf tcradio_src_$(date +%Y%m%d)_$(date +%H%M).tar.gz ./*.sh ./readme.txt ./.gitignore \
															${peri_dir}/src ${peri_dir}/configure.ac ${peri_dir}/Makefile.am \
															${tuner_dir}/src ${tuner_dir}/configure.ac ${tuner_dir}/Makefile.am \
															${hal_dir}/src ${hal_dir}/configure.ac ${hal_dir}/Makefile.am \
															${mw_dir}/src ${mw_dir}/configure.ac ${mw_dir}/Makefile.am \
															${app_dir}/res ${app_dir}/src ${app_dir}/*.pro \
															${git_conf_dir}/ \
															${hd_dir}/src ${hd_dir}/configure.ac ${hd_dir}/Makefile.am
