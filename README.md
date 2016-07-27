cocos-vr-sdks
================

##How to test

1.Create a new cocos project

```
$ cocos new project-name -l cpp[lua][js]
```

2.Use cocos package manager to install sdks

```
$ cocos package import -v -b vrsdkbase -p project-path --staging
```
>Need install **vrsdkbase** first

```
$ cocos package import -v -b gearvr -p project-path --staging
```

```
$ cocos package import -v -b deepoon -p project-path --staging
```

```
$ cocos package import -v -b gvr -p project-path --staging
```

```
$ cocos package import -v -b oculus -p project-path --staging
```

3.Use **switchVRPlatform.py** to switch vr platform

cpp:
```
$ python project-path/vrsdks/switchVRPlatform.py -p gearvr-sdk[deepoon-sdk][gvr-sdk][oculus-sdk]
```

lua and js:
```
$ python project-path/frameworks/runtime-src/vrsdks/switchVRPlatform.py -p gearvr-sdk[deepoon-sdk][gvr-sdk][oculus-sdk]
```

4.compile project

```
$ cocos compile -p android [--android-studio]
```
>**gearvr/deepoon/gvr** only support android platform and **gvr** only support Android Studio to compile

For **oculus**, we need use **Visual Studio** to compile, because it only support win32 platform.
>we need manually add the **oculus-sdk** to **Visual Studio**, because I haven't find a way to automatically add a .vcxproj to **Visual Studio**.

>add **vrsdks/oculus-sdk/oculus/proj.win32/liboculus.vcxproj** to solution.

>add **vrsdks/oculus-sdk/CCVROculusRenderer.cpp** and **vrsdks/oculus-sdk/CCVROculusHeadTracker.cpp** to project.