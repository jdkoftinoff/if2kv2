sudo rm -rf if2kext.kext 2>/dev/null
sudo cp -rp ../build/Release/if2kext.kext . || exit 1
sudo chown -R root:wheel if2kext.kext || exit 1

