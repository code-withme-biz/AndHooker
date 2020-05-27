# AndHooker

## Android hooker for ARM, ARM64 and X86

AndHooker is to inject DEX or APK to any Android process after injection. The project will generate libhook.so file which will be loaded to target process.

Check out AndInjector repo to see how to load libhook.so

### Build process
cd jni

ndk-build

Library files can be found in libs folder. 