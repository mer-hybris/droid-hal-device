OUT_DEVICE=${HABUILD_DEVICE:-$DEVICE}

if [ ! -f ./out/target/product/${OUT_DEVICE}/system/lib/libdroidmedia.so ]; then
    echo "Please build droidmedia as per HADK instructions"
    exit 1
fi

pkg=droidmedia-"${1:-0.0.0}"
fold=hybris/mw/$pkg
rm -rf $fold
mkdir $fold

mkdir -p $fold/out/target/product/${OUT_DEVICE}/system/lib
mkdir -p $fold/out/target/product/${OUT_DEVICE}/system/bin
mkdir -p $fold/external/droidmedia

cp ./external/droidmedia/*.h $fold/external/droidmedia/
cp ./external/droidmedia/hybris.c $fold/external/droidmedia/
# Remove droidmedia bits from out/ (otherwise it would cause a conflict within droid-hal-$OUT_DEVICE):
mv ./out/target/product/${OUT_DEVICE}/system/lib/libdroidmedia.so $fold/out/target/product/${OUT_DEVICE}/system/lib/
mv ./out/target/product/${OUT_DEVICE}/system/lib/libminisf.so $fold/out/target/product/${OUT_DEVICE}/system/lib/
mv ./out/target/product/${OUT_DEVICE}/system/bin/minimediaservice $fold/out/target/product/${OUT_DEVICE}/system/bin/
mv ./out/target/product/${OUT_DEVICE}/system/bin/minisfservice $fold/out/target/product/${OUT_DEVICE}/system/bin/

tar -cjvf $fold.tgz -C $(dirname $fold) $pkg

rm -rf $fold

