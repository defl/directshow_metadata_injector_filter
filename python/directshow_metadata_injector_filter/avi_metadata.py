
class AVIMetaData(object):
    """
    Class which offers accessors to AVI metadata
    """

    def __init__(self, metadata_string):
        """
        :param metadata_string: Something like "82:02:0D:D5:32:E8:60:20:00:00:00:00:00:00:00:00:00"
        """

        self._metadata_split = metadata_string.split(':')

        if not (self._metadata_split[0] == "82" and
                (self._metadata_split[1] == "02" or self._metadata_split[1] == "03")):
            raise RuntimeError("Invalid argument, not AVI metadata: {}".format(metadata_string))

    def csc(self):
        return self._1b_int_from_pos(6) & 0x0C

    def vic(self):
        return self._1b_int_from_pos(7)

    def _1b_int_from_pos(self, pos):
        return int(self._metadata_split[pos], 16)
