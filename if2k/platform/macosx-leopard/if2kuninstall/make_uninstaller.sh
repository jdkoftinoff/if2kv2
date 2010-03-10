if [ "$TARGET_BUILD_DIR" ]; then
    DESTDIR="$TARGET_BUILD_DIR"
else
    DESTDIR=../build/Release
fi

APPNAME="If2k UnInstall"

rm -r -f "$DESTDIR/$APPNAME.app"

platypus -R -a "$APPNAME" -t shell -o TextWindow -i ../../../../if2k-files/misc/if2k-icon.icns  -V "2.2.8" -u "Turner and Sons Productions, Inc." -I com.internetfilter.if2k.uninstall -A  if2kuninstall.sh "$DESTDIR/$APPNAME"
