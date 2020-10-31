#!/bin/sh

NEW_SOFT_VERSION="2020.10.19.4242"

# Example
echo $NEW_SOFT_VERSION > /etc/software_version && sync
echo "Software version changed to $NEW_SOFT_VERSION"

exit 0