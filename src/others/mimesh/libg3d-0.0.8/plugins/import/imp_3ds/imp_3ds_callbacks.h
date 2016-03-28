/* $Id$ */

/*
    libg3d - 3D object loading library

    Copyright (C) 2005-2009  Markus Dahms <mad@automagically.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef _IMP_3DS_CALLBACKS_H
#define _IMP_3DS_CALLBACKS_H

#include "imp_3ds.h"

gboolean x3ds_cb_0x0002(x3ds_global_data *global, x3ds_parent_data *parent);
gboolean x3ds_cb_0x0010(x3ds_global_data *global, x3ds_parent_data *parent);
gboolean x3ds_cb_0x0011(x3ds_global_data *global, x3ds_parent_data *parent);
gboolean x3ds_cb_0x0030(x3ds_global_data *global, x3ds_parent_data *parent);
gboolean x3ds_cb_0x0031(x3ds_global_data *global, x3ds_parent_data *parent);
gboolean x3ds_cb_0x0100(x3ds_global_data *global, x3ds_parent_data *parent);

gboolean x3ds_cb_0x4000(x3ds_global_data *global, x3ds_parent_data *parent);
gboolean x3ds_cb_0x4110(x3ds_global_data *global, x3ds_parent_data *parent);
gboolean x3ds_cb_0x4120(x3ds_global_data *global, x3ds_parent_data *parent);
gboolean x3ds_cb_0x4130(x3ds_global_data *global, x3ds_parent_data *parent);
gboolean x3ds_cb_0x4140(x3ds_global_data *global, x3ds_parent_data *parent);
gboolean x3ds_cb_0x4150(x3ds_global_data *global, x3ds_parent_data *parent);
gboolean x3ds_cb_0x4160(x3ds_global_data *global, x3ds_parent_data *parent);

gboolean x3ds_cb_0xA000(x3ds_global_data *global, x3ds_parent_data *parent);
gboolean x3ds_cb_0xA081(x3ds_global_data *global, x3ds_parent_data *parent);
gboolean x3ds_cb_0xA300(x3ds_global_data *global, x3ds_parent_data *parent);
gboolean x3ds_cb_0xA354(x3ds_global_data *global, x3ds_parent_data *parent);
gboolean x3ds_cb_0xA356(x3ds_global_data *global, x3ds_parent_data *parent);
gboolean x3ds_cb_0xAFFF(x3ds_global_data *global, x3ds_parent_data *parent);

gboolean x3ds_cb_0xB00A(x3ds_global_data *global, x3ds_parent_data *parent);
gboolean x3ds_cb_0xB010(x3ds_global_data *global, x3ds_parent_data *parent);
gboolean x3ds_cb_0xB013(x3ds_global_data *global, x3ds_parent_data *parent);
gboolean x3ds_cb_0xB020(x3ds_global_data *global, x3ds_parent_data *parent);
gboolean x3ds_cb_0xB021(x3ds_global_data *global, x3ds_parent_data *parent);
gboolean x3ds_cb_0xB030(x3ds_global_data *global, x3ds_parent_data *parent);

#endif /* _IMP_3DS_CALLBACKS_H */
