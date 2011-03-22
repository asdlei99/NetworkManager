/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* NetworkManager -- Network link manager
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Copyright (C) 2011 Red Hat, Inc.
 */

#include "config.h"

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "NetworkManager.h"
#include "nm-dbus-glib-types.h"
#include "nm-marshal.h"
#include "nm-properties-changed-signal.h"
#include "nm-compat-active-connection.h"
#include "nm-compat-device.h"

G_DEFINE_TYPE (NMCompatActiveConnection, nm_compat_active_connection, G_TYPE_OBJECT)

#define NM_COMPAT_ACTIVE_CONNECTION_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                                    NM_TYPE_COMPAT_ACTIVE_CONNECTION, \
                                                    NMCompatActiveConnectionPrivate))

typedef struct {
	gboolean disposed;

	NMActRequest *parent;
	char *path;
} NMCompatActiveConnectionPrivate;


enum {
	PROP_0,
	PROP_SERVICE_NAME,
	PROP_CONNECTION,
	PROP_SPECIFIC_OBJECT,
	PROP_DEVICES,
	PROP_STATE,
	PROP_DEFAULT,
	PROP_DEFAULT6,
	PROP_VPN,

	LAST_PROP
};

enum {
	PROPERTIES_CHANGED,
	LAST_SIGNAL
};
static guint signals[LAST_SIGNAL] = { 0 };


#include "nm-compat-active-connection-glue.h"

#define NM_OLD_ACTIVE_CONNECTION_SERVICE_NAME "service-name"

/*************************************************************************/

#define NM_OLD_AC_STATE_UNKNOWN    0
#define NM_OLD_AC_STATE_ACTIVATING 1
#define NM_OLD_AC_STATE_ACTIVATED  2

static guint32
new_state_to_old (NMActiveConnectionState state)
{
	switch (state) {
	case NM_ACTIVE_CONNECTION_STATE_ACTIVATING:
		return NM_OLD_AC_STATE_ACTIVATING;
	case NM_ACTIVE_CONNECTION_STATE_ACTIVATED:
		return NM_OLD_AC_STATE_ACTIVATED;
	default:
		return NM_OLD_AC_STATE_UNKNOWN;
	}
	g_assert_not_reached ();
}

static void
prop_reemit_cb (GObject *object, GParamSpec *pspec, NMCompatActiveConnection *self)
{
	g_object_notify (G_OBJECT (self), pspec->name);
}

/*************************************************************************/

const char *
nm_compat_active_connection_get_path (NMCompatActiveConnection *self)
{
	return NM_COMPAT_ACTIVE_CONNECTION_GET_PRIVATE (self)->path;
}

NMCompatActiveConnection *
nm_compat_active_connection_new (NMActRequest *parent, DBusGConnection *bus)
{
	NMCompatActiveConnection *self;
	NMCompatActiveConnectionPrivate *priv;
	static guint32 idx = 0;

	self = (NMCompatActiveConnection *) g_object_new (NM_TYPE_COMPAT_ACTIVE_CONNECTION, NULL);
	if (self) {
		priv = NM_COMPAT_ACTIVE_CONNECTION_GET_PRIVATE (self);

		priv->parent = parent;
		g_signal_connect (parent, "notify::" NM_OLD_ACTIVE_CONNECTION_SERVICE_NAME, G_CALLBACK (prop_reemit_cb), self);
		g_signal_connect (parent, "notify::" NM_ACTIVE_CONNECTION_CONNECTION, G_CALLBACK (prop_reemit_cb), self);
		g_signal_connect (parent, "notify::" NM_ACTIVE_CONNECTION_SPECIFIC_OBJECT, G_CALLBACK (prop_reemit_cb), self);
		g_signal_connect (parent, "notify::" NM_ACTIVE_CONNECTION_DEVICES, G_CALLBACK (prop_reemit_cb), self);
		g_signal_connect (parent, "notify::" NM_ACTIVE_CONNECTION_STATE, G_CALLBACK (prop_reemit_cb), self);
		g_signal_connect (parent, "notify::" NM_ACTIVE_CONNECTION_DEFAULT, G_CALLBACK (prop_reemit_cb), self);
		g_signal_connect (parent, "notify::" NM_ACTIVE_CONNECTION_DEFAULT6, G_CALLBACK (prop_reemit_cb), self);
		g_signal_connect (parent, "notify::" NM_ACTIVE_CONNECTION_VPN, G_CALLBACK (prop_reemit_cb), self);

		priv->path = g_strdup_printf ("/org/freedesktop/NetworkManagerCompat/ActiveConnection/%d", idx++);
		dbus_g_connection_register_g_object (bus, priv->path, G_OBJECT (self));
	}

	return self;
}

static void
nm_compat_active_connection_init (NMCompatActiveConnection *self)
{
}

static void
set_property (GObject *object, guint prop_id,
			  const GValue *value, GParamSpec *pspec)
{
}

static void
get_property (GObject *object, guint prop_id,
			  GValue *value, GParamSpec *pspec)
{
	NMCompatActiveConnection *self = NM_COMPAT_ACTIVE_CONNECTION (object);
	NMCompatActiveConnectionPrivate *priv = NM_COMPAT_ACTIVE_CONNECTION_GET_PRIVATE (self);
	NMConnection *connection;
	NMCompatDevice *compat;
	GPtrArray *devices;
	char *str;
	guint32 u;
	gboolean b;

	connection = nm_act_request_get_connection (priv->parent);

	switch (prop_id) {
	case PROP_SERVICE_NAME:
		if (g_object_get_data (G_OBJECT (connection), "user"))
			g_value_set_string (value, "org.freedesktop.NetworkManagerUserSettings");
		else
			g_value_set_string (value, "org.freedesktop.NetworkManagerSystemSettings");
		break;
	case PROP_CONNECTION:
		g_value_set_boxed (value, nm_connection_get_path (connection));
		break;
	case PROP_SPECIFIC_OBJECT:
		g_object_get (priv->parent, NM_ACTIVE_CONNECTION_SPECIFIC_OBJECT, &str, NULL);
		g_value_take_boxed (value, str);
		break;
	case PROP_DEVICES:
		compat = nm_device_get_compat (NM_DEVICE (nm_act_request_get_device (priv->parent)));
		devices = g_ptr_array_sized_new (1);
		g_ptr_array_add (devices, g_strdup (nm_compat_device_get_path (compat)));
		g_value_take_boxed (value, devices);
		break;
	case PROP_STATE:
		g_object_get (priv->parent, NM_ACTIVE_CONNECTION_STATE, &u, NULL);
		g_value_set_uint (value, new_state_to_old (u));
		break;
	case PROP_DEFAULT:
		g_object_get (priv->parent, NM_ACTIVE_CONNECTION_DEFAULT, &b, NULL);
		g_value_set_boolean (value, b);
		break;
	case PROP_DEFAULT6:
		g_object_get (priv->parent, NM_ACTIVE_CONNECTION_DEFAULT6, &b, NULL);
		g_value_set_boolean (value, b);
		break;
	case PROP_VPN:
		g_object_get (priv->parent, NM_ACTIVE_CONNECTION_VPN, &b, NULL);
		g_value_set_boolean (value, b);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
finalize (GObject *object)
{
	g_free (NM_COMPAT_ACTIVE_CONNECTION_GET_PRIVATE (object)->path);
	G_OBJECT_CLASS (nm_compat_active_connection_parent_class)->finalize (object);
}

static void
nm_compat_active_connection_class_init (NMCompatActiveConnectionClass *compat_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (compat_class);

	g_type_class_add_private (compat_class, sizeof (NMCompatActiveConnectionPrivate));

	object_class->finalize = finalize;
	object_class->set_property = set_property;
	object_class->get_property = get_property;

	g_object_class_install_property (object_class, PROP_SERVICE_NAME,
		g_param_spec_string (NM_OLD_ACTIVE_CONNECTION_SERVICE_NAME,
		                     "Service name",
		                     "Service name",
		                     NULL,
		                     G_PARAM_READABLE));

    nm_active_connection_install_properties (object_class,
                                             PROP_CONNECTION,
                                             PROP_SPECIFIC_OBJECT,
                                             PROP_DEVICES,
                                             PROP_STATE,
                                             PROP_DEFAULT,
                                             PROP_DEFAULT6,
                                             PROP_VPN);

	signals[PROPERTIES_CHANGED] =
		nm_properties_changed_signal_new (object_class,
		                                  G_STRUCT_OFFSET (NMCompatActiveConnectionClass, properties_changed));

	dbus_g_object_type_install_info (G_TYPE_FROM_CLASS (compat_class), &dbus_glib_nm_compat_active_connection_object_info);
}

