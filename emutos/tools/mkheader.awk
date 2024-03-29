#
# mkheader.awk - generate definitions for the TOS header
#
# Copyright (c) 2003 EmuTOS development team.
#
# Authors:
#  LVL     Laurent Vogel
#
# This file is distributed under the GPL, version 2 or at your
# option any later version.  See doc/license.txt for details.
#

function get_date(   d, a)
{
    "date +%Y,%m,%d" | getline d
    close("date")
    split(d, a, ",")
    year = a[1]
    month = a[2]
    day = a[3]
}

BEGIN {
    # obtain variables year, month and day
    get_date()
    today = year "-" month "-" day
    
    # check parameters
    if (ARGC != 2 || ! match(ARGV[1], /^[a-z][a-z]$/)) {
        print ARGC ARGV[0] ARGV[1]
        print "usage: mkheader xx"
        print "where xx is a lowercase two-char country name"
        exit (1)
    }
    country = ARGV[1]
    uccountry = toupper(country)
    
    print "/*"
    print " * header.h - definitions for the TOS header"
    print " *"
    print " * This file was automatically generated by mkheader.awk on " today
    print " */\n"
    
    print "#ifndef HEADER_H"
    print "#define HEADER_H\n"
    
    print "#include \"ctrycodes.h\"\n"
    
    print "/* The defines below must only be used in startup.S and comprimg.S."
    print " * Then the OS header variables must be used elsewhere."
    print " * This allows tools like tos-lang-change to patch the ROM header afterwards."
    print " */\n"

    print "/* the build date in Binary-Coded Decimal */"
    print "#define OS_DATE 0x" month day year "\n"
    
    dos_date = day + month * 32 + (year - 1980) * 512 

    print "/* the build date in GEMDOS format */"
    print "#define OS_DOSDATE " dos_date "\n"

    print "/* the country number only */"
    print "#define OS_COUNTRY COUNTRY_" uccountry "\n"

    print "/* the PAL/NTSC flag */"
    if (uccountry == "US")
        print "#define OS_PAL 0\n"
    else
        print "#define OS_PAL 1\n"

    print "#endif /* HEADER_H */"
}

