/*  Copyright (C) 2020 NANDO authors
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#define SETTINGS_ORGANIZATION_NAME "NANDO"
#define SETTINGS_APPLICATION_NAME "NANDO"

#define SETTINGS_PROGRAMMER_SECTION "programmer/"
#define SETTINGS_GUI_SECTION "GUI/"
#define SETTINGS_USB_DEV_NAME SETTINGS_PROGRAMMER_SECTION "usb_dev_name"
#define SETTINGS_SKIP_BAD_BLOCKS SETTINGS_PROGRAMMER_SECTION "skip_bad_blocks"
#define SETTINGS_INCLUDE_SPARE_AREA SETTINGS_PROGRAMMER_SECTION \
    "include_spare_area"
#define SETTINGS_ENABLE_HW_ECC SETTINGS_PROGRAMMER_SECTION \
    "enable_hw_ecc"
#define SETTINGS_ENABLE_ALERT SETTINGS_GUI_SECTION "enable_alert"
#define SETTINGS_WORK_FILE_PATH SETTINGS_GUI_SECTION "work_file_path"

#endif // SETTINGS_H
