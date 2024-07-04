tcadt_local_dir=${OECORE_TARGET_SYSROOT}/usr/local
build_dir="build"

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

export LC_ALL=C
aclocal
autoheader
libtoolize --copy --force
autoconf
automake --foreign --add-missing --copy --force
if [ -d ${build_dir} ];then
	rm -rf ${build_dir}
fi
mkdir ${build_dir}
cd ${build_dir}
../configure $CONFIGURE_FLAGS ${config_opt4}
make clean
make
make install-strip DESTDIR=$SDKTARGETSYSROOT
rm -rf ${tcadt_local_dir}/lib/libHDRadio.la
