#!/usr/bin/env python

import os
import shutil

def cleanup(dir):
    if os.path.exists(dir):
        shutil.rmtree(dir)
    os.makedirs(dir)


def file_to_c_code(srcfile, targetfile):
    fcn_name = os.path.basename(srcfile).replace(".", "_")
    index = []
    index.append("const char {}[] PROGMEM = R\"=====(\n".format(fcn_name))

    with open(srcfile, "r") as f:
        html = f.readlines()

    index += html
    index.append(")=====\";")

    with open(targetfile, "w") as f:
        f.writelines(index)


def main():
    gen_includes_dir = "includes_gen"
    c_index = os.path.join(gen_includes_dir, "index.h")
    c_css = os.path.join(gen_includes_dir, "css.h")

    web_dir = "src/web"
    index = os.path.join(web_dir, "index.html")
    css = os.path.join(web_dir, "style.css")

    cleanup(gen_includes_dir)
    file_to_c_code(index, c_index)
    file_to_c_code(css, c_css)


main()
