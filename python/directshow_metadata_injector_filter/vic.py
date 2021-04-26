# EIA/CEA-861 standard resolutions and timings
# https://en.wikipedia.org/wiki/Extended_Display_Identification_Data

import csv
import os


def vic_by_id(id):
    """
    Get the VIC by id
    :return: {} of metadata
    """

    assert 0 < id < 300

    this_dir, this_filename = os.path.split(__file__)

    with open(os.path.join(this_dir, "vic_codes.csv")) as csv_file:
        vic_reader = csv.DictReader(csv_file, delimiter=';')
        for row in vic_reader:
            if int(row['vic']) == id:
                return row

    raise Exception("Unknown VIC: {}".format(id))
