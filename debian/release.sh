BINARY_NAME=nando
RELEASE=3.5.0
FULL_NAME=$BINARY_NAME-$RELEASE
TAR_NAME=$FULL_NAME.tar.gz
EMAIL=bogdan.s.bogush@gmail.com
DEB_RELEASE_DIR=release

# prepare pbuilder envinronment
# sudo apt install dh-make pbuilder
# sudo pbuilder create --distribution <ubuntu release>

# generate private key
# gpg --gen-key

# change log
# dch -i

rm -rf $DEB_RELEASE_DIR
mkdir $DEB_RELEASE_DIR
mkdir $DEB_RELEASE_DIR/$FULL_NAME
cp -r ../qt/ $DEB_RELEASE_DIR/$FULL_NAME/qt
cp -r debian/ $DEB_RELEASE_DIR/$FULL_NAME/debian
cp nando.desktop $DEB_RELEASE_DIR/$FULL_NAME/
cp nando.pro $DEB_RELEASE_DIR/$FULL_NAME/
cp ../LICENSE.txt $DEB_RELEASE_DIR/$FULL_NAME/LICENSE
cd $DEB_RELEASE_DIR
tar -zcvf $TAR_NAME $FULL_NAME/
cd $FULL_NAME/
dh_make -e $EMAIL -f ../$TAR_NAME
pdebuild --debbuildopts -sa

# result
# ls /var/cache/pbuilder/result/



