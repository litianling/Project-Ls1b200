/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * demo_gui.h
 *
 *  Created on: 2015-1-26
 *      Author: Bian
 */

#ifndef _GUI_DESKTOP_H_
#define _GUI_DESKTOP_H_

#ifdef	__cplusplus
extern "C" {
#endif

void gui_drawtext_in_grid(int row, int col, const char *str);
void show_desktop_gui(void);

#ifdef	__cplusplus
}
#endif

#endif /* _DEMO_GUI_H_ */
