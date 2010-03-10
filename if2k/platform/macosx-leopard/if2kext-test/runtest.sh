echo unloading kext
sudo kextunload if2kext.kext 2>/dev/null >/dev/null

./fixperms.sh || exit 1
make testsrc || exit 1

echo load kext
sudo kextload -v 3 -t if2kext.kext || exit 1

echo run test
echo jdkoftinoff.com 80 1
sudo ./testsrc jdkoftinoff.com 80 1
echo jdkoftinoff.com 80 0
sudo ./testsrc jdkoftinoff.com 80 0
echo jdkoftinoff.com 80 1
sudo ./testsrc jdkoftinoff.com 80 1


sudo kextunload if2kext.kext



