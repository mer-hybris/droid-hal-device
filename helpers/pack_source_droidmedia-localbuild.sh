OUT_DEVICE=${HABUILD_DEVICE:-$DEVICE}

if [ -f ./out/target/product/${OUT_DEVICE}/system/lib64/libdroidmedia.so ]; then
DROIDLIB=lib64
else
DROIDLIB=lib
fi

if [ ! -f ./out/target/product/${OUT_DEVICE}/system/${DROIDLIB}/libdroidmedia.so ]; then
    echo "Please build droidmedia as per HADK instructions"
    exit 1
fi

pkg=droidmedia-"${1:-0.0.0}"
fold=hybris/mw/$pkg
rm -rf $fold
mkdir $fold

mkdir -p $fold/out/target/product/${OUT_DEVICE}/system/${DROIDLIB}
mkdir -p $fold/out/target/product/${OUT_DEVICE}/system/bin
mkdir -p $fold/external/droidmedia

cp ./external/droidmedia/*.h $fold/external/droidmedia/
cp ./external/droidmedia/hybris.c $fold/external/droidmedia/
cp ./out/target/product/${OUT_DEVICE}/system/${DROIDLIB}/libdroidmedia.so $fold/out/target/product/${OUT_DEVICE}/system/${DROIDLIB}/
cp ./out/target/product/${OUT_DEVICE}/system/${DROIDLIB}/libminisf.so $fold/out/target/product/${OUT_DEVICE}/system/${DROIDLIB}/
cp ./out/target/product/${OUT_DEVICE}/system/bin/minimediaservice $fold/out/target/product/${OUT_DEVICE}/system/bin/
cp ./out/target/product/${OUT_DEVICE}/system/bin/minisfservice $fold/out/target/product/${OUT_DEVICE}/system/bin/

tar -cjvf $fold.tgz -C $(dirname $fold) $pkg

rm -rf $fold

