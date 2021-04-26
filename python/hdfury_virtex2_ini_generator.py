# This program generates an ini file containing the correct HDR metadata from a HDFury Virtex2

import argparse
from time import sleep

import requests

from directshow_metadata_injector_filter.avi_metadata import AVIMetaData
from directshow_metadata_injector_filter.hdr_metadata import HDRMetaData

# Parse arguments
from directshow_metadata_injector_filter.ini import write_ini
from directshow_metadata_injector_filter.vic import vic_by_id

parser = argparse.ArgumentParser()
parser.add_argument("ip", help="IP address of the HDFury Virtex2")
parser.add_argument("ini_filename", help="Full path to the generated .ini file")
parser.add_argument("--once", help="Do not run continuously but stop after first run", action="store_true")
args = parser.parse_args()

# Main loop
while True:

    output = {}

    #
    # Get data from Vertex2
    #
    url = 'http://{}/ssi/hdrpage.ssi'.format(args.ip)
    data = requests.get(url).json()

    #
    # Process AVI metadata
    #
    avi = AVIMetaData(data['AVI'])

    # CSC range
    csc = avi.csc()
    if csc == 0x00:
        output['nominal_range'] = 0  # unknown
    elif csc == 0x04:
        output['nominal_range'] = 2  # 16-235 (wide)
    elif csc == 0x08:
        output['nominal_range'] = 1  # 0-255 (normal, full)
    else:
        raise Exception("Don't know AVI CSC: {}".format(csc))

    # VIC and then aspect ratio
    vic_id = avi.vic()
    vic_data = vic_by_id(vic_id)
    dar = vic_data['display_aspect_ratio']
    dar_parts = dar.split(':')
    assert(len(dar_parts) == 2)
    output['aspect_ratio_x'] = int(dar_parts[0])
    output['aspect_ratio_y'] = int(dar_parts[1])

    #
    # Process HDR metadata
    #
    hdr = HDRMetaData(data['HDR'])

    # ETOF
    etof = hdr.etof()
    if etof == 0:
        output['video_transfer_function'] = 5  # SDR Luminance Range
    elif etof == 2:
        output['video_transfer_function'] = 15  # SMPTE ST 2084 [PQ]
    elif etof == 3:
        output['video_transfer_function'] = 16  # HLG ITU-R BT.2100-0
    else:
        raise Exception("Don't know HDR ETOF: {}" + etof)

    # White points
    output['white_point_x'] = hdr.whitepoint_x()
    output['white_point_y'] = hdr.whitepoint_y()

    # Mastering display min/max luminance
    output['mastering_luminance_max'] = hdr.mastering_luminance_max()
    output['mastering_luminance_min'] = hdr.mastering_luminance_min()

    # MaxCLL / MaxFALL
    output['luminance_max_cll'] = hdr.luminance_max_cll()
    output['luminance_max_fall'] = hdr.luminance_max_fall()

    # Primaries
    output['display_primary_green_x'] = hdr.display_primary_green_x()
    output['display_primary_green_y'] = hdr.display_primary_green_y()
    output['display_primary_blue_x'] = hdr.display_primary_blue_x()
    output['display_primary_blue_y'] = hdr.display_primary_blue_y()
    output['display_primary_red_x'] = hdr.display_primary_red_x()
    output['display_primary_red_y'] = hdr.display_primary_red_y()

    #
    # Write file
    #
    write_ini(args.ini_filename, output)

    print("Generated {}".format(args.ini_filename))

    if args.once:
        break

    sleep(10)
