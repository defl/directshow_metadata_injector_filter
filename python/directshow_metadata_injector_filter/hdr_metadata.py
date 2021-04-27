from enum import Enum


class PrimaryColorSpace(Enum):
    UNKNOWN = 0
    DCI_P3 = 1
    BT2020 = 2
    REC_709 = 3


class HDRMetaData(object):
    """
    Class which offers accessors to HDR metadata
    """

    def __init__(self, metadata_string):
        """
        :param metadata_string: Something like "87:01:1A:90:02:...:03:32:00:F7:03:83:00:00"
        """

        self._metadata_split = metadata_string.split(':')

        if self._metadata_split[0] != '87' or self._metadata_split[1] != '01':
            raise RuntimeError("Invalid argument, not hdr metadata: {}".format(metadata_string))

    def eotf(self):
        return self._1b_int_from_pos(4)

    def whitepoint_x(self):
        return self._2b_cie_from_pos(18)

    def whitepoint_y(self):
        return self._2b_cie_from_pos(20)

    def mastering_luminance_max(self):
        return self._2b_int_from_pos(22)

    def mastering_luminance_min(self):
        return self._2b_int_from_pos(24) * 0.0001

    def luminance_max_cll(self):
        return self._2b_int_from_pos(26)

    def luminance_max_fall(self):
        return self._2b_int_from_pos(28)

    #
    # CIE primaries
    #

    def display_primary_green_x(self):
        return self._2b_cie_from_pos(6)

    def display_primary_green_y(self):
        return self._2b_cie_from_pos(8)

    def display_primary_blue_x(self):
        return self._2b_cie_from_pos(10)

    def display_primary_blue_y(self):
        return self._2b_cie_from_pos(12)

    def display_primary_red_x(self):
        return self._2b_cie_from_pos(14)

    def display_primary_red_y(self):
        return self._2b_cie_from_pos(16)
    def primary_color_space(self) -> PrimaryColorSpace:
        gx = self._2b_int_from_pos(6)
        gy = self._2b_int_from_pos(8)
        bx = self._2b_int_from_pos(10)
        by = self._2b_int_from_pos(12)
        rx = self._2b_int_from_pos(14)
        ry = self._2b_int_from_pos(16)

        if gx == 13250 and gy == 34500 and \
           bx == 7500 and by == 3000 and \
           rx == 34000 and ry == 16000:
            return PrimaryColorSpace.DCI_P3

        if gx == 8500 and gy == 39850 and \
           bx == 6550 and by == 2300 and \
           rx == 35400 and ry == 14600:
            return PrimaryColorSpace.BT2020

        if gx == 15000 and gy == 30000 and \
           bx == 7500 and by == 3000 and \
           rx == 32000 and ry == 16500:
            return PrimaryColorSpace.REC_709

        return PrimaryColorSpace.UNKNOWN

    #
    # Private
    #

    def _2b_int_from_pos(self, pos):
        return self._1b_int_from_pos(pos + 1) << 8 | self._1b_int_from_pos(pos)

    def _2b_cie_from_pos(self, pos):
        cie = self._2b_int_from_pos(pos) / 50000.0
        assert 0 <= cie <= 1.0
        return cie

    def _1b_int_from_pos(self, pos):
        return int(self._metadata_split[pos], 16)
