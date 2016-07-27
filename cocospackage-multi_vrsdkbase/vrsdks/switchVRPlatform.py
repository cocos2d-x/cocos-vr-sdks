#!/usr/bin/python

import sys
import json
import os, os.path
import shutil
from optparse import OptionParser

def os_is_win32():
    return sys.platform == 'win32'

def add_path_prefix(path_str):
    if not os_is_win32():
        return path_str

    if path_str.startswith("\\\\?\\"):
        return path_str

    ret = "\\\\?\\" + os.path.abspath(path_str)
    ret = ret.replace("/", "\\")
    return ret

def replace_string(filepath, src_string, dst_string):
    """ From file's content replace specified string
    Arg:
        filepath: Specify a file contains the path
        src_string: old string
        dst_string: new string
    """
    if src_string is None or dst_string is None:
        raise TypeError

    content = ""
    f1 = open(filepath, "rb")
    for line in f1:
        strline = line.decode('utf8')
        if src_string in strline:
            content += strline.replace(src_string, dst_string)
        else:
            content += strline
    f1.close()
    f2 = open(filepath, "wb")
    f2.write(content.encode('utf8'))
    f2.close()
# end of replace_string

def copy_file(dst_dir, file, needClean):
    if os.path.exists(dst_dir):
        if needClean == True:
            shutil.rmtree(dst_dir)

    if file is None:
        return
    if not os.path.exists(dst_dir):
        os.mkdir(dst_dir)

    shutil.copy(file, dst_dir)

def copy_tree(dst_dir, src_dir):
    shutil.copytree(src_dir, dst_dir)


def replace_lines(filepath, line_begin_sign, line_end_sign, lines):
    content = ""
    begin_sign = False
    f1 = open(filepath, "rb")
    for line in f1:
        strline = line.decode('utf8')
        
        if line_begin_sign in strline:
            begin_sign = True
            content += strline
        elif line_end_sign in strline:
            for index in range(len(lines)):
                content += lines[index] + '\n'
            begin_sign = False
        
        if begin_sign == False:
            content += strline

    f1.close()
    f2 = open(filepath, "wb")
    f2.write(content.encode('utf8'))
    f2.close()

def add_lines(filepath, lines):
    if lines is None:
        raise TypeError
    
    content = ""
    f1 = open(filepath, "rb")
    for line in f1:
        strline = line.decode('utf8')
        content += strline
    f1.close()

    for index in range(len(lines)):
        content += lines[index] + '\n'
    f2 = open(filepath, "wb")
    f2.write(content.encode('utf8'))
    f2.close()

COCOS_PROJECT_ROOT = None
VR_SDK_ROOT = None


def set_vr_platform_includes(include_files, config_constants):
    includes = []
    for include in include_files:
        includes.append('#include "' + include + '"')
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['COCOS_CLASSES_DIR']) + 'AppDelegate.cpp', 'VR_PLATFORM_INCLUDES_BEGIN', 'VR_PLATFORM_INCLUDES_END', includes)
    print '> Replaced AppDelegate.cpp VR_PLATFORM_INCLUDES'

def set_vr_platform_sources(sources, config_constants):
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['COCOS_CLASSES_DIR']) + 'AppDelegate.cpp', 'VR_PLATFORM_SOURCES_BEGIN', 'VR_PLATFORM_SOURCES_END', sources)
    print '> Replaced AppDelegate.cpp VR_PLATFORM_SOURCES'

def set_vr_android_source_files(source_files, config_constants):
    android_local_src_files = 'LOCAL_SRC_FILES +='
    android_studio_local_src_files = 'LOCAL_SRC_FILES +='
    for index in range(len(source_files)):
        android_local_src_files += ' ../../vrsdks/' + source_files[index]
        android_studio_local_src_files += ' ../../../vrsdks/' + source_files[index]
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_MK']), '_COCOS_VR_HEADER_ANDROID_BEGIN', '_COCOS_VR_HEADER_ANDROID_END', [android_local_src_files])
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_STUDIO_MK']), '_COCOS_VR_HEADER_ANDROID_BEGIN', '_COCOS_VR_HEADER_ANDROID_END', [android_studio_local_src_files])
    print '> Replaced Android.mk _COCOS_VR_HEADER_ANDROID_'

def set_vr_android_link_libs(link_libs, config_constants):
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_MK']), '_COCOS_VR_LIB_ANDROID_BEGIN', '_COCOS_VR_LIB_ANDROID_END', link_libs)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_STUDIO_MK']), '_COCOS_VR_LIB_ANDROID_BEGIN', '_COCOS_VR_LIB_ANDROID_END', link_libs)
    print '> Replaced Android.mk _COCOS_VR_LIB_ANDROID_'

def set_vr_android_module_deps(module_deps, config_constants):
    modules = []
    for module in module_deps:
        modules.append('$(call import-module, ' + module + ')')
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_MK']), '_COCOS_VR_LIB_IMPORT_ANDROID_BEGIN', '_COCOS_VR_LIB_IMPORT_ANDROID_END', modules)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_STUDIO_MK']), '_COCOS_VR_LIB_IMPORT_ANDROID_BEGIN', '_COCOS_VR_LIB_IMPORT_ANDROID_END', modules)
    print '> Replaced Android.mk _COCOS_VR_LIB_IMPORT_ANDROID_'

def set_vr_sdk_wrapper_imports(sources, config_constants):
    imports = []
    for imp in sources:
        imports.append('import ' + imp + ';')
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_ACTIVITY_PATH']) + 'VRSDKWrapper.java', 'VR_SDK_WRAPPER_IMPORTS_BEGIN', 'VR_SDK_WRAPPER_IMPORTS_END', imports)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_STUDIO_ACTIVITY_PATH']) + 'VRSDKWrapper.java', 'VR_SDK_WRAPPER_IMPORTS_BEGIN', 'VR_SDK_WRAPPER_IMPORTS_END', imports)
    print '> Replaced VRSDKWrapper.java VR_SDK_WRAPPER_IMPORTS'

def set_vr_sdk_wrapper_members(sources, config_constants):
    lines = []
    for line in sources:
        lines.append('    ' + line)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_ACTIVITY_PATH']) + 'VRSDKWrapper.java', 'VR_SDK_WRAPPER_MEMBERS_BEGIN', 'VR_SDK_WRAPPER_MEMBERS_END', lines)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_STUDIO_ACTIVITY_PATH']) + 'VRSDKWrapper.java', 'VR_SDK_WRAPPER_MEMBERS_BEGIN', 'VR_SDK_WRAPPER_MEMBERS_END', lines)
    print '> Replaced VRSDKWrapper.java VR_SDK_WRAPPER_MEMBERS'

def set_vr_sdk_wrapper_functions(sources, config_constants):
    lines = []
    for line in sources:
        lines.append('    ' + line)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_ACTIVITY_PATH']) + 'VRSDKWrapper.java', 'VR_SDK_WRAPPER_FUNCTIONS_BEGIN', 'VR_SDK_WRAPPER_FUNCTIONS_END', lines)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_STUDIO_ACTIVITY_PATH']) + 'VRSDKWrapper.java', 'VR_SDK_WRAPPER_FUNCTIONS_BEGIN', 'VR_SDK_WRAPPER_FUNCTIONS_END', lines)
    print '> Replaced VRSDKWrapper.java VR_SDK_WRAPPER_FUNCTIONS'

def set_vr_sdk_wrapper_onCreate(sources, config_constants):
    lines = []
    for line in sources:
        lines.append('        ' + line)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_ACTIVITY_PATH']) + 'VRSDKWrapper.java', 'VR_SDK_WRAPPER_ON_CREATE_BEGIN', 'VR_SDK_WRAPPER_ON_CREATE_END', lines)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_STUDIO_ACTIVITY_PATH']) + 'VRSDKWrapper.java', 'VR_SDK_WRAPPER_ON_CREATE_BEGIN', 'VR_SDK_WRAPPER_ON_CREATE_END', lines)
    print '> Replaced VRSDKWrapper.java VR_SDK_WRAPPER_ON_CREATE'

def set_vr_sdk_wrapper_onDestroy(sources, config_constants):
    lines = []
    for line in sources:
        lines.append('        ' + line)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_ACTIVITY_PATH']) + 'VRSDKWrapper.java', 'VR_SDK_WRAPPER_ON_DESTROY_BEGIN', 'VR_SDK_WRAPPER_ON_DESTROY_END', lines)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_STUDIO_ACTIVITY_PATH']) + 'VRSDKWrapper.java', 'VR_SDK_WRAPPER_ON_DESTROY_BEGIN', 'VR_SDK_WRAPPER_ON_DESTROY_END', lines)
    print '> Replaced VRSDKWrapper.java VR_SDK_WRAPPER_ON_DESTROY'

def set_vr_sdk_wrapper_onResume(sources, config_constants):
    lines = []
    for line in sources:
        lines.append('        ' + line)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_ACTIVITY_PATH']) + 'VRSDKWrapper.java', 'VR_SDK_WRAPPER_ON_RESUME_BEGIN', 'VR_SDK_WRAPPER_ON_RESUME_END', lines)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_STUDIO_ACTIVITY_PATH']) + 'VRSDKWrapper.java', 'VR_SDK_WRAPPER_ON_RESUME_BEGIN', 'VR_SDK_WRAPPER_ON_RESUME_END', lines)
    print '> Replaced VRSDKWrapper.java VR_SDK_WRAPPER_ON_RESUME'

def set_vr_sdk_wrapper_onPause(sources, config_constants):
    lines = []
    for line in sources:
        lines.append('        ' + line)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_ACTIVITY_PATH']) + 'VRSDKWrapper.java', 'VR_SDK_WRAPPER_ON_PAUSE_BEGIN', 'VR_SDK_WRAPPER_ON_PAUSE_END', lines)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_STUDIO_ACTIVITY_PATH']) + 'VRSDKWrapper.java', 'VR_SDK_WRAPPER_ON_PAUSE_BEGIN', 'VR_SDK_WRAPPER_ON_PAUSE_END', lines)
    print '> Replaced VRSDKWrapper.java VR_SDK_WRAPPER_ON_PAUSE'

def set_vr_sdk_wrapper_onRestart(sources, config_constants):
    lines = []
    for line in sources:
        lines.append('        ' + line)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_ACTIVITY_PATH']) + 'VRSDKWrapper.java', 'VR_SDK_WRAPPER_ON_RESTART_BEGIN', 'VR_SDK_WRAPPER_ON_RESTART_END', lines)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_STUDIO_ACTIVITY_PATH']) + 'VRSDKWrapper.java', 'VR_SDK_WRAPPER_ON_RESTART_BEGIN', 'VR_SDK_WRAPPER_ON_RESTART_END', lines)
    print '> Replaced VRSDKWrapper.java VR_SDK_WRAPPER_ON_RESTART'

def set_vr_sdk_wrapper_onStop(sources, config_constants):
    lines = []
    for line in sources:
        lines.append('        ' + line)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_ACTIVITY_PATH']) + 'VRSDKWrapper.java', 'VR_SDK_WRAPPER_ON_STOP_BEGIN', 'VR_SDK_WRAPPER_ON_STOP_END', lines)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_STUDIO_ACTIVITY_PATH']) + 'VRSDKWrapper.java', 'VR_SDK_WRAPPER_ON_STOP_BEGIN', 'VR_SDK_WRAPPER_ON_STOP_END', lines)
    print '> Replaced VRSDKWrapper.java VR_SDK_WRAPPER_ON_STOP'

def set_vr_sdk_android_manifest_def(sources, config_constants):
    manifest = []
    for mf in sources:
        manifest.append('    ' + mf)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_PROJECT_DIR']) + 'AndroidManifest.xml', '_COCOS_VR_ANDROID_MANIFEST_SDK_BEGIN_', '_COCOS_VR_ANDROID_MANIFEST_SDK_END_', manifest)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_STUDIO_PROJECT_DIR']) + 'app/AndroidManifest.xml', '_COCOS_VR_ANDROID_MANIFEST_SDK_BEGIN_', '_COCOS_VR_ANDROID_MANIFEST_SDK_END_', manifest)
    print '> Replaced AndroidManifest.xml _COCOS_VR_ANDROID_MANIFEST_SDK_'

def set_vr_sdk_android_manifest_app_def(sources, config_constants):
    application = []
    for app in sources:
        application.append('        ' + app)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_PROJECT_DIR']) + 'AndroidManifest.xml', '_COCOS_VR_ANDROID_MANIFEST_APPLICATION_BEGIN_', '_COCOS_VR_ANDROID_MANIFEST_APPLICATION_END_', application)
    replace_lines(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_STUDIO_PROJECT_DIR']) + 'app/AndroidManifest.xml', '_COCOS_VR_ANDROID_MANIFEST_APPLICATION_BEGIN_', '_COCOS_VR_ANDROID_MANIFEST_APPLICATION_END_', application)
    print '> Replaced AndroidManifest.xml _COCOS_VR_ANDROID_MANIFEST_APPLICATION_'

def set_vr_sdk_file_added(file_path, added):
    add_lines(file_path, added)
    print '> Added Lines to ' + file_path

def clean_libs(config_constants):
    copy_file(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_PROJECT_DIR']) + 'libs', None, True)
    copy_file(os.path.join(COCOS_PROJECT_ROOT, config_constants['ANDROID_STUDIO_PROJECT_DIR']) + 'app/libs', None, True)

def search_installed_sdk_in(rootdir):
    sdk_names = []
    for parent,dirnames,filenames in os.walk(rootdir):
        for dirname in dirnames:
            sdk_names.append(dirname)
        return sdk_names

def load_vr_sdk_settings(file_path, config_constants):
    f = open(file_path)
    settings = json.load(f)
    f.close()
    
    includes = []
    codes = []
    if 'AppDelegate.cpp' in settings:
        app_delegate = settings['AppDelegate.cpp']
        if 'includes' in app_delegate:
            includes = app_delegate['includes']
        if 'codes' in app_delegate:
            for code in app_delegate['codes']:
                codes.append('     ' + code)
    set_vr_platform_includes(includes, config_constants)
    set_vr_platform_sources(codes, config_constants)

    local_src_files = []
    libs = []
    import_module = []
    if 'Android.mk' in settings:
        android_mk = settings['Android.mk']
        if 'local_src_files' in android_mk:
            local_src_files = android_mk['local_src_files']
        if 'shared_libs' in android_mk:
            shared_libs = 'LOCAL_SHARED_LIBRARIES :='
            for lib in android_mk['shared_libs']:
                shared_libs += ' ' + lib
            libs.append(shared_libs)
        if 'static_libs' in android_mk:
            static_libs = 'LOCAL_STATIC_LIBRARIES +='
            for lib in android_mk['shared_libs']:
                static_libs += ' ' + lib
            libs.append(static_libs)
        if 'import_module' in android_mk:
            import_module = android_mk['import_module']
    set_vr_android_source_files(local_src_files, config_constants)
    set_vr_android_link_libs(libs, config_constants)
    set_vr_android_module_deps(import_module, config_constants)

    manifest = []
    application = []
    if 'AndroidManifest.xml' in settings:
        android_manifest = settings['AndroidManifest.xml']
        if 'manifest' in android_manifest:
            manifest = android_manifest['manifest']
        if 'application' in android_manifest:
            application = android_manifest['application']
    set_vr_sdk_android_manifest_def(manifest, config_constants)
    set_vr_sdk_android_manifest_app_def(application, config_constants)

    imports = []
    members = []
    functions = []
    onCreate = []
    onDestroy = []
    onResume = []
    onPause = []
    onRestart = []
    onStop = []
    if 'VRSDKWrapper.java' in settings:
        sdk_wrapper = settings['VRSDKWrapper.java']
        if 'imports' in sdk_wrapper:
            imports = sdk_wrapper['imports']
        if 'members' in sdk_wrapper:
            members = sdk_wrapper['members']
        if 'functions' in sdk_wrapper:
            functions = sdk_wrapper['functions']
        if 'onCreate' in sdk_wrapper:
            onCreate = sdk_wrapper['onCreate']
        if 'onDestroy' in sdk_wrapper:
            onDestroy = sdk_wrapper['onDestroy']
        if 'onResume' in sdk_wrapper:
            onResume = sdk_wrapper['onResume']
        if 'onPause' in sdk_wrapper:
            onPause = sdk_wrapper['onPause']
        if 'onRestart' in sdk_wrapper:
            onRestart = sdk_wrapper['onRestart']
        if 'onStop' in sdk_wrapper:
            onStop = sdk_wrapper['onStop']
    set_vr_sdk_wrapper_imports(imports, config_constants)
    set_vr_sdk_wrapper_members(members, config_constants)
    set_vr_sdk_wrapper_functions(functions, config_constants)
    set_vr_sdk_wrapper_onCreate(onCreate, config_constants)
    set_vr_sdk_wrapper_onDestroy(onDestroy, config_constants)
    set_vr_sdk_wrapper_onResume(onResume, config_constants)
    set_vr_sdk_wrapper_onPause(onPause, config_constants)
    set_vr_sdk_wrapper_onRestart(onRestart, config_constants)
    set_vr_sdk_wrapper_onStop(onStop, config_constants)

    if 'files' in settings:
        files = settings['files']
        for file_name in files:
            file = files[file_name]
            file_path = get_real_path(file['dir'], config_constants) + '/' + file_name
            file_path = os.path.join(COCOS_PROJECT_ROOT, file_path)
            if 'added' in file:
                add_lines(file_path, file['added'])
                print '> Add Lines to ' + file_path
            if 'modified' in file:
                modified = file['modified']
                for modified_line in modified:
                    replace_string(file_path, modified_line, modified[modified_line])
                print '> Modify File ' + file_path

    clean_libs(config_constants)
    if 'copy_files' in settings:
        copy_files = settings['copy_files']
        for file in copy_files:
            src = file['src']
            dst = file['dst']
            src = get_real_path(src, config_constants)
            dst = get_real_path(dst, config_constants)
            src = os.path.join(COCOS_PROJECT_ROOT, src)
            dst = os.path.join(COCOS_PROJECT_ROOT, dst)
            if os.path.isdir(src) is True:
                copy_tree(dst, src)
                print '> Copy tree from ' + src + ' to ' + dst
            else:
                copy_file(dst, src, False)
                print '> Copy file from ' + src + ' to ' + dst


def load_vr_sdk_settings_default(config_constants):
    set_vr_platform_includes([], config_constants)
    set_vr_platform_sources(['     auto vrImpl = new VRGenericRenderer;', '     glview->setVR(vrImpl);'], config_constants)
    set_vr_android_source_files([], config_constants)
    set_vr_android_link_libs([], config_constants)
    set_vr_android_module_deps([], config_constants)
    set_vr_sdk_android_manifest_def([], config_constants)
    set_vr_sdk_android_manifest_app_def([], config_constants)
    set_vr_sdk_wrapper_imports([], config_constants)
    set_vr_sdk_wrapper_members([], config_constants)
    set_vr_sdk_wrapper_functions([], config_constants)
    set_vr_sdk_wrapper_onCreate([], config_constants)
    set_vr_sdk_wrapper_onDestroy([], config_constants)
    set_vr_sdk_wrapper_onResume([], config_constants)
    set_vr_sdk_wrapper_onPause([], config_constants)
    set_vr_sdk_wrapper_onRestart([], config_constants)
    set_vr_sdk_wrapper_onStop([], config_constants)
    clean_libs(config_constants)

def find_file(file_name ,dir):
    list_dirs = os.walk(dir)
    for root, dirs, files in list_dirs:
        for f in files:
            if f == file_name:
                return True
        return False

def find_cocos_package_root_dir(current_path, file_name):
    while current_path[1] is not '':
        if find_file(file_name, current_path[0]):
            return current_path[0]
        current_path = os.path.split(current_path[0])
    return None

def load_package_configuration(file_path, sdk_name):
    f = open(file_path)
    configs = json.load(f)
    f.close()
    for config in configs:
        if config in sdk_name or sdk_name in config:
            sdk_config = configs[config]
            return sdk_config['constants']

def get_real_path(path, path_ids):
    if 'VR_SDK_ROOT' in path:
        return path.replace('VR_SDK_ROOT', VR_SDK_ROOT)
    
    for path_id in path_ids:
        if path_id in path:
            return path.replace(path_id, path_ids[path_id])

# -------------- main --------------
if __name__ == '__main__':
    current_path = os.path.split(os.path.realpath(__file__))
    VR_SDK_ROOT = current_path[0]
    COCOS_PROJECT_ROOT = find_cocos_package_root_dir(current_path, '.cocos-project.json')
    if COCOS_PROJECT_ROOT is not None:
        installed_sdks = search_installed_sdk_in(VR_SDK_ROOT)
        help_str = 'vr platform'
        for sdk in installed_sdks:
            help_str += ' [' + sdk + ']'
        parser = OptionParser()
        parser.add_option("-p", "--platform", dest="vr_platform", help=help_str)
        (opts, args) = parser.parse_args()
    
        if opts.vr_platform in installed_sdks:
            print('-----------------Switch To ' + opts.vr_platform + '-----------------')
            config_constants = load_package_configuration(os.path.join(COCOS_PROJECT_ROOT, '.cocos-package.json'), opts.vr_platform)
            load_vr_sdk_settings(os.path.join(VR_SDK_ROOT, opts.vr_platform + '/vr-sdk-setting.json'), config_constants)
            print('-----------------Switch To ' + opts.vr_platform + '-----------------')
        else:
            print('-----------------Switch To default vr sdk-----------------')
            config_constants = load_package_configuration(os.path.join(COCOS_PROJECT_ROOT, '.cocos-package.json'), 'vrsdkbase')
            load_vr_sdk_settings_default(config_constants)
            print('-----------------Switch To default vr sdk-----------------')
    else:
        print('Can not find cocos package configuration!')
