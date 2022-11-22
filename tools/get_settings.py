#!/usr/bin/python
"""
Script to download the settings.json from the esp.
The esp needs to be connected to the wifi and
you may need to adjust the ip address in the
esp_ip variable.
"""

import subprocess

esp_ip = "192.168.0.117"

cmd = ["wget", "-O", "data/settings.json", f"{esp_ip}/settings.json"]
subprocess.run(cmd)
