#!/usr/bin/env python3
Import("env")

import os
from datetime import datetime

def update_app_version():
    now = datetime.now()
    version_string = now.strftime("%Y%m%d%H%M%S")

    globals_file = os.path.join("src", "globals.cpp")

    if not os.path.exists(globals_file):
        print(f"❌ Error: {globals_file} not found!")
        return False

    try:
        with open(globals_file, 'r', encoding='utf-8') as file:
            lines = file.readlines()

        found_comment = False
        updated = False

        for i, line in enumerate(lines):
            if line.strip() == "//APP VERSION":
                found_comment = True
                continue

            if found_comment and line.strip().startswith("const char* APP_VERSION"):
                lines[i] = f'const char* APP_VERSION = "{version_string}";\n'
                updated = True
                break

        if not found_comment:
            print("❌ Error: //APP VERSION comment not found in globals.cpp")
            return False

        if not updated:
            print("❌ Error: APP_VERSION line not found after //APP VERSION comment")
            return False

        with open(globals_file, 'w', encoding='utf-8') as file:
            file.writelines(lines)

        print(f"✅ Successfully updated APP_VERSION to: {version_string}")
        return True

    except Exception as e:
        print(f"❌ Exception: {e}")
        return False

# Always run the function when imported by PlatformIO
success = update_app_version()
if not success:
    raise Exception("Build halted due to APP_VERSION update failure.")