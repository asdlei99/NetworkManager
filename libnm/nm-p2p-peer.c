/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 * Copyright 2007 - 2008 Novell, Inc.
 * Copyright 2007 - 2018 Red Hat, Inc.
 */

#include "nm-default.h"

#include "nm-p2p-peer.h"

#include <string.h>

#include "nm-connection.h"
#include "nm-setting-connection.h"
#include "nm-setting-p2p-wireless.h"
#include "nm-utils.h"

#include "nm-dbus-interface.h"
#include "nm-object-private.h"

G_DEFINE_TYPE (NMP2PPeer, nm_p2p_peer, NM_TYPE_OBJECT)

#define NM_P2P_PEER_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), NM_TYPE_P2P_PEER, NMP2PPeerPrivate))

typedef struct {
	NM80211ApFlags flags;

	char *name;
	char *manufacturer;
	char *model;
	char *model_number;
	char *serial;

	char *hw_address;

	guint8 strength;
	int last_seen;
} NMP2PPeerPrivate;

enum {
	PROP_0,
	PROP_FLAGS,
	PROP_NAME,
	PROP_MANUFACTURER,
	PROP_MODEL,
	PROP_MODEL_NUMBER,
	PROP_SERIAL,
	PROP_WFDIES,
	PROP_HW_ADDRESS,
	PROP_STRENGTH,
	PROP_LAST_SEEN,

	LAST_PROP
};

/**
 * nm_p2p_peer_get_flags:
 * @peer: a #NMP2PPeer
 *
 * Gets the flags of the P2P peer.
 *
 * Returns: the flags
 *
 * Since: 1.16
 **/
NM80211ApFlags
nm_p2p_peer_get_flags (NMP2PPeer *peer)
{
	g_return_val_if_fail (NM_IS_P2P_PEER (peer), NM_802_11_AP_FLAGS_NONE);

	return NM_P2P_PEER_GET_PRIVATE (peer)->flags;
}

/**
 * nm_p2p_peer_get_name:
 * @peer: a #NMP2PPeer
 *
 * Gets the name of the P2P peer.
 *
 * Returns: the name
 *
 * Since: 1.16
 **/
const char *
nm_p2p_peer_get_name (NMP2PPeer *peer)
{
	g_return_val_if_fail (NM_IS_P2P_PEER (peer), NULL);

	return NM_P2P_PEER_GET_PRIVATE (peer)->name;
}

/**
 * nm_p2p_peer_get_manufacturer:
 * @peer: a #NMP2PPeer
 *
 * Gets the manufacturer of the P2P peer.
 *
 * Returns: the manufacturer
 *
 * Since: 1.16
 **/
const char *
nm_p2p_peer_get_manufacturer (NMP2PPeer *peer)
{
	g_return_val_if_fail (NM_IS_P2P_PEER (peer), NULL);

	return NM_P2P_PEER_GET_PRIVATE (peer)->manufacturer;
}

/**
 * nm_p2p_peer_get_model:
 * @peer: a #NMP2PPeer
 *
 * Gets the model of the P2P peer.
 *
 * Returns: the model
 *
 * Since: 1.16
 **/
const char *
nm_p2p_peer_get_model (NMP2PPeer *peer)
{
	g_return_val_if_fail (NM_IS_P2P_PEER (peer), NULL);

	return NM_P2P_PEER_GET_PRIVATE (peer)->model;
}

/**
 * nm_p2p_peer_get_model_number:
 * @peer: a #NMP2PPeer
 *
 * Gets the model number of the P2P peer.
 *
 * Returns: the model number
 *
 * Since: 1.16
 **/
const char *
nm_p2p_peer_get_model_number (NMP2PPeer *peer)
{
	g_return_val_if_fail (NM_IS_P2P_PEER (peer), NULL);

	return NM_P2P_PEER_GET_PRIVATE (peer)->model_number;
}

/**
 * nm_p2p_peer_get_serial:
 * @peer: a #NMP2PPeer
 *
 * Gets the serial number of the P2P peer.
 *
 * Returns: the serial number
 *
 * Since: 1.16
 **/
const char *
nm_p2p_peer_get_serial (NMP2PPeer *peer)
{
	g_return_val_if_fail (NM_IS_P2P_PEER (peer), NULL);

	return NM_P2P_PEER_GET_PRIVATE (peer)->serial;
}

/**
 * nm_p2p_peer_get_hw_address:
 * @peer: a #NMP2PPeer
 *
 * Gets the hardware address of the P2P peer.
 *
 * Returns: the hardware adress
 *
 * Since: 1.16
 **/
const char *
nm_p2p_peer_get_hw_address (NMP2PPeer *peer)
{
	g_return_val_if_fail (NM_IS_P2P_PEER (peer), NULL);

	return NM_P2P_PEER_GET_PRIVATE (peer)->hw_address;
}

/**
 * nm_p2p_peer_get_strength:
 * @peer: a #NMP2PPeer
 *
 * Gets the current signal strength of the P2P peer as a percentage.
 *
 * Returns: the signal strength (0 to 100)
 *
 * Since: 1.16
 **/
guint8
nm_p2p_peer_get_strength (NMP2PPeer *peer)
{
	g_return_val_if_fail (NM_IS_P2P_PEER (peer), 0);

	return NM_P2P_PEER_GET_PRIVATE (peer)->strength;
}

/**
 * nm_p2p_peer_get_last_seen:
 * @peer: a #NMP2PPeer
 *
 * Returns the timestamp (in CLOCK_BOOTTIME seconds) for the last time the
 * P2P peer was seen.  A value of -1 means the P2P peer has never been seen.
 *
 * Returns: the last seen time in seconds
 *
 * Since: 1.16
 **/
int
nm_p2p_peer_get_last_seen (NMP2PPeer *peer)
{
	g_return_val_if_fail (NM_IS_P2P_PEER (peer), -1);

	return NM_P2P_PEER_GET_PRIVATE (peer)->last_seen;
}

/**
 * nm_p2p_peer_connection_valid:
 * @peer: an #NMP2PPeer to validate @connection against
 * @connection: an #NMConnection to validate against @peer
 *
 * Validates a given connection against a given Wi-Fi P2P peer to ensure that
 * the connection may be activated with that peer. The connection must match the
 * @peer's address and in the future possibly other attributes.
 *
 * Returns: %TRUE if the connection may be activated with this Wi-Fi P2P Peer,
 * %FALSE if it cannot be.
 *
 * Since: 1.16
 **/
gboolean
nm_p2p_peer_connection_valid (NMP2PPeer *peer, NMConnection *connection)
{
	NMSettingConnection *s_con;
	NMSettingP2PWireless *s_p2p_wifi;
	const char *ctype;
	const char *hw_address;
	const char *setting_peer;

	s_con = nm_connection_get_setting_connection (connection);
	if (!s_con)
		return FALSE;

	ctype = nm_setting_connection_get_connection_type (s_con);
	if (!ctype || !nm_streq (ctype, NM_SETTING_P2P_WIRELESS_SETTING_NAME))
		return FALSE;

	s_p2p_wifi = (NMSettingP2PWireless*) nm_connection_get_setting (connection, NM_TYPE_SETTING_P2P_WIRELESS);
	if (!s_p2p_wifi)
		return FALSE;

	/* HW Address check */
	hw_address = nm_p2p_peer_get_hw_address (peer);
	if (!hw_address)
		return FALSE;
	setting_peer = nm_setting_p2p_wireless_get_peer (s_p2p_wifi);
	if (   !setting_peer
	    || g_strcmp0 (hw_address, setting_peer))
		return FALSE;

	return TRUE;
}

/**
 * nm_p2p_peer_filter_connections:
 * @peer: an #NMP2PPeer to filter connections for
 * @connections: (element-type NMConnection): an array of #NMConnections to
 * filter
 *
 * Filters a given array of connections for a given #NMP2PPeer object and
 * returns connections which may be activated with the P2P peer.  Any
 * returned connections will match the @peers's HW address and in the future
 * possibly other attributes.
 *
 * To obtain the list of connections that are compatible with this P2P peer,
 * use nm_client_get_connections() and then filter the returned list for a given
 * #NMDevice using nm_device_filter_connections() and finally filter that list
 * with this function.
 *
 * Returns: (transfer container) (element-type NMConnection): an array of
 * #NMConnections that could be activated with the given @peer. The array should
 * be freed with g_ptr_array_unref() when it is no longer required.
 *
 * Since: 1.16
 **/
GPtrArray *
nm_p2p_peer_filter_connections (NMP2PPeer *peer, const GPtrArray *connections)
{
	GPtrArray *filtered;
	int i;

	filtered = g_ptr_array_new_with_free_func (g_object_unref);
	for (i = 0; i < connections->len; i++) {
		NMConnection *candidate = connections->pdata[i];

		if (nm_p2p_peer_connection_valid (peer, candidate))
			g_ptr_array_add (filtered, g_object_ref (candidate));
	}

	return filtered;
}

/*****************************************************************************/

static void
get_property (GObject *object,
              guint prop_id,
              GValue *value,
              GParamSpec *pspec)
{
	NMP2PPeer *peer = NM_P2P_PEER (object);

	switch (prop_id) {
	case PROP_FLAGS:
		g_value_set_flags (value, nm_p2p_peer_get_flags (peer));
		break;
	case PROP_NAME:
		g_value_set_string (value, nm_p2p_peer_get_name (peer));
		break;
	case PROP_MANUFACTURER:
		g_value_set_string (value, nm_p2p_peer_get_manufacturer (peer));
		break;
	case PROP_MODEL:
		g_value_set_string (value, nm_p2p_peer_get_model (peer));
		break;
	case PROP_MODEL_NUMBER:
		g_value_set_string (value, nm_p2p_peer_get_model_number (peer));
		break;
	case PROP_SERIAL:
		g_value_set_string (value, nm_p2p_peer_get_serial (peer));
		break;
	case PROP_HW_ADDRESS:
		g_value_set_string (value, nm_p2p_peer_get_hw_address (peer));
		break;
	case PROP_STRENGTH:
		g_value_set_uchar (value, nm_p2p_peer_get_strength (peer));
		break;
	case PROP_LAST_SEEN:
		g_value_set_int (value, nm_p2p_peer_get_last_seen (peer));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
nm_p2p_peer_init (NMP2PPeer *peer)
{
	NM_P2P_PEER_GET_PRIVATE (peer)->last_seen = -1;
}

static void
finalize (GObject *object)
{
	NMP2PPeerPrivate *priv = NM_P2P_PEER_GET_PRIVATE (object);

	g_free (priv->name);
	g_free (priv->manufacturer);
	g_free (priv->model);
	g_free (priv->model_number);
	g_free (priv->serial);

	G_OBJECT_CLASS (nm_p2p_peer_parent_class)->finalize (object);
}

static void
init_dbus (NMObject *object)
{
	NMP2PPeerPrivate *priv = NM_P2P_PEER_GET_PRIVATE (object);
	const NMPropertiesInfo property_info[] = {
		{ NM_P2P_PEER_FLAGS,        &priv->flags },
		{ NM_P2P_PEER_NAME,         &priv->name },
		{ NM_P2P_PEER_MANUFACTURER, &priv->manufacturer },
		{ NM_P2P_PEER_MODEL,        &priv->model },
		{ NM_P2P_PEER_MODEL_NUMBER, &priv->model_number },
		{ NM_P2P_PEER_SERIAL,       &priv->serial },
		{ NM_P2P_PEER_HW_ADDRESS,   &priv->hw_address },
		{ NM_P2P_PEER_STRENGTH,     &priv->strength },
		{ NM_P2P_PEER_LAST_SEEN,    &priv->last_seen },
		{ NULL },
	};

	NM_OBJECT_CLASS (nm_p2p_peer_parent_class)->init_dbus (object);

	_nm_object_register_properties (object,
	                                NM_DBUS_INTERFACE_P2P_PEER,
	                                property_info);
}

static void
nm_p2p_peer_class_init (NMP2PPeerClass *peer_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (peer_class);
	NMObjectClass *nm_object_class = NM_OBJECT_CLASS (peer_class);

	g_type_class_add_private (peer_class, sizeof (NMP2PPeerPrivate));

	/* virtual methods */
	object_class->get_property = get_property;
	object_class->finalize = finalize;

	nm_object_class->init_dbus = init_dbus;

	/* properties */

	/**
	 * NMP2PPeer:flags:
	 *
	 * The flags of the P2P peer.
	 *
	 * Since: 1.16
	 **/
	g_object_class_install_property
		(object_class, PROP_FLAGS,
		 g_param_spec_flags (NM_P2P_PEER_FLAGS, "", "",
		                     NM_TYPE_802_11_AP_FLAGS,
		                     NM_802_11_AP_FLAGS_NONE,
		                     G_PARAM_READABLE |
		                     G_PARAM_STATIC_STRINGS));

	/**
	 * NMP2PPeer:name:
	 *
	 * The name of the P2P peer.
	 *
	 * Since: 1.16
	 **/
	g_object_class_install_property
		(object_class, PROP_NAME,
		 g_param_spec_string (NM_P2P_PEER_NAME, "", "",
		                      NULL,
		                      G_PARAM_READABLE |
		                      G_PARAM_STATIC_STRINGS));

	/**
	 * NMP2PPeer:manufacturer:
	 *
	 * The manufacturer of the P2P peer.
	 *
	 * Since: 1.16
	 **/
	g_object_class_install_property
		(object_class, PROP_MANUFACTURER,
		 g_param_spec_string (NM_P2P_PEER_MANUFACTURER, "", "",
		                      NULL,
		                      G_PARAM_READABLE |
		                      G_PARAM_STATIC_STRINGS));

	/**
	 * NMP2PPeer:model:
	 *
	 * The model of the P2P peer.
	 *
	 * Since: 1.16
	 **/
	g_object_class_install_property
		(object_class, PROP_MODEL,
		 g_param_spec_string (NM_P2P_PEER_MODEL, "", "",
		                      NULL,
		                      G_PARAM_READABLE |
		                      G_PARAM_STATIC_STRINGS));

	/**
	 * NMP2PPeer:model-number:
	 *
	 * The hardware address of the P2P peer.
	 *
	 * Since: 1.16
	 **/
	g_object_class_install_property
		(object_class, PROP_MODEL_NUMBER,
		 g_param_spec_string (NM_P2P_PEER_MODEL_NUMBER, "", "",
		                      NULL,
		                      G_PARAM_READABLE |
		                      G_PARAM_STATIC_STRINGS));

	/**
	 * NMP2PPeer:serial:
	 *
	 * The serial number of the P2P peer.
	 *
	 * Since: 1.16
	 **/
	g_object_class_install_property
		(object_class, PROP_SERIAL,
		 g_param_spec_string (NM_P2P_PEER_SERIAL, "", "",
		                      NULL,
		                      G_PARAM_READABLE |
		                      G_PARAM_STATIC_STRINGS));

	/**
	 * NMP2PPeer:hw-address:
	 *
	 * The hardware address of the P2P peer.
	 *
	 * Since: 1.16
	 **/
	g_object_class_install_property
		(object_class, PROP_HW_ADDRESS,
		 g_param_spec_string (NM_P2P_PEER_HW_ADDRESS, "", "",
		                      NULL,
		                      G_PARAM_READABLE |
		                      G_PARAM_STATIC_STRINGS));

	/**
	 * NMP2PPeer:strength:
	 *
	 * The current signal strength of the P2P peer.
	 *
	 * Since: 1.16
	 **/
	g_object_class_install_property
		(object_class, PROP_STRENGTH,
		 g_param_spec_uchar (NM_P2P_PEER_STRENGTH, "", "",
		                     0, G_MAXUINT8, 0,
		                     G_PARAM_READABLE |
		                     G_PARAM_STATIC_STRINGS));

	/**
	 * NMP2PPeer:last-seen:
	 *
	 * The timestamp (in CLOCK_BOOTTIME seconds) for the last time the
	 * P2P peer was found.  A value of -1 means the peer has never been seen.
	 *
	 * Since: 1.16
	 **/
	g_object_class_install_property
		(object_class, PROP_LAST_SEEN,
		 g_param_spec_int (NM_P2P_PEER_LAST_SEEN, "", "",
		                   -1, G_MAXINT, -1,
		                   G_PARAM_READABLE |
		                   G_PARAM_STATIC_STRINGS));
}
