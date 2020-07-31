#
# Copyright (c) 2006-2019, RT-Thread Development Team
#
# SPDX-License-Identifier: Apache-2.0
#
# Change Logs:
# Date           Author       Notes
# 2019-09-24     MurphyZhao   First version
# 2019-11-20     MurphyZhao   add zip(rsfota) func
#

import os
import sys, shutil
import zipfile

reload(sys)
sys.setdefaultencoding( "utf-8" )

root_dir = os.getcwd()

rtconfig_h_file = os.path.join(root_dir, "../../project/nbiot_demo/rtconfig.h")

package_xml_file = os.path.join(root_dir, "../../tools/download_config/package.xml")

rtt_bin_file       = os.path.join(root_dir, "rtthread.bin")
dst_rtt_bin_file   = os.path.join(root_dir, "../../tools/download_config/rtthread.bin")
dst_rsota_bin_file = os.path.join(root_dir, "../../tools/download_config/ROM.bin")

rtt_elf_file        = os.path.join(root_dir, "rt-thread.elf")
dst_rtt_elf_file    = os.path.join(root_dir, "../../tools/download_config/rt-thread.elf")
dst_rsfota_elf_file = os.path.join(root_dir, "../../tools/download_config/ROM.elf")

def query_string_in_file(file, query_string):
    with open(file, 'r') as fp:
        for line in fp:
            if query_string in line:
                # print(line)
                return line
    return ""

def query_sdk_version():
    line = query_string_in_file(rtconfig_h_file, "RT_NB_SDK_VERSION")
    if line == "":
        return ""
    
    line_list = line.split()
    # print(line_list)
    if len(line_list) != 3:
        return ""
    ver = line_list[2]
    ver = ver[1:(len(ver)-1)]
    # print(ver)
    return ver

def make_zip(source_dir, output_filename):
    zipf = zipfile.ZipFile(output_filename, 'w')
    pre_len = len(os.path.dirname(source_dir))
    for parent, dirnames, filenames in os.walk(source_dir):
        for filename in filenames:
            pathfile = os.path.join(parent, filename)
            arcname = pathfile[pre_len:].strip(os.path.sep)
            # print(pathfile, " ", arcname)
            # zipf.write(pathfile, arcname)
            zipf.write(pathfile, filename)
    zipf.close()

if __name__ == '__main__':
    # print("==== In main function of (%s) ====" % (__file__))

    # copy rtthread.bin and rt-thread.elf to tools/download_config directory
    if os.path.isfile(rtt_bin_file):
        shutil.copyfile(rtt_bin_file, dst_rtt_bin_file)
        shutil.copyfile(dst_rtt_bin_file, dst_rsota_bin_file)

        shutil.copyfile(rtt_elf_file, dst_rtt_elf_file)
        shutil.copyfile(rtt_elf_file, dst_rsfota_elf_file)
        print("copy rtthread.bin to tools/download_config/ pass.")
    else:
        print("copy rtthread.bin to tools/download_config/ failed!")

    # query sdk version
    version = query_sdk_version()
    if not version.startswith('v'):
        print("[ err ] sdk version error")
        exit(0)
    # print(version)

    # mkdir rsfota-<sdkversion>. eg: rsfota-v1.1
    version = "rsfota-" + version
    rsfota_path = os.path.join(root_dir, "../../tools/download_config", version)
    # print(rsfota_path)

    if os.path.exists(rsfota_path):
        shutil.rmtree(rsfota_path)
    os.mkdir(rsfota_path)

    # move rtthread.bin and rt-thread.elf to rsfota-v1.1/ROM.bin and rsfota-v1.1/ROM.elf
    # move package.xml to rsfota-v1.1/package.xml
    if not (os.path.exists(package_xml_file) and 
        os.path.exists(dst_rsota_bin_file) and 
        os.path.exists(dst_rsfota_elf_file)):
        print("[ err ] not find package.xml/ROM.bin/ROM.elf file")
        exit(0)

    shutil.copyfile(package_xml_file,    os.path.join(rsfota_path, "package.xml"))
    shutil.copyfile(dst_rsota_bin_file,  os.path.join(rsfota_path, "ROM.bin"))
    shutil.copyfile(dst_rsfota_elf_file, os.path.join(rsfota_path, "ROM.elf"))

    # zip(package.xml, ROM.bin, ROM.elf) -> rsfota-v1.1.zip
    #    rsfota-v1.1.zip
    #        |- package.xml
    #        |- ROM.bin
    #        |- ROM.elf
    zip_file_name = rsfota_path + '.zip'
    make_zip(rsfota_path, zip_file_name)
    if os.path.exists(zip_file_name):
        print("zip rsfota pass")
        print(zip_file_name)
    else:
        print("[ err ] zip failed")
    
    # clean tools/download_config/ROM.bin, ROM.elf, rsfota-vx.x
    try:
        if os.path.exists(rsfota_path):
            shutil.rmtree(rsfota_path)
        os.remove(dst_rsota_bin_file)
        os.remove(dst_rsfota_elf_file)
    except Exception as ex:
        print(str(ex))
