if [ ! -f ./out/target/product/${DEVICE}/system/lib/libaudioflingerglue.so ]; then
    echo "Please build audioflingerglue as per HADK instructions"
    exit 1
fi

pkg=audioflingerglue-0.0.1
fold=hybris/mw/$pkg
rm -rf $fold
mkdir $fold

mkdir -p $fold/out/target/product/${DEVICE}/system/lib
mkdir -p $fold/out/target/product/${DEVICE}/system/bin
mkdir -p $fold/external/audioflingerglue

cp ./external/audioflingerglue/*.h $fold/external/audioflingerglue/
cp ./external/audioflingerglue/hybris.c $fold/external/audioflingerglue/
# Remove audioflingerglue bits from out/ (otherwise it would cause a conflict within droid-hal-$DEVICE):
mv ./out/target/product/${DEVICE}/system/lib/libaudioflingerglue.so $fold/out/target/product/${DEVICE}/system/lib/
mv ./out/target/product/${DEVICE}/system/bin/miniafservice $fold/out/target/product/${DEVICE}/system/bin/

tar -cjvf $fold.tgz -C $(dirname $fold) $pkg

rm -rf $fold

