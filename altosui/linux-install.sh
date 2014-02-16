#!/bin/sh

can_ask=y

finish()
{
    if [ "$can_ask" = "y" ]; then
	echo ""
	echo -n "Press enter to continue..."
	read foo
    fi
    exit $1
}

#
# Make sure we have a terminal to talk to
#

if tty -s; then
    :
else
    case "$DISPLAY" in
	"")
	echo 'No user input available'
	can_ask=n
	;;
	*)
	GUESS_XTERMS="x-terminal-emulator xterm rxvt roxterm gnome-terminal dtterm eterm Eterm kvt konsole aterm"
        for a in $GUESS_XTERMS; do
            if type $a >/dev/null 2>&1; then
                XTERM=$a
                break
            fi
        done
	case "$XTERM" in
	    "")
	    echo 'No terminal emulator available'
	    can_ask=n
	    ;;
	    *)
	    exec "$XTERM" -e "sh '$0'"
	    ;;
	esac
	;;
    esac
fi

#
# Make sure we can run java
#

echo -n "Checking for java..."

if java -version > /dev/null 2>&1; then
    echo " found it."
else
    echo " java isn't working."
    echo ""
    echo "You'll need to install a java runtime system"
    echo "on this computer before AltOS will work properly."
    finish 1
fi
    
#
# Pick an installation target
# 

if [ '(' -d /opt -a -w /opt ')' -o '(' -d /opt/AltOS -a -w /opt/AltOS ')' ]; then
    target_default=/opt
else
    target_default="$HOME"
fi

case "$#" in
0)
    echo -n "Installation location [default: $target_default] "
    if [ "$can_ask" = "y" ]; then
	read target
    else
	echo ""
	target=""
    fi
    case "$target" in
	"")
	target="$target_default"
	;;
    esac
    ;;
*)
    target="$1"
    ;;
esac

target_altos="$target"/AltOS

echo -n "Installing to $target..."

#
# Make sure the target exists
#
mkdir -p "$target_altos"

if [ ! -d "$target_altos" ]; then
    echo "$target_altos does not exist and cannot be created"
    finish 1
fi

if [ ! -w "$target_altos" ]; then
    echo "$target_altos cannot be written"
    finish 1
fi

#
# Unpack the tar archive appended to the end of this script
#
archive_line=`awk '/^__ARCHIVE_BELOW__/ {print NR + 1; exit 0; }' "$0"`

tail -n+$archive_line "$0" | tar xjf - -C "$target"

case $? in
0)
    echo " done."
    ;;
*)
    echo "Install failed."
    finish 1
    ;;
esac

#
# Create the .desktop file by editing the paths
#
case "$target" in
/*)
    target_abs="$target"
    ;;
*)
    target_abs=`pwd`/$target
    ;;
esac

BIN="$target_abs"/AltOS

for infile in "$target"/AltOS/*.desktop.in; do
    desktop="$target"/AltOS/`basename "$infile" .in`
    rm -f "$desktop"
    sed -e "s;%bindir%;$BIN;" -e "s;%icondir%;$BIN;" "$infile" > "$desktop"
    chmod +x "$desktop"
done

#
# Figure out where to install the .desktop files. If we can, write it
# to the public /usr/share/applications, otherwise, write it to the
# per-user ~/.local/share/applications
#

public=/usr/share/applications
private=$HOME/.local/share/applications
apps=""

if [ -d "$public" -a -w "$public" ]; then
    apps="$public"
else
    mkdir -p "$private" >/dev/null 2>&1 
    if [ -d "$private" -a -w "$private" ]; then
	apps="$private"
    fi
fi
	
case "$apps" in
"")
    echo "Cannot install application icon"
    finish 1
    ;;
esac

echo -n "Installing .desktop files to $apps..."

cp "$target"/AltOS/*.desktop "$apps"

case "$?" in
0)
    echo " done."
    ;;
*)
    echo " failed."
    ;;
esac

#
# Install icon to desktop if desired
#

if [ -d $HOME/Desktop ]; then
    default_desktop=n
    if [ "$can_ask" = "y" ]; then
	:
    else
	default_desktop=y
    fi

    answered=n
    while [ "$answered" = "n" ]; do
	echo -n "Install icons to desktop? [default: $default_desktop] "
	if [ "$can_ask" = "y" ]; then
	    read do_desktop
	else
	    echo
	    do_desktop=""
	fi

	case "$do_desktop" in
	    "")
	    do_desktop=$default_desktop
	    ;;
	esac

	case "$do_desktop" in
	[yYnN]*)
	    answered=y
	    ;;
	esac
    done

    echo -n "Installing desktop icons..."
    case "$do_desktop" in
    [yY]*)
	for d in "$target"/AltOS/*.desktop; do
	    ln -f -s "$d" "$HOME/Desktop/"
	done
	;;
    esac

    echo " done."
fi

finish 0

__ARCHIVE_BELOW__
