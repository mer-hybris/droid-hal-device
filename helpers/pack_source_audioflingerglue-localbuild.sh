OUT_DEVICE=${HABUILD_DEVICE:-$DEVICE}

if [ -f ./out/target/product/${OUT_DEVICE}/system/lib/libaudioflingerglue.so ]; then
    DROIDLIB=lib
elif [ -f ./out/target/product/${OUT_DEVICE}/system/lib64/libaudioflingerglue.so ]; then
    DROIDLIB=lib64
else
    echo "Please build audioflingerglue as per HADK instructions"
    exit 1
fi

pkg=audioflingerglue-"${1:-0.0.0}"
fold=hybris/mw/$pkg
rm -rf $fold
mkdir $fold

mkdir -p $fold/out/target/product/${OUT_DEVICE}/system/${DROIDLIB}
mkdir -p $fold/out/target/product/${OUT_DEVICE}/system/bin
mkdir -p $fold/external/audioflingerglue

cp ./external/audioflingerglue/*.h $fold/external/audioflingerglue/
cp ./external/audioflingerglue/hybris.c.in $fold/external/audioflingerglue/
cp ./out/target/product/${OUT_DEVICE}/system/${DROIDLIB}/libaudioflingerglue.so $fold/out/target/product/${OUT_DEVICE}/system/${DROIDLIB}/
cp ./out/target/product/${OUT_DEVICE}/system/bin/miniafservice $fold/out/target/product/${OUT_DEVICE}/system/bin/

tar -cjvf $fold.tgz -C $(dirname $fold) $pkg

rm -rf $fold

