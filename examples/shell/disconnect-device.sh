#!/bin/sh
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2012 Red Hat, Inc.
#

# This example takes a device interface name as a parameter and tells
# NetworkManager to disconnect that device (using Disconnect()), closing
# down any network connection it may have

NM_SERVICE_NAME="org.freedesktop.NetworkManager"
NM_OBJECT_PATH="/org/freedesktop/NetworkManager"
DEVICE_IFACE="org.freedesktop.NetworkManager.Device"
DEVICE_DISCONNECT=${DEVICE_IFACE}".Disconnect"
NM_GET_DEVICES="org.freedesktop.NetworkManager.GetDevices"
DBUS_PROPERTIES_GET="org.freedesktop.DBus.Properties.Get"

get_devices()
{
  dbus-send --system --print-reply --dest=$NM_SERVICE_NAME $NM_OBJECT_PATH $NM_GET_DEVICES | \
    grep "object path" | cut -d '"' -f2
}

get_device_property()
{
  # first arg:  device object path
  # second arg: property name
  # returns:    property value

  dbus-send --system --print-reply --dest=$NM_SERVICE_NAME "$1" $DBUS_PROPERTIES_GET string:$DEVICE_IFACE string:"$2" | \
    grep "variant" | awk '{print $3}' | sed 's/"//g'
}

disconnect_device ()
{
  # first arg: device interface name

  for device in `get_devices`
  do
    DEV_INTERFACE=`get_device_property "$device" "Interface"`

    if [ "$DEV_INTERFACE" = "$1" ]; then
      echo "Device with interface '$1' found -> disconnecting."
      dbus-send --system --print-reply --dest=$NM_SERVICE_NAME $device $DEVICE_DISCONNECT
      return 0
    fi
  done

  echo "Device with interface '$1' not found." >&2
  return 1
}


# --- main program ---
if [ ! -n "$1" ]; then
  echo "Usage: `basename $0` <interface name>"
  exit 2
fi

# disconnect device
disconnect_device $1

