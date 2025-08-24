# xiaomi-bootlogo-editor

[中文版](README_CN.md)

Xiaomi/Redmi Qualcomm bootlogo unpack repack tool

## Compatibility

This tool is only compatible with Qualcomm devices.
Since MIUI 13, bootlogo has been moved to `imagefv` partition, 
so this tool is not compatible with MIUI 13 and higher versions.

Before starting, you need to be sure that your device is supported

To do these tasks, extract `logo` partition, and try to unpack it.
If it unpacked successfully, your device is supported.

Note: If `logo` partition failed to unpack, you can also try `splash` partition.

Known supported devices:
- Xiaomi Mi 6 (sagit)
- Xiaomi Mi 8 (dipper)
- Xiaomi Mi 9 (cepheus)
- Redmi K20 Pro (raphael)

## Important

This tool adds BMP extension to every unpacked file, but this is not always right.
For example, dipper has a PNG format (third one) file inside, you need to ignore the BMP file extensions.
After changing these different format logos, make sure it is still `logoX.bmp`

Figure out the real file format with something like winhex or online tool.
If you have no idea, check file size. Under same resolution, other format are significantly smaller.

You need to replace the unpacked file with the file in the same format, 
but you can try different resolution if you have wierd size images unpacked.
But, don't exceed your display resolution, and don't exceed your logo partition size.

## Unpack

0. always delete `unpacked` folder before continuing
1. backup your `logo` partition, put it in the folder where the tool is located
2. open command line (Windows:CMD Linux:Terminal) and enter `logotool unpack <your-backup-image-name>`
3. unpacked images are in `unpacked` folder

## Repack

0. always make sure that you have `unpacked` folder with `logoX.bmp` inside
1. open command line and enter `logotool repack <your-new-image-name>`
2. make sure that your repacked image is not bigger than `logo` partition
3. flash your new image
