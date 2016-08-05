
if OpenAPI.satisfies_cocos_version('3.13'):
    OpenAPI.copy_files(['vrsdks'], PLUGIN_ROOT, COCOS_PROJECT_HINT)
    OpenAPI.android_add_architectures(['armeabi-v7a'])
    
    if COCOS_PROJECT_TYPE == 'cpp':
        if ANDROID_PROJECT_DIR != 'n/a':
            #android_sources = ['../../vrsdk/CCVRSDKManager.cpp']
            #OpenAPI.android_add_sources(android_sources, False)

            #OpenAPI.android_add_shared_libraries(['vrsdk'])
    
            OpenAPI.copy_files('VRSDKWrapper.java', PLUGIN_ROOT + 'patchs/cpp/', ANDROID_ACTIVITY_PATH)
            OpenAPI.apply_patch(ANDROID_ACTIVITY_PATH + ANDROID_ACTIVITY_NAME + '.java', 'AppActivity.java.cpp.3.13.patch', root=ANDROID_ACTIVITY_PATH)
            OpenAPI.apply_patch(ANDROID_JNI_DIR + 'Android.mk', 'Android.mk.cpp.3.13.patch', root=ANDROID_JNI_DIR)
            OpenAPI.apply_patch(ANDROID_PROJECT_DIR + 'AndroidManifest.xml', 'AndroidManifest.xml.cpp.3.13.patch', root=ANDROID_PROJECT_DIR)

        if ANDROID_STUDIO_PROJECT_DIR != 'n/a':
            OpenAPI.copy_files('VRSDKWrapper.java', PLUGIN_ROOT + 'patchs/cpp/', ANDROID_STUDIO_ACTIVITY_PATH)
            OpenAPI.apply_patch(ANDROID_STUDIO_ACTIVITY_PATH + ANDROID_STUDIO_ACTIVITY_NAME + '.java', 'AppActivity.java.cpp.studio.3.13.patch', root=ANDROID_STUDIO_ACTIVITY_PATH)
            OpenAPI.apply_patch(ANDROID_STUDIO_JNI_DIR + 'Android.mk', 'Android.mk.cpp.studio.3.13.patch', root=ANDROID_STUDIO_JNI_DIR)
            OpenAPI.apply_patch(ANDROID_STUDIO_PROJECT_DIR + 'app/' + 'AndroidManifest.xml', 'AndroidManifest.xml.cpp.studio.3.13.patch', root=ANDROID_STUDIO_PROJECT_DIR + 'app/')
    
        OpenAPI.apply_patch(COCOS_CLASSES_DIR + 'AppDelegate.cpp', 'AppDelegate.cpp.cpp.3.13.patch', root=COCOS_CLASSES_DIR)

    if COCOS_PROJECT_TYPE == 'lua':
        if ANDROID_PROJECT_DIR != 'n/a':
            OpenAPI.copy_files('VRSDKWrapper.java', PLUGIN_ROOT + 'patchs/lua/', ANDROID_ACTIVITY_PATH)
            OpenAPI.apply_patch(ANDROID_ACTIVITY_PATH + ANDROID_ACTIVITY_NAME + '.java', 'AppActivity.java.lua.3.13.patch', root=ANDROID_ACTIVITY_PATH)
            OpenAPI.apply_patch(ANDROID_JNI_DIR + 'Android.mk', 'Android.mk.lua.3.13.patch', root=ANDROID_JNI_DIR)
            OpenAPI.apply_patch(ANDROID_PROJECT_DIR + 'AndroidManifest.xml', 'AndroidManifest.xml.lua.3.13.patch', root=ANDROID_PROJECT_DIR)
        
        if ANDROID_STUDIO_PROJECT_DIR != 'n/a':
            OpenAPI.copy_files('VRSDKWrapper.java', PLUGIN_ROOT + 'patchs/lua/', ANDROID_STUDIO_ACTIVITY_PATH)
            OpenAPI.apply_patch(ANDROID_STUDIO_ACTIVITY_PATH + ANDROID_STUDIO_ACTIVITY_NAME + '.java', 'AppActivity.java.lua.studio.3.13.patch', root=ANDROID_STUDIO_ACTIVITY_PATH)
            OpenAPI.apply_patch(ANDROID_STUDIO_JNI_DIR + 'Android.mk', 'Android.mk.lua.studio.3.13.patch', root=ANDROID_STUDIO_JNI_DIR)
            OpenAPI.apply_patch(ANDROID_STUDIO_PROJECT_DIR + 'app/' + 'AndroidManifest.xml', 'AndroidManifest.xml.lua.studio.3.13.patch', root=ANDROID_STUDIO_PROJECT_DIR + 'app/')

        OpenAPI.apply_patch(COCOS_CLASSES_DIR + 'AppDelegate.cpp', 'AppDelegate.cpp.lua.3.13.patch', root=COCOS_CLASSES_DIR)

    if COCOS_PROJECT_TYPE == 'js':
        if ANDROID_PROJECT_DIR != 'n/a':
            OpenAPI.copy_files('VRSDKWrapper.java', PLUGIN_ROOT + 'patchs/js/', ANDROID_ACTIVITY_PATH)
            OpenAPI.apply_patch(ANDROID_ACTIVITY_PATH + ANDROID_ACTIVITY_NAME + '.java', 'AppActivity.java.js.3.13.patch', root=ANDROID_ACTIVITY_PATH)
            OpenAPI.apply_patch(ANDROID_JNI_DIR + 'Android.mk', 'Android.mk.js.3.13.patch', root=ANDROID_JNI_DIR)
            OpenAPI.apply_patch(ANDROID_PROJECT_DIR + 'AndroidManifest.xml', 'AndroidManifest.xml.js.3.13.patch', root=ANDROID_PROJECT_DIR)
        
        if ANDROID_STUDIO_PROJECT_DIR != 'n/a':
            OpenAPI.copy_files('VRSDKWrapper.java', PLUGIN_ROOT + 'patchs/js/', ANDROID_STUDIO_ACTIVITY_PATH)
            OpenAPI.apply_patch(ANDROID_STUDIO_ACTIVITY_PATH + ANDROID_STUDIO_ACTIVITY_NAME + '.java', 'AppActivity.java.js.studio.3.13.patch', root=ANDROID_STUDIO_ACTIVITY_PATH)
            OpenAPI.apply_patch(ANDROID_STUDIO_JNI_DIR + 'Android.mk', 'Android.mk.js.studio.3.13.patch', root=ANDROID_STUDIO_JNI_DIR)
            OpenAPI.apply_patch(ANDROID_STUDIO_PROJECT_DIR + 'app/' + 'AndroidManifest.xml', 'AndroidManifest.xml.js.studio.3.13.patch', root=ANDROID_STUDIO_PROJECT_DIR + 'app/')

        OpenAPI.apply_patch(COCOS_CLASSES_DIR + 'AppDelegate.cpp', 'AppDelegate.cpp.js.3.13.patch', root=COCOS_CLASSES_DIR)
	#OpenAPI.android_add_includes(['../../vrsdk'], False)
	#OpenAPI.android_add_calls(['import-add-path,$(LOCAL_PATH)/../../vrsdk'])
	#OpenAPI.android_studio_add_module('gearvr-sdk', '../gearvr-sdk/VrApi/Projects/AndroidPrebuilt')
