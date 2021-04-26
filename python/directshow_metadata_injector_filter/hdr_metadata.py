
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

    def etof(self):
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

    def _2b_int_from_pos(self, pos):
        return self._1b_int_from_pos(pos + 1) << 8 | self._1b_int_from_pos(pos)

    def _2b_cie_from_pos(self, pos):
        cie = self._2b_int_from_pos(pos) / 50000.0
        assert 0 <= cie <= 1.0
        return cie

    def _1b_int_from_pos(self, pos):
        return int(self._metadata_split[pos], 16)
